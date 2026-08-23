#include "pixel.h"
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

// Start the timer
PixelTimer Pixel_TimerStart(void) {
    PixelTimer timer;
    timespec_get(&timer.start_time, TIME_UTC);
    return timer;
}

// Stop the timer and return elapsed time in milliseconds (float)
float Pixel_TimerElapsedMs(PixelTimer* timer) {
    struct timespec end_time;
    timespec_get(&end_time, TIME_UTC);

    long seconds = end_time.tv_sec - timer->start_time.tv_sec;
    long nanoseconds = end_time.tv_nsec - timer->start_time.tv_nsec;

    return (float)seconds * 1000.0f + (float)nanoseconds / 1000000.0f;
}

// -----------------------------------------------------------------
// 3D Rendering Related stuff (Vertex, Mesh, etc.)
// -----------------------------------------------------------------
Color Pixel_SampleTexture(PixelTexture *texture, float x, float y) {
    return WHITE;
}


// Transform world lights to View Space once per frame
static void Pixel_PrepareLights(const Light *world_lights, Light *view_lights, int light_count, Mat4 view_matrix) {
    // // PixelTimer timer = Pixel_TimerStart();
    for (int i = 0; i < light_count; i++) {
        view_lights[i] = world_lights[i];
        if (world_lights[i].type == 1) { // Point Light Position
            Vec4 vp = Pixel_Mat4MulVec4(view_matrix, (Vec4){world_lights[i].position.x, world_lights[i].position.y, world_lights[i].position.z, 1.0f});
            view_lights[i].position = (Vec3){vp.x, vp.y, vp.z};
        } else if (world_lights[i].type == 2) { // Directional Light Direction (Ignore translation w = 0)
            Vec4 vd = Pixel_Mat4MulVec4(view_matrix, (Vec4){world_lights[i].direction.x, world_lights[i].direction.y, world_lights[i].direction.z, 0.0f});
            view_lights[i].direction = Pixel_Vec3Norm((Vec3){vd.x, vd.y, vd.z});
        }
    }
    // // printf("[Lights World to Viewspace]: %f\n", Pixel_TimerElapsedMs(&timer));
}

// Calculate Blinn-Phong lighting accumulation at a single vertex (Gouraud Shading)
static Color Pixel_CalculateVertexLighting(Vec3 view_pos, Vec3 view_normal, const Light *lights, int light_count, Material material) {
    // // PixelTimer timer = Pixel_TimerStart();
    Vec3 N = Pixel_Vec3Norm(view_normal);
    Vec3 V = Pixel_Vec3Norm(Pixel_Vec3Sub((Vec3){0.0f, 0.0f, 0.0f}, view_pos)); // Camera at origin in View Space

    // Initialize lighting intensity accumulator (1.0 = full baseline ambient multiplier)
    Color total_light = {
        material.ambient.r / 255.0f,
        material.ambient.g / 255.0f,
        material.ambient.b / 255.0f,
        1.0f
    };

    for (int l = 0; l < light_count; l++) {
        Light light = lights[l];
        Vec3 L;
        float attenuation = 1.0f;

        if (light.type == AMBIENT_LIGHT) { // Ambient Light source type
            total_light.r += (light.color.r / 255.0f) * light.intensity * (material.ambient.r / 255.0f);
            total_light.g += (light.color.g / 255.0f) * light.intensity * (material.ambient.g / 255.0f);
            total_light.b += (light.color.b / 255.0f) * light.intensity * (material.ambient.b / 255.0f);
            continue;
        }
        else if (light.type == POINT_LIGHT) { // Point Light
            Vec3 light_dir_vec = Pixel_Vec3Sub(light.position, view_pos);
            float distance = Pixel_Vec3Len(light_dir_vec);
            L = Pixel_Vec3Norm(light_dir_vec);
            // Attenuation formula
            attenuation = 1.0f / (1.0f + 0.09f * distance + 0.032f * (distance * distance));
        }
        else { // Directional Light
            L = Pixel_Vec3Norm(Pixel_Vec3Mul(light.direction, -1.0f));
        }

        // --- Diffuse Term (Lambertian) ---
        float diff = Pixel_Max(0.0f, Pixel_Vec3Dot(N, L));
        float lr = light.color.r / 255.0f;
        float lg = light.color.g / 255.0f;
        float lb = light.color.b / 255.0f;

        total_light.r += lr * light.intensity * diff * (material.diffuse.r / 255.0f) * attenuation;
        total_light.g += lg * light.intensity * diff * (material.diffuse.g / 255.0f) * attenuation;
        total_light.b += lb * light.intensity * diff * (material.diffuse.b / 255.0f) * attenuation;

        // --- Specular Term (Blinn-Phong) ---
        if (diff > 0.0f) {
            Vec3 H = Pixel_Vec3Norm(Pixel_Vec3Add(L, V));
            float spec = powf(Pixel_Max(0.0f, Pixel_Vec3Dot(N, H)), material.shininess);
            // Allow high shininess values to accumulate full magnitude without artificial suppression
            total_light.r += lr * light.intensity * spec * (material.specular.r / 255.0f) * attenuation;
            total_light.g += lg * light.intensity * spec * (material.specular.g / 255.0f) * attenuation;
            total_light.b += lb * light.intensity * spec * (material.specular.b / 255.0f) * attenuation;
        }
    }
    // // printf("[Single Vertex Blin-Phong Lighting Calculation]: %f\n", Pixel_TimerElapsedMs(&timer));
    return total_light;
}

