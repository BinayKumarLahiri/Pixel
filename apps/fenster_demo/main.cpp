#include "../../src/pixel.h"
#include <cstdint>
#include <cstdlib>
#include <iostream>

#define FENSTER_IMPLEMENTATION
#include "fenster.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define WIDTH 600
#define HEIGHT 800
#define TARGET_FPS 60.0f

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
float deg_to_rad(float deg) { return deg * PIXEL_PI / 180; }

int main() {
  uint32_t *buf = (uint32_t *)malloc(WIDTH * HEIGHT * sizeof(uint32_t));
  struct fenster f = {
      .title = "hello", .width = WIDTH, .height = HEIGHT, .buf = buf};

  PixelContext *canvas = Pixel_Init(buf, WIDTH, HEIGHT);
  if (!canvas) {
    std::cout << "Failed to initialize Pixel Context\n";
    return 1;
  }

  const char *image_path = "./assets/textures/elve.png";
  int width = 0;
  int height = 0;
  uint32_t *image = load_image_as_argb(image_path, &width, &height);

  if (!image) {
    printf("Failed to load texture\n");
    return 1;
  }
  PixelTexture tex = {image, width, height};
  printf("Texture Loaded\n\tWidth: %d\n\tHeight: %d\n", tex.width, tex.height);

  canvas->drawing_mode = PIXEL_RENDER;
  canvas->cull_backface = true;
  canvas->enable_lighting = true;

  Material shiny_material = {
      {10.0, 10.0, 10.0, 255.0}, // Ambient (slight bluish tint in shadow)
      WHITE,                     // Diffuse (Base Blue color)
      WHITE,                     // Specular (White highlight flare)
      256.0f                     // Shininess exponent
  };

  PixelMesh elve_mesh = Pixel_LoadObjMesh("./assets/textures/elve.obj");
  if (elve_mesh.vertex_count <= 0) {
    printf("Model Loading failed.");
    return 1;
  }

  float rotation_deg = 0.0f;
  float rotation_speed = 1.0f;

  PixelInstance elve;
  elve.mesh = &elve_mesh;
  elve.transform.position = (Vec3){0.0f, 0.1f, -2.2f};
  elve.transform.rotation = (Vec3){0.0f, deg_to_rad(rotation_deg), 0.0f};
  elve.transform.scale = 2.0f;
  elve.material = shiny_material;
  elve.mesh->texture = &tex;
  elve.mesh->has_texture = true;

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
  scene.instances = &elve;
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

  const float TARGET_FRAME_MS = 1000.0f / TARGET_FPS;
  fenster_open(&f);

  float max_fps = 0.0f;
  float min_fps = 1000000.0f; // Initialize to a very high number
  int frame_count = 0;

  while (fenster_loop(&f) == 0) {
    // 1. Start the timer at the beginning of the frame
    PixelTimer frame_timer = Pixel_TimerStart();

    // --- Game Logic & Update ---
    rotation_deg += rotation_speed;
    rotation_deg = rotation_deg >= 360.f ? 0.0f : rotation_deg;
    elve.transform.rotation.y = deg_to_rad(rotation_deg);

    // --- Render Processing ---
    Pixel_Clear(canvas, 0xff000000); // Dark gray background
    Pixel_RenderScene(canvas, &scene, view_matrix, proj_matrix);

    // Measure exact time taken for logic and rendering
    float frame_time_ms = Pixel_TimerElapsedMs(&frame_timer);

    Pixel_PutText(canvas, 10, 10, PIXEL_DEFAULT_FONT, WHITE, 1,
                  "Model Name: Elve.obj\nVertices Count: 61275\nTriangle "
                  "Count: 20419\nFPS: %f",
                  (1000.0f / frame_time_ms));
    Pixel_SwapBuffers(canvas);

    if (frame_time_ms > 0.0f) {
      float current_fps = 1000.0f / frame_time_ms;

      // Track max and min FPS (skipping the first couple frames if they spike
      // during startup)
      if (frame_count > 5) {
        if (current_fps > max_fps)
          max_fps = current_fps;
        if (current_fps < min_fps)
          min_fps = current_fps;
      }
    }
    frame_count++;

    // 2. Lock the frame rate: wait out the remaining time
    while (Pixel_TimerElapsedMs(&frame_timer) < TARGET_FRAME_MS) {
      // Busy-wait loop to precisely hold the frame rate.
    }
  }

  printf("Max FPS: %.2f | Min FPS: %.2f\n", max_fps, min_fps);
  fenster_close(&f);
  delete[] image;
  Pixel_Destroy(canvas);
  free(buf);
  return 0;
}
