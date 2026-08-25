#include "pixel.h"
#include <cmath>
#include <cstdint>

// -----------------------------------------------------------------
// Primitive Shapes Drawing Methods
// -----------------------------------------------------------------

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

void Pixel_DrawRectangleTexture(PixelContext *ctx, int x0, int y0,
                               uint32_t width, uint32_t height, PixelTexture *texture) {
    for(uint32_t dy = 0;dy < height; ++dy) {
        for(uint32_t dx = 0; dx < width; ++dx) {
            uint32_t x = x0 + dx;
            uint32_t y = y0 + dy;
            float u = (float)dx/(float)width;
            float v = (float)dy/(float)height;
            uint32_t color = Pixel_SampleTextureUint32(texture, u, v);
            Pixel_PutPixel(ctx, x, y, color);
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