static PixelBoundingBox Pixel_ComputeTriangleBoundingBox(PixelContext *ctx, VertexOutput v0, VertexOutput v1, VertexOutput v2) {
    PixelBoundingBox box = {0};
    box.x_min = (int)Pixel_Max(0.0f, floorf(Pixel_Min(Pixel_Min(v0.screen_pos.x, v1.screen_pos.x), v2.screen_pos.x)));
    box.x_max = (int)Pixel_Min((float)(ctx->width - 1), ceilf(Pixel_Max(Pixel_Max(v0.screen_pos.x, v1.screen_pos.x), v2.screen_pos.x)));
    box.y_min = (int)Pixel_Max(0.0f, floorf(Pixel_Min(Pixel_Min(v0.screen_pos.y, v1.screen_pos.y), v2.screen_pos.y)));
    box.y_max = (int)Pixel_Min((float)(ctx->height - 1), ceilf(Pixel_Max(Pixel_Max(v0.screen_pos.y, v1.screen_pos.y), v2.screen_pos.y)));
    return box;
}

static PixelBaryWeightDiff Pixel_ComputeBaryWeightDiff(VertexOutput v0, VertexOutput v1, VertexOutput v2) {
    PixelBaryWeightDiff diff = {0};
    diff.w0_col = v1.screen_pos.y - v2.screen_pos.y;
    diff.w1_col = v2.screen_pos.y - v0.screen_pos.y;
    diff.w2_col = v0.screen_pos.y - v1.screen_pos.y;

    diff.w0_row = v2.screen_pos.x - v1.screen_pos.x;
    diff.w1_row = v0.screen_pos.x - v2.screen_pos.x;
    diff.w2_row = v1.screen_pos.x - v0.screen_pos.x;

    return  diff;
}

static void Pixel_InterpolateAttributes(VertexOutput v0, VertexOutput v1, VertexOutput v2, float alpha, float beta, float gamma, PixelInterpolatedAttributes *interpolated_values) {
    interpolated_values->depth = v0.screen_pos.z * alpha + v1.screen_pos.z * beta + v2.screen_pos.z * gamma;

    interpolated_values->light_color = {
        v0.light_factor.r * alpha + v1.light_factor.r * beta + v2.light_factor.r * gamma,
        v0.light_factor.g * alpha + v1.light_factor.g * beta + v2.light_factor.g * gamma,
        v0.light_factor.b * alpha + v1.light_factor.b * beta + v2.light_factor.b * gamma,
        1.0f
    };

    interpolated_values->base = {
        v0.base_color.r * alpha + v1.base_color.r * beta + v2.base_color.r * gamma,
        v0.base_color.g * alpha + v1.base_color.g * beta + v2.base_color.g * gamma,
        v0.base_color.b * alpha + v1.base_color.b * beta + v2.base_color.b * gamma,
        255.0f
    };

    interpolated_values->uv = {
        v0.uv.x * alpha + v1.uv.x * beta + v2.uv.x * gamma,
        v0.uv.y * alpha + v1.uv.y * beta + v2.uv.y * gamma
    };
}

