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

int main() {
  uint32_t buffer[WIDTH * HEIGHT];
  PixelContext *canvas = Pixel_Init(buffer, WIDTH, HEIGHT);
  if (!canvas) {
    std::cerr << "Failed to initialize Pixel Context\n";
    return 1;
  }

  PixelTexture player;
  player.pixels = load_image_as_argb("./assets/player.png", &player.width, &player.height);

  if(!player.pixels || player.width <= 0 || player.height <= 0) {
    std::cerr << "Error Loading Texture\n";
    return 1;
  }

  Pixel_Clear(canvas, 0xff212121);
  Pixel_DrawRectangelTexturePro(canvas, 0, 0, 600, 600, 0, 0, 16, 16, &player, PIXEL_NEAREST);
  Pixel_SwapBuffers(canvas);
  Pixel_CreateDirectories("./images");
  Pixel_SaveBMP(canvas, "./images/texture.bmp");

  Pixel_Destroy(canvas);
  return 0;
}
