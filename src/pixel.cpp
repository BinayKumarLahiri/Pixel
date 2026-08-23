#include "pixel.h"
#include <cmath>
#include <cstdio>

// -----------------------------------------------------------------
// Some Helper Methods useful for testing the code or building it
// -----------------------------------------------------------------






#if defined(_WIN32) || defined(__MINGW32__)
#include <windows.h>
// Windows API uses 1 argument and ignores POSIX permissions completely
#define MAKE_DIR(path)                                                         \
  (CreateDirectoryA(path, NULL) || GetLastError() == ERROR_ALREADY_EXISTS)
#else
#include <sys/stat.h>
#include <sys/types.h>
// POSIX systems (Linux/macOS) use 2 arguments
#define MAKE_DIR(path) (mkdir(path, 0777) == 0)
#endif

void Pixel_CreateDirectories(const char *path) {
  char tempPath[512];
  strncpy(tempPath, path, sizeof(tempPath) - 1);
  tempPath[sizeof(tempPath) - 1] = '\0';

  for (char *p = tempPath; *p; p++) {
    if (*p == '/' || *p == '\\') {
      if (p == tempPath)
        continue;
      char originalChar = *p;
      *p = '\0';          // Temporarily truncate the string
      MAKE_DIR(tempPath); // Invoke the safe macro
      *p = originalChar;  // Restore character
    }
  }
}

void Pixel_SwapInt(uint32_t *value1, uint32_t *value2) {
  int temp = *value1;
  *value1 = *value2;
  *value2 = temp;
}


// -----------------------------------------------------------------
// Simple Primitive 3D Shapes
// -----------------------------------------------------------------

PixelMesh CreateCubeMesh() {
    // 24 Vertices (4 independent vertices per face for flat-shaded/correct normals)
    static PixelVertex vertices[24] = {
        // --- Front Face (+Z) ---
        {{-0.5f, -0.5f,  0.5f}, {0,0,1}, WHITE, {0, 0}},
        {{ 0.5f, -0.5f,  0.5f}, {0,0,1}, WHITE, {1, 0}},
        {{ 0.5f,  0.5f,  0.5f}, {0,0,1}, WHITE, {1, 1}},
        {{-0.5f,  0.5f,  0.5f}, {0,0,1}, WHITE, {0, 1}},

        // --- Back Face (-Z) ---
        {{ 0.5f, -0.5f, -0.5f}, {0,0,-1}, WHITE, {0, 0}},
        {{-0.5f, -0.5f, -0.5f}, {0,0,-1}, WHITE, {1, 0}},
        {{-0.5f,  0.5f, -0.5f}, {0,0,-1}, WHITE, {1, 1}},
        {{ 0.5f,  0.5f, -0.5f}, {0,0,-1}, WHITE, {0, 1}},

        // --- Left Face (-X) ---
        {{-0.5f, -0.5f, -0.5f}, {-1,0,0}, WHITE, {0, 0}},
        {{-0.5f, -0.5f,  0.5f}, {-1,0,0}, WHITE, {1, 0}},
        {{-0.5f,  0.5f,  0.5f}, {-1,0,0}, WHITE, {1, 1}},
        {{-0.5f,  0.5f, -0.5f}, {-1,0,0}, WHITE, {0, 1}},

        // --- Right Face (+X) ---
        {{ 0.5f, -0.5f,  0.5f}, {1,0,0}, WHITE, {0, 0}},
        {{ 0.5f, -0.5f, -0.5f}, {1,0,0}, WHITE, {1, 0}},
        {{ 0.5f,  0.5f, -0.5f}, {1,0,0}, WHITE, {1, 1}},
        {{ 0.5f,  0.5f,  0.5f}, {1,0,0}, WHITE, {0, 1}},

        // --- Top Face (+Y) ---
        {{-0.5f,  0.5f,  0.5f}, {0,1,0}, WHITE, {0, 0}},
        {{ 0.5f,  0.5f,  0.5f}, {0,1,0}, WHITE, {1, 0}},
        {{ 0.5f,  0.5f, -0.5f}, {0,1,0}, WHITE, {1, 1}},
        {{-0.5f,  0.5f, -0.5f}, {0,1,0}, WHITE, {0, 1}},

        // --- Bottom Face (-Y) ---
        {{-0.5f, -0.5f, -0.5f}, {0,-1,0}, WHITE, {0, 0}},
        {{ 0.5f, -0.5f, -0.5f}, {0,-1,0}, WHITE, {1, 0}},
        {{ 0.5f, -0.5f,  0.5f}, {0,-1,0}, WHITE, {1, 1}},
        {{-0.5f, -0.5f,  0.5f}, {0,-1,0}, WHITE, {0, 1}}
    };

    static PixelTriangle triangles[12] = {
        {{0, 1, 2}, 0}, {{0, 2, 3}, 0},     // Front
        {{4, 5, 6}, 0}, {{4, 6, 7}, 0},     // Back
        {{8, 9, 10}, 0}, {{8, 10, 11}, 0},   // Left
        {{12, 13, 14}, 0}, {{12, 14, 15}, 0}, // Right
        {{16, 17, 18}, 0}, {{16, 18, 19}, 0}, // Top
        {{20, 21, 22}, 0}, {{20, 22, 23}, 0}  // Bottom
    };

    PixelMesh mesh;
    mesh.vertices = vertices;
    mesh.vertex_count = 24;
    mesh.triangles = triangles;
    mesh.triangle_count = 12;
    mesh.texture = nullptr;
    mesh.has_texture = false;
    return mesh;
}