static uint32_t Pixel_ShadeFragment(PixelContext *ctx, const PixelTexture *texture, bool use_texture, PixelInterpolatedAttributes *interpolated_attr) {
    Color surface_color;
    if (use_texture && texture) {
        surface_color = Pixel_SampleTexture((PixelTexture*)texture, interpolated_attr->uv.x, interpolated_attr->uv.y);
    } else {
        surface_color = interpolated_attr->base;
    }

    float final_r = Pixel_Max(0.0f, Pixel_Min(255.0f, surface_color.r * interpolated_attr->light_color.r));
    float final_g = Pixel_Max(0.0f, Pixel_Min(255.0f, surface_color.g * interpolated_attr->light_color.g));
    float final_b = Pixel_Max(0.0f, Pixel_Min(255.0f, surface_color.b * interpolated_attr->light_color.b));

    return ((uint32_t)surface_color.a << 24) |
                          ((uint32_t)final_r << 16) |
                          ((uint32_t)final_g << 8) |
                          (uint32_t)final_b;
}

static void Pixel_DrawTriangleGeneric(PixelContext *ctx, VertexOutput v0, VertexOutput v1, VertexOutput v2,
                               const PixelTexture *texture, bool use_texture, Material material) {

                                   // PixelTimer timer = Pixel_TimerStart();

    // Calculate the Triangle Bounding Box
    PixelBoundingBox bounding_box = Pixel_ComputeTriangleBoundingBox(ctx, v0, v1, v2);
    // // ctx->benchmark.bounding_box += Pixel_TimerElapsedMs(&timer);
    if (bounding_box.x_min > bounding_box.x_max || bounding_box.y_min > bounding_box.y_max) return;

    // // PixelTimer area_setup_timer = Pixel_TimerStart();
    float triangle_area = Pixel_Vec3EdgeCross(v0.screen_pos, v1.screen_pos, v2.screen_pos);
    if (fabsf(triangle_area) < 0.0001f) return;
    float inv_area = 1.0f/triangle_area;
    Vec3 p0 = {(float)bounding_box.x_min, (float)bounding_box.y_min, 0};
    float w0_row = Pixel_Vec3EdgeCross(v1.screen_pos, v2.screen_pos, p0);
    float w1_row = Pixel_Vec3EdgeCross(v2.screen_pos, v0.screen_pos, p0);
    float w2_row = Pixel_Vec3EdgeCross(v0.screen_pos, v1.screen_pos, p0);

    PixelBaryWeightDiff delta = Pixel_ComputeBaryWeightDiff(v0, v1, v2);
    PixelInterpolatedAttributes interpolated_attr = {0};

    // // ctx->benchmark.edge_cross_setup += Pixel_TimerElapsedMs(&area_setup_timer);

    for (int y = bounding_box.y_min; y <= bounding_box.y_max; ++y) {
        float w0 = w0_row;
        float w1 = w1_row;
        float w2 = w2_row;

        for (int x = bounding_box.x_min; x <= bounding_box.x_max; ++x) {
            // ctx->benchmark.pixel_tested++;

            // Inside-triangle coverage test
            bool inside = (triangle_area > 0) ? (w0 >= 0 && w1 >= 0 && w2 >= 0)
                                              : (w0 <= 0 && w1 <= 0 && w2 <= 0);
            if (inside) {
                // ctx->benchmark.pixel_inside++;
                float alpha = w0 * inv_area;
                float beta  = w1 * inv_area;
                float gamma = w2 * inv_area;
                // // PixelTimer interPolation_timer = Pixel_TimerStart();
                Pixel_InterpolateAttributes(v0, v1, v2, alpha, beta, gamma, &interpolated_attr);
                // // ctx->benchmark.attribute_interpolation += Pixel_TimerElapsedMs(&interPolation_timer);
                int index = y * ctx->width + x;
                if (ctx->depth_buffer[index] > interpolated_attr.depth) {
                    // ctx->benchmark.pixel_writtern++;
                    ctx->depth_buffer[index] = interpolated_attr.depth;
                    uint32_t fill_color = Pixel_ShadeFragment(ctx, texture, use_texture, &interpolated_attr);
                    ctx->back_buffer[index] = fill_color;
                }else {
                    // ctx->benchmark.pixel_depth_failed++;
                }
            }

            w0 += delta.w0_col;
            w1 += delta.w1_col;
            w2 += delta.w2_col;
        }

        w0_row += delta.w0_row;
        w1_row += delta.w1_row;
        w2_row += delta.w2_row;
    }
    // ctx->benchmark.triangle_rendering += Pixel_TimerElapsedMs(&timer);

    // // // printf("[Triangle Render]: %f\n", Pixel_TimerElapsedMs(&traingle_timer));
}


