#include "pixel.h"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <math.h>
#include <stdio.h>

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
// Vector and Maths Related stuff
// -----------------------------------------------------------------
float Pixel_Max(float val1, float val2) { return val1 > val2 ? val1 : val2; }

float Pixel_Min(float val1, float val2) { return val1 > val2 ? val2 : val1; }

float Pixel_Lerp(float start, float end, float alpha) {
  return start + (end - start) * alpha;
}

Vec3 Pixel_Vec3Add(const Vec3 vector1, const Vec3 vector2) {
  return Vec3{vector1.x + vector2.x, vector1.y + vector2.y,
              vector1.z + vector2.z};
}

Vec3 Pixel_Vec3Sub(const Vec3 vector1, const Vec3 vector2) {
  return Vec3{vector1.x - vector2.x, vector1.y - vector2.y,
              vector1.z - vector2.z};
}

Vec3 Pixel_Vec3Mul(const Vec3 vector1, float scale) {
  return Vec3{vector1.x * scale, vector1.y * scale, vector1.z * scale};
}

float Pixel_Vec3Len(const Vec3 vector) {
  return sqrtf(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
}

Vec3 Pixel_Vec3Norm(const Vec3 vector) {
  float len = Pixel_Vec3Len(vector);
  return Vec3{vector.x / len, vector.y / len, vector.z / len};
}

float Pixel_Vec3Dot(const Vec3 vector1, const Vec3 vector2) {
  return vector1.x * vector2.x + vector1.y * vector2.y + vector1.z * vector2.z;
}

Vec3 Pixel_Vec3Cross(const Vec3 vector1, const Vec3 vector2) {
  return Vec3{vector1.y * vector2.z - vector2.y * vector1.z,
              vector2.x * vector1.z - vector1.x * vector2.z,
              vector1.x * vector2.y - vector2.x * vector1.y};
}

void Pixel_Vec3Swap(Vec3 *vec1, Vec3 *vec2) {
  Vec3 temp = *(vec1);
  *vec1 = *vec2;
  *vec2 = temp;
}

float Pixel_Vec3EdgeCross(const Vec3 vector1, const Vec3 vector2,
                          const Vec3 vector3) {
  Vec3 v12 = Pixel_Vec3Sub(vector1, vector2);
  Vec3 v13 = Pixel_Vec3Sub(vector1, vector3);
  return (v12.x * v13.y) - (v12.y * v13.x);
}

Vec4 Pixel_Vec3ToVec4(Vec3 vector) {
  return Vec4{vector.x, vector.y, vector.z, 1.0};
}

void Pixel_Vec3ToVec4Ref(Vec3 source, Vec4 *dest) {
  if (dest != NULL) {
    dest->x = source.x;
    dest->y = source.y;
    dest->z = source.z;
    dest->w = 1.0;
  }
}

Vec3 Pixel_Vec4ToVec3(Vec4 vector) {
  if (vector.w != 1 && vector.w != 0) {
    return Vec3{vector.x / vector.w, vector.y / vector.w, vector.z / vector.w};
  }
  return Vec3{vector.x, vector.y, vector.z};
}


Mat4 Pixel_Mat4Identity() {
  Mat4 result = {0};
  result.m[0] = 1;
  result.m[5] = 1;
  result.m[10] = 1;
  result.m[15] = 1;
  return result;
}

Mat4 Pixel_Mat4Mul(Mat4 mat1, Mat4 mat2) {
  Mat4 result = {0};
  for (int col = 0; col < 4; ++col) {
    for (int row = 0; row < 4; ++row) {
      result.m[col * 4 + row] = mat1.m[0 * 4 + row] * mat2.m[col * 4 + 0] +
                                mat1.m[1 * 4 + row] * mat2.m[col * 4 + 1] +
                                mat1.m[2 * 4 + row] * mat2.m[col * 4 + 2] +
                                mat1.m[3 * 4 + row] * mat2.m[col * 4 + 3];
    }
  }
  return result;
}

Vec4 Pixel_Mat4MulVec4(Mat4 mat, Vec4 vec) {
  Vec4 result = {0};
  result.x = mat.m[0] * vec.x + mat.m[4] * vec.y + mat.m[8] * vec.z +
             mat.m[12] * vec.w;
  result.y = mat.m[1] * vec.x + mat.m[5] * vec.y + mat.m[9] * vec.z +
             mat.m[13] * vec.w;
  result.z = mat.m[2] * vec.x + mat.m[6] * vec.y + mat.m[10] * vec.z +
             mat.m[14] * vec.w;
  result.w = mat.m[3] * vec.x + mat.m[7] * vec.y + mat.m[11] * vec.z +
             mat.m[15] * vec.w;
  return result;
}


// Transformation Matrices
Mat4 Pixel_Mat4ScaleMatrix(Vec3 scale_factor) {
  Mat4 result = {0};
  result.m[0] = scale_factor.x;
  result.m[5] = scale_factor.y;
  result.m[10] = scale_factor.z;
  result.m[15] = 1.0;
  return result;
}

Mat4 Pixel_Mat4TranslationMatrix(Vec3 translation_values) {
  Mat4 result = {0};
  result.m[0] = 1;
  result.m[5] = 1;
  result.m[10] = 1;
  result.m[15] = 1;

  result.m[12] = translation_values.x;
  result.m[13] = translation_values.y;
  result.m[14] = translation_values.z;
  return result;
}

Mat4 Pixel_Mat4RotationMatrixX(float angle) {
  Mat4 result = {0};
  result.m[0] = 1;
  result.m[5] = cos(angle);
  result.m[6] = sin(angle);
  result.m[9] = -sin(angle);
  result.m[10] = cos(angle);
  result.m[15] = 1;
  return result;
}

Mat4 Pixel_Mat4RotationMatrixY(float angle) {
  Mat4 result = {0};
  result.m[5] = 1;
  result.m[0] = cos(angle);
  result.m[8] = sin(angle);
  result.m[2] = -sin(angle);
  result.m[10] = cos(angle);
  result.m[15] = 1;
  return result;
}

Mat4 Pixel_Mat4RotationMatrixZ(float angle) {
  Mat4 result = {0};
  result.m[10] = 1;
  result.m[0] = cos(angle);
  result.m[1] = sin(angle);
  result.m[4] = -sin(angle);
  result.m[5] = cos(angle);
  result.m[15] = 1;
  return result;
}

Mat4 Pixel_Mat4RotationMatrixXYZ(Vec3 angles_XYZ) {
  Mat4 x_rotate = Pixel_Mat4RotationMatrixX(angles_XYZ.x);
  Mat4 y_rotate = Pixel_Mat4RotationMatrixY(angles_XYZ.y);
  Mat4 z_rotate = Pixel_Mat4RotationMatrixZ(angles_XYZ.z);
  Mat4 result = Pixel_Mat4Mul(x_rotate, Pixel_Mat4Mul(y_rotate, z_rotate));
  return result;
}

Mat4 Pixel_Mat4ViewMatrixTRS(Vec3 position, Vec3 angles_XYZ) {
  Vec3 inverted_rotation = {-angles_XYZ.x, -angles_XYZ.y, -angles_XYZ.z};
  Vec3 inverted_translation = {-position.x, -position.y, -position.z};
  Mat4 rotation = Pixel_Mat4RotationMatrixXYZ(inverted_rotation);
  Mat4 translation = Pixel_Mat4TranslationMatrix(inverted_translation);
  Mat4 view_matrix = Pixel_Mat4Mul(translation, rotation);
  return view_matrix;
}

Mat4 Pixel_Mat4PerspectiveProjectionMatrix(float fov_radians, float aspect_ratio, float near_plane, float far_plane) {
    Mat4 result = {0};
    float tan_half_fov = tanf(fov_radians * 0.5f);

    result.m[0] = 1.0f / (aspect_ratio * tan_half_fov);
    result.m[5] = 1.0f / tan_half_fov;

    // MODIFIED FOR [0, 1] Range:
    result.m[10] = far_plane / (near_plane - far_plane);
    result.m[11] = -1.0f;
    result.m[14] = (near_plane * far_plane) / (near_plane - far_plane);

    return result;
}


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

// -----------------------------------------------------------------
// Rendering and PixelContext based Methods Implementations
// -----------------------------------------------------------------

PixelContext *Pixel_Init(uint32_t *external_buffer, uint32_t width, uint32_t height) {
  // PixelContext Creation begins here
  PixelContext *pixelContext = (PixelContext *)malloc(sizeof(PixelContext));
  if (!pixelContext) {
    printf("[Pixel] Failed to create PixelContext\n");
    return NULL;
  }

  pixelContext->width = width;
  pixelContext->height = height;

  // Creating the buffers for the front, back and depth buffers
  // the front buffer is provided by the user of this library and
  // the library itself handles the back buffer and the depth buffers
  // on its own
  pixelContext->front_buffer = external_buffer;
  pixelContext->back_buffer =
      (uint32_t *)malloc(width * height * sizeof(uint32_t));
  pixelContext->depth_buffer = (float *)malloc(width * height * sizeof(float));

  for(int i=0;i<width*height;++i) pixelContext->depth_buffer[i] = 1.0;
  // The default DrawMode of the library is set to PixelRender i.e.
  // Drawing full lighting, shading, colors as they are intended to be drawn
  pixelContext->drawing_mode = PIXEL_RENDER;
  return pixelContext;
}

void Pixel_Destroy(PixelContext *ctx) {
  if (!ctx)
    return;
  // Set the front buffer to NULL instead of freeing it cause the front buffer
  // is provided by the user and the PixelContext is not the owner of
  // the buffer
  ctx->front_buffer = NULL;
  // Free the memory assigned by the Initialization
  free(ctx->back_buffer);
  free(ctx->depth_buffer);
  // Free the PixelContext itself
  free(ctx);
}

void Pixel_SwapBuffers(PixelContext *ctx) {
  int buffer_size = ctx->width * ctx->height * sizeof(uint32_t);
  memcpy(ctx->front_buffer, ctx->back_buffer, buffer_size);
}

void Pixel_SetDrawMode(PixelContext *ctx, DrawMode draw_mode) {
  ctx->drawing_mode = draw_mode;
}

void Pixel_Clear(PixelContext *ctx, uint32_t color) {
  int total_pixels = ctx->width * ctx->height;
  for (int i = 0; i < total_pixels; i++) {
    ctx->back_buffer[i] = color;
    ctx->depth_buffer[i] = 1.0;
  }
}

void Pixel_PutPixel(PixelContext *ctx, uint32_t x, uint32_t y, uint32_t color) {
  if (x < ctx->width && y < ctx->height) {
    ctx->back_buffer[y * ctx->width + x] = color;
  }
}

void Pixel_SaveBMP(PixelContext *ctx, const char *filename) {
  uint32_t *buffer = ctx->front_buffer;
  int width = ctx->width;
  int height = ctx->height;

  // Create any subdirectories that may not exist and only after that
  // create the file
  Pixel_CreateDirectories(filename);

  printf("[Pixel] Saving the Pixel Buffer into the image file : %s\n",
         filename);
  int rowSize = (width * 3 + 3) & ~3; // Align rows to 4-byte boundaries
  int dataSize = rowSize * height;
  int fileSize = 54 + dataSize;

  uint8_t fileHeader[14] = {'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0};
  uint8_t infoHeader[40] = {40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0};

  // Pack file and size configurations into header array positions
  *(uint32_t *)(fileHeader + 2) = fileSize;
  *(int32_t *)(infoHeader + 4) = width;
  *(int32_t *)(infoHeader + 8) = height; // Positive = bottom-to-top layout
  *(uint32_t *)(infoHeader + 20) = dataSize;

  FILE *f = fopen(filename, "wb");
  fwrite(fileHeader, 1, 14, f);
  fwrite(infoHeader, 1, 40, f);

  uint8_t padding[3] = {0, 0, 0};
  int paddingSize = rowSize - (width * 3);

  // Write backwards (bottom-to-top) loop cycle matching BMP requirements
  for (int y = height - 1; y >= 0; --y) {
    for (int x = 0; x < width; ++x) {
      uint32_t p = buffer[y * width + x];
      uint8_t bgr[3] = {(uint8_t)((p >> 0) & 0xFF), (uint8_t)((p >> 8) & 0xFF),
                        (uint8_t)((p >> 16) & 0xFF)};
      fwrite(bgr, 1, 3, f);
    }
    if (paddingSize > 0) {
      fwrite(padding, 1, paddingSize, f);
    }
  }
  fclose(f);
  printf("[Pixel] File Save complete.");
}

void Pixel_DrawLine(PixelContext *ctx, uint32_t x0, uint32_t y0, uint32_t x1,
                    uint32_t y1, Color color) {
  uint32_t line_color = Pixel_Color2Uint32(color);
  int sx0 = (int)x0;
  int sy0 = (int)y0;
  int sx1 = (int)x1;
  int sy1 = (int)y1;

  if (abs(sx1 - sx0) > abs(sy1 - sy0)) {
    // Draw the Horizontal Line
    if (sx0 > sx1) {
      Pixel_SwapInt((uint32_t *)&sx0, (uint32_t *)&sx1);
      Pixel_SwapInt((uint32_t *)&sy0, (uint32_t *)&sy1);
    }

    int dx = sx1 - sx0;
    int dy = sy1 - sy0;
    int dir = dy < 0 ? -1 : 1;
    dy *= dir;

    if (dx != 0) {
      int y = sy0;
      int p = 2 * dy - dx;
      for (int i = 0; i <= dx; ++i) {
        Pixel_PutPixel(ctx, sx0 + i, y, line_color);
        if (p >= 0) {
          y += dir;
          p = p - 2 * dx;
        }
        p = p + 2 * dy;
      }
    }
  } else {
    // Draw the Vertical Line
    if (sy0 > sy1) {
      Pixel_SwapInt((uint32_t *)&sx0, (uint32_t *)&sx1);
      Pixel_SwapInt((uint32_t *)&sy0, (uint32_t *)&sy1);
    }

    int dx = sx1 - sx0;
    int dy = sy1 - sy0;

    int dir = dx < 0 ? -1 : 1;
    dx *= dir;

    if (dy != 0) {
      int x = sx0;
      int p = 2 * dx - dy;
      for (int i = 0; i <= dy; ++i) {
        Pixel_PutPixel(ctx, x, sy0 + i, line_color);
        if (p >= 0) {
          x += dir;
          p = p - 2 * dy;
        }
        p = p + 2 * dx;
      }
    }
  }
}

void Pixel_DrawLineV(PixelContext *ctx, Vec3 point1, Vec3 point2, Color color) {
  Pixel_DrawLine(ctx, point1.x, point1.y, point2.x, point2.y, color);
}

void Pixel_PlotArcPixel(PixelContext *ctx, int xc, int yc, int x, int y,
                        float start_rad, float end_rad, uint32_t color) {
  // Calculate current pixel angle in radians (-PI to PI)
  float angle = atan2(y - yc, x - xc);

  // Normalize angle to 0 to 2*PI if your inputs are 0 to 2*PI
  if (angle < 0)
    angle += 2 * PIXEL_PI;

  // Handle wrapping (e.g., arc from 350 degrees to 20 degrees)
  if (start_rad <= end_rad) {
    if (angle >= start_rad && angle <= end_rad)
      Pixel_PutPixel(ctx, x, y, color);
  } else { // Arc crosses the 0-degree boundary
    if (angle >= start_rad || angle <= end_rad)
      Pixel_PutPixel(ctx, x, y, color);
  }
}

void Pixel_DrawArc(PixelContext *ctx, int xc, int yc, int r, float start_angle,
                   float end_angle, Color color) {
  uint32_t arc_color = Pixel_Color2Uint32(color);
  int x = 0;
  int y = r;
  int p = 1 - r;

  while (x <= y) {
    // Pass all 8 symmetric points to the filter
    Pixel_PlotArcPixel(ctx, xc, yc, xc + x, yc + y, start_angle, end_angle,
                       arc_color); // Octant 2
    Pixel_PlotArcPixel(ctx, xc, yc, xc + y, yc + x, start_angle, end_angle,
                       arc_color); // Octant 1
    Pixel_PlotArcPixel(ctx, xc, yc, xc - x, yc + y, start_angle, end_angle,
                       arc_color); // Octant 3
    Pixel_PlotArcPixel(ctx, xc, yc, xc - y, yc + x, start_angle, end_angle,
                       arc_color); // Octant 4
    Pixel_PlotArcPixel(ctx, xc, yc, xc + x, yc - y, start_angle, end_angle,
                       arc_color); // Octant 7
    Pixel_PlotArcPixel(ctx, xc, yc, xc + y, yc - x, start_angle, end_angle,
                       arc_color); // Octant 8
    Pixel_PlotArcPixel(ctx, xc, yc, xc - x, yc - y, start_angle, end_angle,
                       arc_color); // Octant 6
    Pixel_PlotArcPixel(ctx, xc, yc, xc - y, yc - x, start_angle, end_angle,
                       arc_color); // Octant 5

    x++;
    if (p < 0) {
      p += 2 * x + 1;
    } else {
      y--;
      p += 2 * (x - y) + 1;
    }
  }
}

void Pixel_DrawArcV(PixelContext *ctx, Vec3 center, float r, float start_angle,
                    float end_angle, Color color) {
  Pixel_DrawArc(ctx, center.x, center.y, r, start_angle, end_angle, color);
}

void Pixel_DrawArcDegree(PixelContext *ctx, int xc, int yc, int r,
                         float start_angle, float end_angle, Color color) {
  float start_rad = start_angle * PIXEL_PI / 180.0;
  float end_rad = end_angle * PIXEL_PI / 180.0;
  Pixel_DrawArc(ctx, xc, yc, r, start_rad, end_rad, color);
}

void Pixel_DrawCircle(PixelContext *ctx, float xc, float yc, float r,
                      Color color) {
  Pixel_DrawArc(ctx, xc, yc, r, 0, 2 * PIXEL_PI, color);
}

void Pixel_DrawCircleV(PixelContext *ctx, Vec3 center, float r, Color color) {
  Pixel_DrawCircle(ctx, center.x, center.y, r, color);
}

void Pixel_DrawScanLine(PixelContext *ctx, int start_x, int end_x, int y,
                        Color color) {
  int start = start_x;
  int end = end_x;
  if (start_x > end_x) {
    start = end_x;
    end = start_x;
  }
  uint32_t scanline_color = Pixel_Color2Uint32(color);
  for (int x = start; x <= end; ++x) {
    Pixel_PutPixel(ctx, x, y, scanline_color);
  }
}

void Pixel_DrawFilledCircle(PixelContext *ctx, int xc, int yc, int r,
                            Color color) {
  int x = 0;
  int y = r;
  int p = 1 - r;

  while (x <= y) {
    Pixel_DrawScanLine(ctx, xc - x, xc + x, yc + y, color);
    Pixel_DrawScanLine(ctx, xc - y, xc + y, yc + x, color);
    Pixel_DrawScanLine(ctx, xc - x, xc + x, yc - y, color);
    Pixel_DrawScanLine(ctx, xc - y, xc + y, yc - x, color);
    x++;
    if (p < 0) {
      p += 2 * x + 1;
    } else {
      y--;
      p += 2 * (x - y) + 1;
    }
  }
}

void Pixel_DrawFilledCircleV(PixelContext *ctx, Vec3 center, int r,
                             Color color) {
  Pixel_DrawFilledCircle(ctx, center.x, center.y, r, color);
}

void Pixel_DrawRectangle(PixelContext *ctx, int x0, int y0, uint32_t width,
                         uint32_t height, Color color) {
  Pixel_DrawLine(ctx, x0, y0, x0 + width, y0, color);
  Pixel_DrawLine(ctx, x0 + width, y0, x0 + width, y0 + height, color);
  Pixel_DrawLine(ctx, x0 + width, y0 + height, x0, y0 + height, color);
  Pixel_DrawLine(ctx, x0, y0 + height, x0, y0, color);
}

void Pixel_DrawFilledRectangle(PixelContext *ctx, int x0, int y0,
                               uint32_t width, uint32_t height, Color color) {
  uint32_t fill_color = Pixel_Color2Uint32(color);
  for (uint32_t x = x0; x < x0 + width; ++x) {
    for (uint32_t y = y0; y < y0 + height; ++y) {
      Pixel_PutPixel(ctx, x, y, fill_color);
    }
  }
}

void Pixel_DrawTriangleLines(PixelContext *ctx, int x0, int y0, int x1, int y1,
                             int x2, int y2, Color color) {
  Pixel_DrawLine(ctx, x0, y0, x1, y1, color);
  Pixel_DrawLine(ctx, x1, y1, x2, y2, color);
  Pixel_DrawLine(ctx, x2, y2, x0, y0, color);
}

void Pixel_DrawTriangleShaded(PixelContext *ctx, int x0, int y0, int x1, int y1,
                              int x2, int y2, Color color0, Color color1,
                              Color color2, float h0, float h1, float h2) {
  Vec3 v0 = {(float)x0, (float)y0, 0};
  Vec3 v1 = {(float)x1, (float)y1, 0};
  Vec3 v2 = {(float)x2, (float)y2, 0};

  int x_min = Pixel_Min(Pixel_Min(x0, x1), x2);
  int x_max = Pixel_Max(Pixel_Max(x0, x1), x2);
  int y_min = Pixel_Min(Pixel_Min(y0, y1), y2);
  int y_max = Pixel_Max(Pixel_Max(y0, y1), y2);

  float delta_w0_col = v1.y - v2.y;
  float delta_w1_col = v2.y - v0.y;
  float delta_w2_col = v0.y - v1.y;

  float delta_w0_row = v2.x - v1.x;
  float delta_w1_row = v0.x - v2.x;
  float delta_w2_row = v1.x - v0.x;

  float triangle_area = Pixel_Vec3EdgeCross(v0, v1, v2);
  float r0 = color0.r;
  float g0 = color0.g;
  float b0 = color0.b;
  float r1 = color1.r;
  float g1 = color1.g;
  float b1 = color1.b;
  float r2 = color2.r;
  float g2 = color2.g;
  float b2 = color2.b;

  float a = color0.a;
  // uint32_t fill_color = Pixel_Color2Uint32(color);
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

      if ((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0)) {
        float h = h0 * alpha + h1 * beta + h2 * gamma;
        float r = (r0 * alpha + r1 * beta + r2 * gamma) * h;
        float g = (g0 * alpha + g1 * beta + g2 * gamma) * h;
        float b = (b0 * alpha + b1 * beta + b2 * gamma) * h;
        uint32_t fill_color = ((uint32_t)a << 24) | ((uint32_t)r << 16) |
                              ((uint32_t)g << 8) | (uint32_t)b;
        Pixel_PutPixel(ctx, x, y, fill_color);
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

void Pixel_DrawTriangleFilled(PixelContext *ctx, int x0, int y0, int x1, int y1,
                              int x2, int y2, Color color) {
  Pixel_DrawTriangleShaded(ctx, x0, y0, x1, y1, x2, y2, color, color, color,
                           1.0, 1.0, 1.0);
}

void Pixel_DrawTriangleFilledV(PixelContext *ctx, Vec3 point0, Vec3 point1,
                               Vec3 point2, Color color) {
  Pixel_DrawTriangleShaded(ctx, point0.x, point0.y, point1.x, point1.y,
                           point2.x, point2.y, color, color, color, 1.0, 1.0,
                           1.0);
}

// -----------------------------------------------------------------
// 3D Rendering Related stuff (Vertex, Mesh, )
// -----------------------------------------------------------------

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
