#include "pixel.h"

// -----------------------------------------------------------------
// Defining Color related structs and methods
// -----------------------------------------------------------------

uint32_t Pixel_Color2Uint32(const Color color) {
  return ((uint32_t)color.a << 24) | ((uint32_t)color.r << 16) |
         ((uint32_t)color.g << 8) | (uint32_t)color.b;
}

Color Pixel_Uint322Color(const uint32_t color) {
  float a = (color >> 24) & 0xFF;
  float r = (color >> 16) & 0xFF;
  float g = (color >> 8) & 0xFF;
  float b = (color >> 0) & 0xFF;
  return Color{r, g, b, a};
}

Color Pixel_ColorScale(const Color color, float brightness) {
  // 1. Unpack channels assuming ARGB8888 structure (0xAARRGGBB)
  uint8_t a = color.a;
  uint8_t r = color.r;
  uint8_t g = color.g;
  uint8_t b = color.b;

  // 2. Multiply each RGB channel by brightness
  float new_r = (float)r * brightness;
  float new_g = (float)g * brightness;
  float new_b = (float)b * brightness;

  // 3. Clamp the values to the safe 0 - 255 range
  if (new_r > 255.0f)
    new_r = 255.0f;
  if (new_r < 0.0f)
    new_r = 0.0f;
  if (new_g > 255.0f)
    new_g = 255.0f;
  if (new_g < 0.0f)
    new_g = 0.0f;
  if (new_b > 255.0f)
    new_b = 255.0f;
  if (new_b < 0.0f)
    new_b = 0.0f;

  // 4. Repack channels back into a single 32-bit ARGB integer
  return Color{new_r, new_g, new_b, (float)a};
}

Color Pixel_ColorBlend(const Color color1, const Color color2, float alpha) {
  return Color{Pixel_Lerp(color1.r, color2.r, alpha),
               Pixel_Lerp(color1.g, color2.g, alpha),
               Pixel_Lerp(color1.b, color2.b, alpha),
               Pixel_Lerp(color1.a, color2.a, alpha)};
}