static inline Vec3 TransformVertexToScreen(Vec3 local_pos, Mat4 mvp, int width, int height) {
    Vec4 clip = Pixel_Mat4MulVec4(mvp, Pixel_Vec3ToVec4(local_pos));
    Vec3 ndc = Pixel_Vec4ToVec3(clip);
    Vec3 screen;
    screen.x = (ndc.x + 1.0f) * 0.5f * (float)width;
    screen.y = (1.0f - ndc.y) * 0.5f * (float)height;
    screen.z = ndc.z;
    return screen;
}

static inline VertexOutput Pixel_ComputeVertexOutput(Vec3 screen_pos, Mat4 mv_matrix, Mat4 normal_matrix, const Light *view_lights, int light_count, Material material, PixelVertex raw) {
    // // PixelTimer out_vertex = Pixel_TimerStart();
    VertexOutput out = {0};
    out.screen_pos = screen_pos;
    out.view_pos = Pixel_Vec4ToVec3(Pixel_Mat4MulVec4(mv_matrix, Pixel_Vec3ToVec4(raw.coordinate)));
    out.view_normal = Pixel_Vec4ToVec3(Pixel_Mat4MulVec4(normal_matrix, Pixel_Vec3ToVec4(raw.normal)));
    out.light_factor = Pixel_CalculateVertexLighting(out.view_pos, out.view_normal, view_lights, light_count, material);
    out.uv = raw.uv;
    out.base_color = raw.color;
    // // printf("[Single Out Vertex Calculation]: %f\n", Pixel_TimerElapsedMs(&out_vertex));
    return out;
}

// static void Pixel_DrawMesh(PixelContext *ctx, const PixelMesh *mesh, PixelTransform transform,
//                            Mat4 view_matrix, Mat4 projection_matrix,
//                            const Light *view_lights, int light_count, Material material) {

//     if (!ctx || !mesh || !mesh->vertices || !mesh->triangles) return;

//     // PixelTimer timer = Pixel_TimerStart();

//     // 1. Compute Transformation Matrices
//     Mat4 scale_mat = Pixel_Mat4ScaleMatrix((Vec3){transform.scale, transform.scale, transform.scale});
//     Mat4 rot_mat = Pixel_Mat4RotationMatrixXYZ(transform.rotation);
//     Mat4 trans_mat = Pixel_Mat4TranslationMatrix(transform.position);
//     Mat4 model_matrix = Pixel_Mat4Mul(trans_mat, Pixel_Mat4Mul(rot_mat, scale_mat));

