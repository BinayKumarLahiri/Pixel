#include "../../src/pixel.h"
#include <cstdint>
#include <iostream>

#define WIDTH  500
#define HEIGHT 500


int main() {
    uint32_t buffer[WIDTH * HEIGHT];
    PixelContext *canvas = Pixel_Init(buffer, WIDTH, HEIGHT);
    if (!canvas) {
        std::cout << "Failed to initialize Pixel Context[cite: 1]\n";
        return 1;
    }

    // Set to PIXEL_RENDER mode so Gouraud lighting calculations apply
    canvas->drawing_mode = PIXEL_RENDER;
    canvas->cull_backface = true;

    Pixel_Clear(canvas, 0xff181818); // Dark gray background

    PixelMesh cube_mesh = CreateCubeMesh();

    // Define a shiny plastic-like material to test ambient, diffuse, and specular highlights clearly
    // Material shiny_material = {
    //     {20, 20, 50, 255},    // Ambient (slight bluish tint in shadow)
    //     {0, 121, 241, 255},   // Diffuse (Base Blue color)
    //     {255, 255, 255, 255}, // Specular (White highlight flare)
    //     64.0f                 // Shininess exponent
    // };

    Material shiny_material = {
        {10.0, 10.0, 10.0, 255.0},    // Ambient (slight bluish tint in shadow)
        RED,   // Diffuse (Base Blue color)
        WHITE, // Specular (White highlight flare)
        256.0f                 // Shininess exponent
    };

    PixelInstance instance;
    instance.mesh = &cube_mesh;
    instance.transform.position = (Vec3){0.0f, 0.0f, -2.5f}; // Centered in front of camera
    instance.transform.rotation = (Vec3){0.6f, 0.9f, 0.0f}; // Tilted so multiple faces catch light
    instance.transform.scale    = 1.0f;
    // instance.transform.rotation = (Vec3){0.5f, 0.6f, 0.0f};
    instance.material = shiny_material;


    // Creates a sphere of radius 0.5f with 16 latitude and 16 longitude subdivisions
    PixelMesh sphere_mesh = CreateSphereMesh(0.5f, 64, 64);

    PixelInstance sphere_instance;
    sphere_instance.mesh = &sphere_mesh;
    sphere_instance.transform.position = (Vec3){0.0f, 0.0f, -1.5f};
    sphere_instance.transform.rotation = (Vec3){0.0f, 0.8f, 0.0f};
    sphere_instance.transform.scale    = 1.0f;
    sphere_instance.material = shiny_material;

    // Define a single Point Light positioned slightly to the top-right-front of the cube
    Light scene_lights[1] = {
        {
            POINT_LIGHT,                      // Type 1 = Point Light
            {-2.5f, 3.0f, -1.5f},    // World position
            {0.0f, 0.0f, 0.0f},     // Direction (unused for point lights)
            WHITE,                  // Color
            1.0f                    // Intensity multiplier
        }
    };

    PixelScene scene;
    scene.instances = &sphere_instance;
    scene.instance_count = 1;
    scene.lights = scene_lights;
    scene.light_count = 1;

    Mat4 view_matrix = Pixel_Mat4ViewMatrixTRS({0.0f, 0.0f, -0.3f}, {0.0f, 0.0f, 0.0f});

    float fov = 60.0f;
    float fov_rad = fov * (PIXEL_PI / 180.0f);
    float aspect = (float)WIDTH / (float)HEIGHT;
    Mat4 proj_matrix = Pixel_Mat4PerspectiveProjectionMatrix(fov_rad, aspect, 0.1f, 100.0f);

    // Render scene using the pipeline
    Pixel_RenderScene(canvas, &scene, view_matrix, proj_matrix);

    Pixel_SwapBuffers(canvas);
    Pixel_CreateDirectories("./images");
    Pixel_SaveBMP(canvas, "./images/Lighting_Test.bmp");

    std::cout << "Lighting test scene saved to ./images/Lighting_Test.bmp[cite: 1]\n";

    Pixel_Destroy(canvas);
    return 0;
}