PixelMesh CreateSphereMesh(float radius, int lats, int longs) {
    int vertex_capacity = (lats + 1) * (longs + 1);
    int triangle_capacity = lats * longs * 2;

    PixelVertex* vertices = (PixelVertex*)malloc(vertex_capacity * sizeof(PixelVertex));
    PixelTriangle* triangles = (PixelTriangle*)malloc(triangle_capacity * sizeof(PixelTriangle));

    int vertex_count = 0;
    int triangle_count = 0;

    // 1. Generate Vertices
    for (int i = 0; i <= lats; i++) {
        float theta = (float)i * (float)PIXEL_PI / (float)lats;
        float sinTheta = sinf(theta);
        float cosTheta = cosf(theta);

        for (int j = 0; j <= longs; j++) {
            float phi = (float)j * (2.0f * (float)PIXEL_PI / (float)longs);
            float sinPhi = sinf(phi);
            float cosPhi = cosf(phi);

            // Correct standard outward unit normal
            Vec3 normal = {
                cosPhi * sinTheta,
                cosTheta,
                sinPhi * sinTheta
            };
            normal = Pixel_Vec3Norm(normal);

            // Vertex position based on radius and normal
            Vec3 coord = {
                radius * normal.x,
                radius * normal.y,
                radius * normal.z
            };

            // Standard UV coordinates mapping
            Vec2 uv = {
                (float)j / (float)longs,
                (float)i / (float)lats
            };

            PixelVertex vertex = { coord, normal, WHITE, uv };
            vertices[vertex_count++] = vertex;
        }
    }

    // 2. Generate Triangles with standard consistent winding order
    for (int i = 0; i < lats; i++) {
        for (int j = 0; j < longs; j++) {
            int first = (i * (longs + 1)) + j;
            int second = first + longs + 1;

            // Correct triangle winding order for proper front-face culling and lighting
            PixelTriangle t1 = {{first, first + 1, second + 1}, 0};
            PixelTriangle t2 = {{first, second + 1, second}, 0};

            triangles[triangle_count++] = t1;
            triangles[triangle_count++] = t2;
        }
    }

    PixelMesh mesh;
    mesh.vertices = vertices;
    mesh.vertex_count = vertex_count;
    mesh.triangles = triangles;
    mesh.triangle_count = triangle_count;
    mesh.texture = NULL;
    mesh.has_texture = false;

    return mesh;
}

void Pixel_PrintBenchmark(PixelContext* ctx) {
    printf("\n\n\n");
    switch (ctx->drawing_mode) {
        case PIXEL_VERTEX:  printf("[DrawMode: PIXEL_VERTEX]\n");break;
        case PIXEL_SOLID:  printf("[DrawMode: PIXEL_SOLID]\n");break;
        case PIXEL_WIREFRAME:  printf("[DrawMode: PIXEL_WIREFRAME]\n");break;
        case PIXEL_RENDER:  printf("[DrawMode: PIXEL_RENDER]\n");break;
    }
    printf("\n[Geometry Data]\n");
    printf("[Total Vertices Processed]: %d\n", ctx->benchmark.vertex_count);
    printf("[Total Triangle Contained]: %d\n", ctx->benchmark.triangle_count);
    printf("[Total Triangle Drawn]: %d\n", ctx->benchmark.drawn_traingle_count);
    printf("\n[Timing Data]\n");
    printf("1. Scene Rendering: %f\n", ctx->benchmark.scene_rendering);
    printf("2. Mesh Rendering: %f\n", ctx->benchmark.mesh_rendering);
    printf("3. Light Transformation: %f\n", ctx->benchmark.light_transformation);
    printf("4. OutVertex Calculation: %f\n", ctx->benchmark.vertex_out_with_lighting);
    printf("5. Triangle Rendering: %f\n", ctx->benchmark.triangle_rendering);
    // printf("\t5.1. Bounding Box: %f\n", ctx->benchmark.bounding_box);
    // printf("\t5.2. Edge Cross Setup: %f\n", ctx->benchmark.edge_cross_setup);
    // printf("\t5.3. Attribute Interpolation: %f\n", ctx->benchmark.attribute_interpolation);
    printf("\t5.1. Pixel Tested: %d\n", ctx->benchmark.pixel_tested);
    printf("\t5.2. Pixel Inside: %d\n", ctx->benchmark.pixel_inside);
    printf("\t5.3. Pixel Depth Failed: %d\n", ctx->benchmark.pixel_depth_failed);
    printf("\t5.4. Pixel Written: %d\n", ctx->benchmark.pixel_writtern);
}