//     Mat4 mv_matrix = Pixel_Mat4Mul(view_matrix, model_matrix);
//     Mat4 mvp = Pixel_Mat4Mul(projection_matrix, mv_matrix);
//     Mat4 normal_matrix = Pixel_ComputeNormalMatrix(mv_matrix);

//     for (int i = 0; i < mesh->triangle_count; i++) {
//         PixelTriangle tri = mesh->triangles[i];
//         PixelVertex raw_v0 = mesh->vertices[tri.vertices[0]];
//         PixelVertex raw_v1 = mesh->vertices[tri.vertices[1]];
//         PixelVertex raw_v2 = mesh->vertices[tri.vertices[2]];

//         // Backface Culling Check (Using Screen Space Area)
//         Vec3 s0 = TransformVertexToScreen(raw_v0.coordinate, mvp, ctx->width, ctx->height);
//         Vec3 s1 = TransformVertexToScreen(raw_v1.coordinate, mvp, ctx->width, ctx->height);
//         Vec3 s2 = TransformVertexToScreen(raw_v2.coordinate, mvp, ctx->width, ctx->height);

//         float area = (s1.x - s0.x) * (s2.y - s0.y) - (s1.y - s0.y) * (s2.x - s0.x);
//         if (ctx->cull_backface && area >= 0.0f) continue; // Counter-clockwise front-face culling

//         // ctx->benchmark.drawn_traingle_count++;

//         switch (ctx->drawing_mode) {
//             case PIXEL_VERTEX: {
//                 // Draw only the vertices as points or small markers
//                 Pixel_PutPixel(ctx, (uint32_t)s0.x, (uint32_t)s0.y, Pixel_Color2Uint32(raw_v0.color));
//                 Pixel_PutPixel(ctx, (uint32_t)s1.x, (uint32_t)s1.y, Pixel_Color2Uint32(raw_v1.color));
//                 Pixel_PutPixel(ctx, (uint32_t)s2.x, (uint32_t)s2.y, Pixel_Color2Uint32(raw_v2.color));
//                 break;
//             }
//             case PIXEL_WIREFRAME: {
//                 // Draw lines connecting the triangle vertices
//                 Pixel_DrawLine(ctx, (uint32_t)s0.x, (uint32_t)s0.y, (uint32_t)s1.x, (uint32_t)s1.y, raw_v0.color);
//                 Pixel_DrawLine(ctx, (uint32_t)s1.x, (uint32_t)s1.y, (uint32_t)s2.x, (uint32_t)s2.y, raw_v1.color);
//                 Pixel_DrawLine(ctx, (uint32_t)s2.x, (uint32_t)s2.y, (uint32_t)s0.x, (uint32_t)s0.y, raw_v2.color);
//                 break;
//             }
//             case PIXEL_SOLID: {
//                 // PixelTimer out_timer = Pixel_TimerStart();
//                 // Draw solid flat colors ignoring lighting calculations
//                 VertexOutput out0, out1, out2;
//                 out0.screen_pos = s0; out0.base_color = raw_v0.color; out0.light_factor = (Color){1,1,1,1}; out0.uv = raw_v0.uv;
//                 out1.screen_pos = s1; out1.base_color = raw_v1.color; out1.light_factor = (Color){1,1,1,1}; out1.uv = raw_v1.uv;
//                 out2.screen_pos = s2; out2.base_color = raw_v2.color; out2.light_factor = (Color){1,1,1,1}; out2.uv = raw_v2.uv;
//                 // ctx->benchmark.vertex_out_with_lighting += Pixel_TimerElapsedMs(&out_timer);
//                 // Pass base color with lighting factor forced to 1.0 (no shading)
//                 Pixel_DrawTriangleGeneric(ctx, out0, out1, out2, mesh->texture, mesh->has_texture, material);
//                 break;
//             }
//             case PIXEL_RENDER:
//             default: {
//                 // PixelTimer out_timer = Pixel_TimerStart();
//                 // Full rendering mode with lighting, textures, and material attributes enabled
//                 VertexOutput out0 = Pixel_ComputeVertexOutput(s0, mv_matrix, normal_matrix, view_lights, light_count, material, raw_v0);
//                 VertexOutput out1 = Pixel_ComputeVertexOutput(s1, mv_matrix, normal_matrix, view_lights, light_count, material, raw_v1);
//                 VertexOutput out2 = Pixel_ComputeVertexOutput(s2, mv_matrix, normal_matrix, view_lights, light_count, material, raw_v2);

