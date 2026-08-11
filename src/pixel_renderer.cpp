#include "pixel.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>

// -----------------------------------------------------------------
// 3D Rendering Related stuff (Vertex, Mesh, )
// -----------------------------------------------------------------

void Pixel_DrawTriangleDepthColor(PixelContext *ctx, float x0, float y0, float x1, float y1,
                              float x2, float y2, Color color0, Color color1,
                              Color color2, float z0, float z1, float z2) {
  // Keep full float precision for screen vertices!
  Vec3 v0 = {x0, y0, 0};
  Vec3 v1 = {x1, y1, 0};
  Vec3 v2 = {x2, y2, 0};

  int x_min = Pixel_Max(0, (int)floorf(Pixel_Min(Pixel_Min(x0, x1), x2)));
  int x_max = Pixel_Min(ctx->width - 1, (int)ceilf(Pixel_Max(Pixel_Max(x0, x1), x2)));
  int y_min = Pixel_Max(0, (int)floorf(Pixel_Min(Pixel_Min(y0, y1), y2)));
  int y_max = Pixel_Min(ctx->height - 1, (int)ceilf(Pixel_Max(Pixel_Max(y0, y1), y2)));

  // Early exit if bounding box is completely off-screen
  if (x_min > x_max || y_min > y_max) return;

  float delta_w0_col = v1.y - v2.y;
  float delta_w1_col = v2.y - v0.y;
  float delta_w2_col = v0.y - v1.y;

  float delta_w0_row = v2.x - v1.x;
  float delta_w1_row = v0.x - v2.x;
  float delta_w2_row = v1.x - v0.x;

  float triangle_area = Pixel_Vec3EdgeCross(v0, v1, v2);
  if (fabsf(triangle_area) < 0.0001f) return; // Degenerate triangle

  float r0 = color0.r, g0 = color0.g, b0 = color0.b, a0 = color0.a;
  float r1 = color1.r, g1 = color1.g, b1 = color1.b;
  float r2 = color2.r, g2 = color2.g, b2 = color2.b;

  Vec3 p0 = {(float)x_min, (float)y_min, 0};
  float w0_row = Pixel_Vec3EdgeCross(v1, v2, p0);
  float w1_row = Pixel_Vec3EdgeCross(v2, v0, p0);
  float w2_row = Pixel_Vec3EdgeCross(v0, v1, p0);

  for (int y = y_min; y <= y_max; ++y) {
    float w0 = w0_row;
    float w1 = w1_row;
    float w2 = w2_row;
    for (int x = x_min; x <= x_max; ++x) {
      float alpha = w0 / triangle_area;
      float beta = w1 / triangle_area;
      float gamma = w2 / triangle_area;

      // Check if pixel is inside the triangle (handling winding order rules)
      bool inside = (triangle_area > 0) ? (w0 >= 0 && w1 >= 0 && w2 >= 0)
                                        : (w0 <= 0 && w1 <= 0 && w2 <= 0);

      // bool inside = true;

      if (inside) {
        float z = z0 * alpha + z1 * beta + z2 * gamma;
        float r = r0 * alpha + r1 * beta + r2 * gamma;
        float g = g0 * alpha + g1 * beta + g2 * gamma;
        float b = b0 * alpha + b1 * beta + b2 * gamma;

        uint32_t fill_color = ((uint32_t)a0 << 24) |
                              ((uint32_t)r << 16) |
                              ((uint32_t)g << 8)  |
                              (uint32_t)b;

        int index = y * ctx->width + x;

        // printf("z = %f\n", z);
        if (ctx->depth_buffer[index] > z) {
            ctx->depth_buffer[index] = z;
            ctx->back_buffer[index] = fill_color;
        }
        // ctx->back_buffer[index] = fill_color;
      }
      w0 += delta_w0_col;
      w1 += delta_w1_col;
      w2 += delta_w2_col;
    }
    w0_row += delta_w0_row;
    w1_row += delta_w1_row;
    w2_row += delta_w2_row;
  }
}

