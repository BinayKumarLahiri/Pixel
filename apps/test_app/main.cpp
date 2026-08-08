// --- Standard Library Build Test ---
// When developing, include the raw header from the src directory
#include "../../src/pixel.h"
#include <cstdint>
#include <iostream>

// --- Single Header Release Test ---
// To test the STB-style header after running 'make dist',
// comment out the include above, and uncomment the two lines below:

// #define PIXEL_H_IMPLEMENTATION
// #include "../../dist/pixel.h"
#define WIDTH 	500
#define HEIGHT 	500

int main() {
	uint32_t buffer[WIDTH * HEIGHT];
	PixelContext* canvas = Pixel_Init(buffer, WIDTH, HEIGHT);
	if(!canvas) {
		std::cout << "Failed to initialize Pixel Context\n";
		return 1;
	}

	Pixel_Clear(canvas, 0xffffffff);
	// Pixel_DrawLine(canvas, 0, 0, 500, 400, RED);
	// Pixel_DrawLine(canvas, 0, 400, 500, 0, RED);
	// Pixel_DrawLine(canvas, 250, 0, 250, 400, GREEN);
	// Pixel_DrawLine(canvas, 0, 200, 500, 200, BLUE);

	// Pixel_DrawArcDegree(canvas, 250, 200, 100, 270, 90, WHITE);
	// Pixel_DrawArcDegree(canvas, 250, 200, 150, 0, 360, WHITE);
	// Pixel_DrawArc(canvas, 250, 200, 100, PIXEL_PIBY2, 3 * PIXEL_PI / 2, PURPLE);

	// Pixel_DrawFilledCircle(canvas, 250, 200, 100, GREEN);
	// Pixel_DrawCircle(canvas, 250, 200, 100, BLACK);

	// Pixel_DrawFilledRectangle(canvas, 0, 0, 450, 100, ORANGE);
	// Pixel_DrawFilledRectangle(canvas, 0, 100, 450, 200, WHITE);
	// Pixel_DrawFilledRectangle(canvas, 0, 200, 450, 300, GREEN);

	// Pixel_DrawFilledCircleV(canvas, {225, 150, 0}, 48, BLUE);

	Pixel_DrawTriangleShaded(canvas, 250, 10, 10, 480, 490, 480, RED, GREEN, BLUE, 1.0, 1.0, 1.0);
	Pixel_DrawTriangleLines(canvas, 250, 10, 10, 480, 490, 480, BLACK);
	Pixel_SwapBuffers(canvas);
	Pixel_SaveBMP(canvas, "./images/Line.bmp");
	Pixel_Destroy(canvas);
  return 0;
}