//                 // ctx->benchmark.vertex_out_with_lighting += Pixel_TimerElapsedMs(&out_timer);

//                 Pixel_DrawTriangleGeneric(ctx, out0, out1, out2, mesh->texture, mesh->has_texture, material);

//                 break;
//             }
//         }
//     }

//     // ctx->benchmark.mesh_rendering += Pixel_TimerElapsedMs(&timer);
//     // // printf("[Mesh]: %f\n", Pixel_TimerElapsedMs(&timer));
// }

static void Pixel_DrawMesh(PixelContext *ctx, const PixelMesh *mesh, PixelTransform transform,
                           Mat4 view_matrix, Mat4 projection_matrix,
                           const Light *view_lights, int light_count, Material material) {

    if (!ctx || !mesh || !mesh->vertices || !mesh->triangles) return;

    // PixelTimer timer = Pixel_TimerStart();

    // 1. Compute Transformation Matrices
    Mat4 scale_mat = Pixel_Mat4ScaleMatrix((Vec3){transform.scale, transform.scale, transform.scale});
    Mat4 rot_mat = Pixel_Mat4RotationMatrixXYZ(transform.rotation);
    Mat4 trans_mat = Pixel_Mat4TranslationMatrix(transform.position);
    Mat4 model_matrix = Pixel_Mat4Mul(trans_mat, Pixel_Mat4Mul(rot_mat, scale_mat));

    Mat4 mv_matrix = Pixel_Mat4Mul(view_matrix, model_matrix);
    Mat4 mvp = Pixel_Mat4Mul(projection_matrix, mv_matrix);
    Mat4 normal_matrix = Pixel_ComputeNormalMatrix(mv_matrix);

    // =========================================================================
    // PHASE 1: Ensure buffer capacity and precompute all vertex outputs
    // =========================================================================
    if (ctx->out_vertices_capacity < mesh->vertex_count) {
        ctx->out_vertices_capacity = mesh->vertex_count;
        ctx->out_vertices = (VertexOutput*)realloc(ctx->out_vertices, ctx->out_vertices_capacity * sizeof(VertexOutput));
    }

    // PixelTimer out_timer = Pixel_TimerStart();
    for (int i = 0; i < mesh->vertex_count; i++) {
        PixelVertex raw_v = mesh->vertices[i];
        Vec3 screen_pos = TransformVertexToScreen(raw_v.coordinate, mvp, ctx->width, ctx->height);

        if (ctx->drawing_mode == PIXEL_SOLID) {
            VertexOutput out;
            out.screen_pos = screen_pos;
            out.base_color = raw_v.color;
            out.light_factor = (Color){1, 1, 1, 1};
            out.uv = raw_v.uv;
            ctx->out_vertices[i] = out;
        } else if (ctx->drawing_mode == PIXEL_RENDER) {
            ctx->out_vertices[i] = Pixel_ComputeVertexOutput(screen_pos, mv_matrix, normal_matrix, view_lights, light_count, material, raw_v);
        }
    }
    // ctx->benchmark.vertex_out_with_lighting += Pixel_TimerElapsedMs(&out_timer);

    // =========================================================================
    // PHASE 2: Triangle Traversal, Culling, and Drawing Dispatch
    // =========================================================================
    for (int i = 0; i < mesh->triangle_count; i++) {
        PixelTriangle tri = mesh->triangles[i];

        // Fetch precomputed vertex outputs using indices
        VertexOutput out0 = ctx->out_vertices[tri.vertices[0]];
        VertexOutput out1 = ctx->out_vertices[tri.vertices[1]];
        VertexOutput out2 = ctx->out_vertices[tri.vertices[2]];

        // Backface Culling Check (Using Screen Space Area from precomputed positions)
        float area = (out1.screen_pos.x - out0.screen_pos.x) * (out2.screen_pos.y - out0.screen_pos.y) -
                     (out1.screen_pos.y - out0.screen_pos.y) * (out2.screen_pos.x - out0.screen_pos.x);

        if (ctx->cull_backface && area >= 0.0f) continue; // Counter-clockwise front-face culling

        // ctx->benchmark.drawn_traingle_count++;

        switch (ctx->drawing_mode) {
            case PIXEL_VERTEX: {
                Pixel_PutPixel(ctx, (uint32_t)out0.screen_pos.x, (uint32_t)out0.screen_pos.y, Pixel_Color2Uint32(out0.base_color));
                Pixel_PutPixel(ctx, (uint32_t)out1.screen_pos.x, (uint32_t)out1.screen_pos.y, Pixel_Color2Uint32(out1.base_color));
                Pixel_PutPixel(ctx, (uint32_t)out2.screen_pos.x, (uint32_t)out2.screen_pos.y, Pixel_Color2Uint32(out2.base_color));
                break;
            }
            case PIXEL_WIREFRAME: {
                Pixel_DrawLine(ctx, (uint32_t)out0.screen_pos.x, (uint32_t)out0.screen_pos.y, (uint32_t)out1.screen_pos.x, (uint32_t)out1.screen_pos.y, out0.base_color);
                Pixel_DrawLine(ctx, (uint32_t)out1.screen_pos.x, (uint32_t)out1.screen_pos.y, (uint32_t)out2.screen_pos.x, (uint32_t)out2.screen_pos.y, out1.base_color);
                Pixel_DrawLine(ctx, (uint32_t)out2.screen_pos.x, (uint32_t)out2.screen_pos.y, (uint32_t)out0.screen_pos.x, (uint32_t)out0.screen_pos.y, out2.base_color);
                break;
            }
            case PIXEL_SOLID:
            case PIXEL_RENDER:
            default: {
                Pixel_DrawTriangleGeneric(ctx, out0, out1, out2, mesh->texture, mesh->has_texture, material);
                break;
            }
        }
    }

    // ctx->benchmark.mesh_rendering += Pixel_TimerElapsedMs(&timer);
}

