#include "../../src/pixel.h"
#include <cstdint>
#include <cstdio>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "../../src/stb_image.h"

#define WIDTH 600
#define HEIGHT 800

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

int main() {
  uint32_t buffer[WIDTH * HEIGHT];
  PixelContext *canvas = Pixel_Init(buffer, WIDTH, HEIGHT);
  if (!canvas) {
    std::cout << "Failed to initialize Pixel Context\n";
    return 1;
  }

  const char *image_path =
      "C:\\Users\\binay\\OneDrive\\Documents\\Pixel\\textures\\elve.png";
  // const char* image_path =
  // "C:\\Users\\binay\\OneDrive\\Documents\\Pixel\\Calcata+(glb)\\textures\\calcata.jpg";
  int width = 0;
  int height = 0;
  uint32_t *image = load_image_as_argb(image_path, &width, &height);

  if (!image) {
    printf("Failed to load texture\n");
    return 1;
  }

  PixelTexture tex = {image, width, height};
  printf("Texture Loaded\n\tWidth: %d\n\tHeight: %d\n", tex.width, tex.height);

  canvas->drawing_mode = PIXEL_SOLID;
  canvas->cull_backface = true;
  canvas->enable_lighting = true;

  Material shiny_material = {
      {10.0, 10.0, 10.0, 255.0}, // Ambient (slight bluish tint in shadow)
      WHITE,                     // Diffuse (Base Blue color)
      WHITE,                     // Specular (White highlight flare)
      256.0f                     // Shininess exponent
  };

  // PixelMesh cube_mesh = CreateCubeMesh();
  // PixelMesh cube_mesh =
  // Pixel_LoadObjMesh("C:\\Binary\\Coding\\Projects\\Pixel\\assets\\LowPolyCars.obj");
  PixelMesh cube_mesh = Pixel_LoadObjMesh(
      "C:\\Users\\binay\\OneDrive\\Documents\\Pixel\\textures\\elve.obj");
  // PixelMesh cube_mesh =
  // Pixel_LoadObjMesh("C:\\Users\\binay\\OneDrive\\Documents\\Pixel\\Calcata+(glb)\\textures\\calcata.obj");
  if (cube_mesh.vertex_count <= 0) {
    printf("Model Loading failed.");
    return 1;
  }

  PixelInstance cube_instance;
  cube_instance.mesh = &cube_mesh;
  cube_instance.transform.position = (Vec3){0.0f, -0.2f, -1.6f};
  cube_instance.transform.rotation = (Vec3){0.0f, 0.2f, 0.0f};
  cube_instance.transform.scale = 2.0f;
  cube_instance.material = shiny_material;
  cube_instance.mesh->texture = &tex;
  cube_instance.mesh->has_texture = true;

  Light scene_lights[4] = {
      {
          POINT_LIGHT,          // Type 1 = Point Light
          {-2.5f, 3.0f, -1.5f}, // World position
          {0.0f, 0.0f, 0.0f},   // Direction (unused for point lights)
          RED,                  // Color
          6.0f                  // Intensity multiplier
      },
      {
          POINT_LIGHT,         // Type 1 = Point Light
          {2.5f, 3.0f, -1.5f}, // World position
          {0.0f, 0.0f, 0.0f},  // Direction (unused for point lights)
          BLUE,                // Color
          6.0f                 // Intensity multiplier
      },
      {
          POINT_LIGHT,         // Type 1 = Point Light
          {0.0f, 0.0f, 10.0f}, // World position
          {0.0f, 0.0f, 0.0f},  // Direction (unused for point lights)
          GREEN,               // Color
          4.0f                 // Intensity multiplier
      },
      {AMBIENT_LIGHT, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, WHITE, 20.0},
  };

  PixelScene scene;
  scene.instances = &cube_instance;
  scene.instance_count = 1;
  scene.lights = scene_lights;
  scene.light_count = 4;

  Mat4 view_matrix =
      Pixel_Mat4ViewMatrixTRS({0.0f, 0.0f, -0.0f}, {0.0f, 0.0f, 0.0f});

  float fov = 60.0f;
  float fov_rad = fov * (PIXEL_PI / 180.0f);
  float aspect = (float)WIDTH / (float)HEIGHT;
  Mat4 proj_matrix =
      Pixel_Mat4PerspectiveProjectionMatrix(fov_rad, aspect, 0.1f, 100.0f);

  // Pixel_Clear(canvas, 0xff212121); // Dark gray background
  // Pixel_Clear(canvas, 0xFFADD8E6); // Dark gray background
  Pixel_Clear(canvas, 0xff000000); // Dark gray background
  Pixel_RenderScene(canvas, &scene, view_matrix, proj_matrix);
  Pixel_SwapBuffers(canvas);
  Pixel_CreateDirectories("./images");
  Pixel_SaveBMP(canvas, "./images/elf_bilinear.bmp");

  delete[] image;
  Pixel_Destroy(canvas);
  return 0;
}
