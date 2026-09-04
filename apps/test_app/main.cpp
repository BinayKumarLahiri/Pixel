#include "../../src/pixel.h"
#include <cstdint>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define WIDTH 600
#define HEIGHT 600

uint32_t *load_image_as_argb(const char *filename, int *out_width,
                             int *out_height) {
  if (!filename || !out_width || !out_height) {
    return nullptr;
  }

  int file_channels = 0;
  // Force stb_image to output 4 channels (RGBA memory layout: R, G, B, A
  // sequentially)
  int desired_channels = 4;

  unsigned char *raw_rgba_data = stbi_load(filename, out_width, out_height,
                                           &file_channels, desired_channels);

  if (!raw_rgba_data) {
    std::cerr << "stb_image failed to load: " << stbi_failure_reason()
              << std::endl;
    return nullptr;
  }

  int width = *out_width;
  int height = *out_height;
  size_t total_pixels = static_cast<size_t>(width) * height;

  // Allocate the destination buffer for our custom ARGB format
  uint32_t *argb_buffer = new uint32_t[total_pixels];

  // Convert raw sequential [R, G, B, A] memory into 32-bit integer 0xAARRGGBB
  for (size_t i = 0; i < total_pixels; ++i) {
    size_t raw_index = i * 4;

    uint32_t r = raw_rgba_data[raw_index];
    uint32_t g = raw_rgba_data[raw_index + 1];
    uint32_t b = raw_rgba_data[raw_index + 2];
    uint32_t a = raw_rgba_data[raw_index + 3];

    // Construct 0xAARRGGBB component by component
    argb_buffer[i] = (a << 24) | (r << 16) | (g << 8) | b;
  }

  // Free the temporary memory used by stb_image
  stbi_image_free(raw_rgba_data);

  return argb_buffer;
}
PixelMesh CreatePlaneMesh(float width, float height, int divs_x, int divs_y) {
    // 1. Capacity Math:
    // Number of vertices = (divisions + 1) along each axis because squares share edges.
    int vertex_count = (divs_x + 1) * (divs_y + 1);
    // Number of triangles = squares (divs_x * divs_y) * 2 triangles per square.
    int triangle_count = divs_x * divs_y * 2;

    PixelVertex* vertices = (PixelVertex*)malloc(vertex_count * sizeof(PixelVertex));
    PixelTriangle* triangles = (PixelTriangle*)malloc(triangle_count * sizeof(PixelTriangle));

    // 2. Vertex Generation Math:
    int v_idx = 0;
    for (int y = 0; y <= divs_y; y++) {
        // v goes from 0.0 to 1.0 across the height divisions
        float v = (float)y / (float)divs_y;
        // Map v to [-height/2, height/2] to center the plane at the origin (0,0,0)
        float z = (v - 0.5f) * height;

        for (int x = 0; x <= divs_x; x++) {
            // u goes from 0.0 to 1.0 across the width divisions
            float u = (float)x / (float)divs_x;
            // Map u to [-width/2, width/2]
            float px = (u - 0.5f) * width;

            // 3D Position vector in local space
            Vec3 coord = {px, 0.0f, z};

            // Flat plane facing straight up along the positive Y axis
            Vec3 normal = {0.0f, 1.0f, 0.0f};
            Color color = WHITE;

            // UV texture mapping coordinates directly use normalized u, v [0, 1]
            Vec2 uv = {u, v};

            vertices[v_idx++] = (PixelVertex){coord, normal, color, uv};
        }
    }

    // 3. Triangle Index & Winding Order Math:
    int t_idx = 0;
    for (int y = 0; y < divs_y; y++) {
        for (int x = 0; x < divs_x; x++) {
            // Calculate flat array index offsets for the 4 corners of the current grid square.
            // Row stride is (divs_x + 1) because there are divs_x squares, meaning divs_x + 1 vertices per row.
            int i0 = x + y * (divs_x + 1);             // Bottom-Left
            int i1 = (x + 1) + y * (divs_x + 1);         // Bottom-Right
            int i2 = x + (y + 1) * (divs_x + 1);         // Top-Left
            int i3 = (x + 1) + (y + 1) * (divs_x + 1);     // Top-Right

            // Split the square cell into two triangles.
            // Winding order is kept Counter-Clockwise (i0 -> i2 -> i3 and i0 -> i3 -> i1)
            // so that surface normals pointing to +Y pass backface culling checks.
            triangles[t_idx++] = (PixelTriangle){{i0, i2, i3}, 0};
            triangles[t_idx++] = (PixelTriangle){{i0, i3, i1}, 0};
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
float GetTerrainHeight(float x, float z) {
    return sinf(x * 0.2f) * cosf(z * 0.2f) * 2.0f; // Example rolling hills
}

int main() {
  uint32_t buffer[WIDTH * HEIGHT];
  PixelContext *canvas = Pixel_Init(buffer, WIDTH, HEIGHT);
  if (!canvas) {
    std::cout << "Failed to initialize Pixel Context\n";
    return 1;
  }
  canvas->drawing_mode = PIXEL_WIREFRAME;
  canvas->cull_backface = true; // Disabled backface culling to ensure visibility from any angle
  canvas->enable_lighting = false;

  // Create a ground plane mesh
  int divs_x = 40;
  int divs_y = 40;
  PixelMesh plane = CreatePlaneMesh(60.0f, 60.0f, divs_x, divs_y);


  // PixelMesh plane = CreateCubeMesh();
  PixelInstance plane_inst;
  plane_inst.mesh = &plane;
  plane_inst.material = Material {
      {50.0f, 50.0f, 50.0f, 255.0f}, // Ambient
      BLUE,                          // Distinct diffuse color so it's clearly visible
      WHITE,                         // Specular
      256.0f                         // Shininess exponent
  };

  // Position the plane directly within the camera's view frustum (ahead on Z and slightly below on Y)
  plane_inst.transform.position = (Vec3){0.0f, -5.0f, -20.0f};
  plane_inst.transform.rotation = (Vec3){0.5f, 0.0f, 0.0f};
  plane_inst.transform.scale = 1.0f;

  int stride = divs_x + 1;
  float start_z = 2;
  for (int y = 0; y <= divs_y; y++) {
      float start_x = 0;
      for (int x = 0; x <= divs_x; x++) {
          int v_idx = x + y * stride;

          float c_x = plane.vertices[v_idx].coordinate.x;
          float c_z = plane.vertices[v_idx].coordinate.z;
          plane.vertices[v_idx].coordinate.y = GetTerrainHeight(start_x, start_z);
          start_x += 1.7;
      }
      start_z += 0.5;
  }

  PixelScene scene;
  scene.instance_count = 1;
  scene.instances = &plane_inst;
  scene.light_count = 0;
  scene.lights = nullptr;

  // Place the camera at the origin looking down the positive Z axis
  Mat4 view_matrix =
      Pixel_Mat4ViewMatrixTRS({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f});

  float fov = 60.0f;
  float fov_rad = fov * (PIXEL_PI / 180.0f);
  float aspect = (float)WIDTH / (float)HEIGHT;
  Mat4 proj_matrix =
      Pixel_Mat4PerspectiveProjectionMatrix(fov_rad, aspect, 0.1f, 100.0f);

  Pixel_Clear(canvas, 0xff212121); // Dark gray background to provide contrast with the plane
  Pixel_RenderScene(canvas, &scene, view_matrix, proj_matrix);
  Pixel_PutText(canvas, 10, 10, PIXEL_DEFAULT_FONT, WHITE, 1, "Vertex Count: %d\nTriangle Count: %d", plane.vertex_count, plane.triangle_count);
  Pixel_SwapBuffers(canvas);
  Pixel_CreateDirectories("./images");
  Pixel_SaveBMP(canvas, "./images/large_ground.bmp");

  Pixel_Destroy(canvas);
  return 0;
}