void Pixel_RenderScene(PixelContext *ctx, const PixelScene *scene, Mat4 view, Mat4 projection) {
    if (!ctx || !scene)
        return;

    // PixelTimer timer = Pixel_TimerStart();

    // Dynamically allocate or use a safe stack/heap buffer for view lights to prevent overflow
    Light *view_lights = NULL;
    if (scene->light_count > 0) {
        view_lights = (Light *)malloc(sizeof(Light) * scene->light_count);
        if (!view_lights) return;
        Pixel_PrepareLights(scene->lights, view_lights, scene->light_count, view);
        // ctx->benchmark.light_transformation += Pixel_TimerElapsedMs(&timer);
    }

    // Loop through all instances in the scene array
    for (int i = 0; i < scene->instance_count; i++) {
        const PixelInstance *inst = &scene->instances[i];
        if (inst->mesh == NULL)
            continue;

        // Draw each mesh using its stored transform, material, view, projection, and prepared lights
        // ctx->benchmark.vertex_count += inst->mesh->vertex_count;
        // ctx->benchmark.triangle_count += inst->mesh->triangle_count;
        Pixel_DrawMesh(ctx, inst->mesh, inst->transform, view, projection, view_lights, scene->light_count, inst->material);
    }

    if (view_lights) {
        free(view_lights);
    }

    // ctx->benchmark.scene_rendering += Pixel_TimerElapsedMs(&timer);
    // // printf("[Render Scene]: %f\n", Pixel_TimerElapsedMs(&timer));


    // Pixel_PrintBenchmark(ctx);

}
