#include "pixel.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

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
  pixelContext->out_vertices = (VertexOutput *)malloc(256 * sizeof(VertexOutput));
  pixelContext->out_vertices_capacity = 256;

  for(uint32_t i=0;i<width*height;++i) pixelContext->depth_buffer[i] = 1.0;
  // The default DrawMode of the library is set to PixelRender i.e.
  // Drawing full lighting, shading, colors as they are intended to be drawn
  pixelContext->drawing_mode = PIXEL_RENDER;
  pixelContext->filter_mode = PIXEL_BILINEAR;
  pixelContext->cull_backface = true;
  pixelContext->enable_lighting = true;
  pixelContext->benchmark = {0};

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
  free(ctx->out_vertices);
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
    // ctx->back_buffer[y * ctx->width + x] = color;
    ctx->back_buffer[y * ctx->width + x] = Pixel_AlphaBlendUint32(color, ctx->back_buffer[y * ctx->width + x]);
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
