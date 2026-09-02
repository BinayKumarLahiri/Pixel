#include "../../src/pixel.h"
#include <cstdint>
#include <iostream>

#define WIDTH 600
#define HEIGHT 800

int main() {
  uint32_t buffer[WIDTH * HEIGHT];
  PixelContext *canvas = Pixel_Init(buffer, WIDTH, HEIGHT);
  if (!canvas) {
    std::cout << "Failed to initialize Pixel Context\n";
    return 1;
  }

  int FPS = 30;

  Pixel_Clear(canvas, 0xffffffff);
  Pixel_PutText(canvas, 10, 10, PIXEL_DEFAULT_FONT, BLACK, 1, "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG.\nthe quick brown fox jumps over the lazy dog.\nThe Quick Brown Fox Jumps Over The Lazy Dog.\n0123456789\n+-*/%%^!&#(){}[]<>:;\"'%d", FPS);
  Pixel_SwapBuffers(canvas);
  Pixel_CreateDirectories("./images");
  Pixel_SaveBMP(canvas, "./images/font_rendering.bmp");

  Pixel_Destroy(canvas);
  return 0;
}