Mat4 Pixel_ComputeMVP(PixelTransform transform, Mat4 view_matrix, Mat4 projection_matrix) {
    // 1. Create transformation matrices from transform components
    // (Note: scale is a single float here, so we apply it uniformly to X, Y, Z)
    Mat4 scale_mat = Pixel_Mat4ScaleMatrix((Vec3){transform.scale, transform.scale, transform.scale});
    Mat4 rot_mat   = Pixel_Mat4RotationMatrixXYZ(transform.rotation);
    Mat4 trans_mat = Pixel_Mat4TranslationMatrix(transform.position);

    // 2. Model Matrix = Translation * Rotation * Scale
    Mat4 rot_scale    = Pixel_Mat4Mul(rot_mat, scale_mat);
    Mat4 model_matrix = Pixel_Mat4Mul(trans_mat, rot_scale);

    // 3. MVP = Projection * View * Model
    Mat4 vp = Pixel_Mat4Mul(projection_matrix, view_matrix);
    Mat4 mvp = Pixel_Mat4Mul(vp, model_matrix);

    return mvp;
}

static Vec3 TransformVertexToScreen(Vec3 local_pos, Mat4 mvp, int width, int height) {
    Vec4 clip = Pixel_Mat4MulVec4(mvp, Pixel_Vec3ToVec4(local_pos));
    Vec3 ndc = Pixel_Vec4ToVec3(clip); // Handles perspective division (x/w, y/w, z/w)

    Vec3 screen;
    screen.x = (ndc.x + 1.0f) * 0.5f * (float)width;
    screen.y = (1.0f - ndc.y) * 0.5f * (float)height; // Inverted Y for top-left window origin
    screen.z = ndc.z; // Depth for Z-buffer
    return screen;
}

void Pixel_DrawMesh(PixelContext *ctx, const PixelMesh *mesh, PixelTransform transform, Mat4 view_matrix, Mat4 projection_matrix) {
    if (!ctx || !mesh || !mesh->vertices || !mesh->triangles) return;

    Mat4 mvp = Pixel_ComputeMVP(transform, view_matrix, projection_matrix);

    for (int i = 0; i < mesh->triangle_count; i++) {
        PixelTriangle tri = mesh->triangles[i];

        // Getting the 3 Vertices of the currently drawing triangle
        PixelVertex v0 = mesh->vertices[tri.vertices[0]];
        PixelVertex v1 = mesh->vertices[tri.vertices[1]];
        PixelVertex v2 = mesh->vertices[tri.vertices[2]];

        // Transforming local coordinates to Screen Space
        Vec3 s0 = TransformVertexToScreen(v0.coordinate, mvp, ctx->width, ctx->height);
        Vec3 s1 = TransformVertexToScreen(v1.coordinate, mvp, ctx->width, ctx->height);
        Vec3 s2 = TransformVertexToScreen(v2.coordinate, mvp, ctx->width, ctx->height);

        float area = (s1.x - s0.x) * (s2.y - s0.y) - (s1.y - s0.y) * (s2.x - s0.x);

        // one sign means it's facing away from the screen.
        // If area >= 0 (since using counter-clockwise front faces) skip it
        if (area >= 0.0f) {
            continue;
        }

        // Switching the drawing style based on the context drawing mode
        switch (ctx->drawing_mode) {
            case PIXEL_VERTEX:
                Pixel_PutPixel(ctx, s0.x, s0.y, Pixel_Color2Uint32(v0.color));
                Pixel_PutPixel(ctx, s1.x, s1.y, Pixel_Color2Uint32(v1.color));
                Pixel_PutPixel(ctx, s2.x, s2.y, Pixel_Color2Uint32(v2.color));
                break;
            case PIXEL_WIREFRAME:
                Pixel_DrawTriangleLines(ctx, s0.x, s0.y, s1.x, s1.y, s2.x, s2.y, WHITE);
                break;

            // NOTE: The above draw_modes does not use z_buffers yet, cause the methods used here doesn't either
            case PIXEL_SOLID:
            case PIXEL_RENDER:
                Pixel_DrawTriangleDepthColor(ctx,
                    s0.x, s0.y,
                    s1.x, s1.y,
                    s2.x, s2.y,
                    v0.color, v1.color, v2.color,
                    s0.z, s1.z, s2.z
                );
                break;
            default:
                printf("Invalid Drawing Mode.");
                exit(1);
        }
    }
}

void Pixel_RenderScene(PixelContext *ctx, const PixelInstance *scene, int instance_count, Mat4 view, Mat4 projection) {
    if (!ctx || !scene) return;
    for (int i = 0; i < instance_count; i++) {
        if (scene[i].mesh == NULL) continue;
        Pixel_DrawMesh(ctx, scene[i].mesh, scene[i].transform, view, projection);
    }
}
