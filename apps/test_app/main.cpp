#include "../../src/pixel.h"
#include <cstdint>
#include <iostream>

#define WIDTH  500
#define HEIGHT 500

PixelMesh CreateCubeMesh() {
    // 24 Vertices (4 independent vertices for each of the 6 faces)
    // This allows each face to have its own solid, flat color without bleeding into neighbors.
    static PixelVertex vertices[24] = {
        // --- Front Face (RED) ---
        {{-0.5f, -0.5f,  0.5f}, {0,0,1}, RED, 0, 0}, // 0
        {{ 0.5f, -0.5f,  0.5f}, {0,0,1}, RED, 1, 0}, // 1
        {{ 0.5f,  0.5f,  0.5f}, {0,0,1}, RED, 1, 1}, // 2
        {{-0.5f,  0.5f,  0.5f}, {0,0,1}, RED, 0, 1}, // 3

        // --- Back Face (GREEN) ---
        {{ 0.5f, -0.5f, -0.5f}, {0,0,-1}, GREEN, 0, 0}, // 4
        {{-0.5f, -0.5f, -0.5f}, {0,0,-1}, GREEN, 1, 0}, // 5
        {{-0.5f,  0.5f, -0.5f}, {0,0,-1}, GREEN, 1, 1}, // 6
        {{ 0.5f,  0.5f, -0.5f}, {0,0,-1}, GREEN, 0, 1}, // 7

        // --- Left Face (BLUE) ---
        {{-0.5f, -0.5f, -0.5f}, {-1,0,0}, BLUE, 0, 0}, // 8
        {{-0.5f, -0.5f,  0.5f}, {-1,0,0}, BLUE, 1, 0}, // 9
        {{-0.5f,  0.5f,  0.5f}, {-1,0,0}, BLUE, 1, 1}, // 10
        {{-0.5f,  0.5f, -0.5f}, {-1,0,0}, BLUE, 0, 1}, // 11

        // --- Right Face (YELLOW) ---
        {{ 0.5f, -0.5f,  0.5f}, {1,0,0}, YELLOW, 0, 0}, // 12
        {{ 0.5f, -0.5f, -0.5f}, {1,0,0}, YELLOW, 1, 0}, // 13
        {{ 0.5f,  0.5f, -0.5f}, {1,0,0}, YELLOW, 1, 1}, // 14
        {{ 0.5f,  0.5f,  0.5f}, {1,0,0}, YELLOW, 0, 1}, // 15

        // --- Top Face (PURPLE) ---
        {{-0.5f,  0.5f,  0.5f}, {0,1,0}, PURPLE, 0, 0}, // 16
        {{ 0.5f,  0.5f,  0.5f}, {0,1,0}, PURPLE, 1, 0}, // 17
        {{ 0.5f,  0.5f, -0.5f}, {0,1,0}, PURPLE, 1, 1}, // 18
        {{-0.5f,  0.5f, -0.5f}, {0,1,0}, PURPLE, 0, 1}, // 19

        // --- Bottom Face (ORANGE) ---
        {{-0.5f, -0.5f, -0.5f}, {0,-1,0}, ORANGE, 0, 0}, // 20
        {{ 0.5f, -0.5f, -0.5f}, {0,-1,0}, ORANGE, 1, 0}, // 21
        {{ 0.5f, -0.5f,  0.5f}, {0,-1,0}, ORANGE, 1, 1}, // 22
        {{-0.5f, -0.5f,  0.5f}, {0,-1,0}, ORANGE, 0, 1}  // 23
    };

    // 12 Triangles pointing to our newly mapped 24 indices
    static PixelTriangle triangles[12] = {
        // Front Face
        {{0, 1, 2}, 0}, {{0, 2, 3}, 0},
        // Back Face
        {{4, 5, 6}, 0}, {{4, 6, 7}, 0},
        // Left Face
        {{8, 9, 10}, 0}, {{8, 10, 11}, 0},
        // Right Face
        {{12, 13, 14}, 0}, {{12, 14, 15}, 0},
        // Top Face
        {{16, 17, 18}, 0}, {{16, 18, 19}, 0},
        // Bottom Face
        {{20, 21, 22}, 0}, {{20, 22, 23}, 0}
    };

    PixelMesh mesh;
    mesh.vertices = vertices;
    mesh.vertex_count = 24;
    mesh.triangles = triangles;
    mesh.triangle_count = 12;
    return mesh;
}

int main() {
    uint32_t buffer[WIDTH * HEIGHT];
    PixelContext *canvas = Pixel_Init(buffer, WIDTH, HEIGHT);
    if (!canvas) {
        std::cout << "Failed to initialize Pixel Context\n";
        return 1;
    }

    canvas->drawing_mode = PIXEL_SOLID;

    // 1. Clear background and depth buffer
    Pixel_Clear(canvas, 0xff181818); // Dark gray background color

    // 2. Create the Cube Mesh
    PixelMesh cube_mesh = CreateCubeMesh();

    // 3. Define a Scene Instance array of size 1
    int instance_count = 3;
    PixelInstance scene_instances[3];
    scene_instances[0].mesh = &cube_mesh;
    scene_instances[0].transform.position = (Vec3){-0.5f, 0.0f, -2.5f}; // 2.5 units away
    scene_instances[0].transform.rotation = (Vec3){0.5f, 0.8f, 0.0f}; // Rotated slightly
    scene_instances[0].transform.scale    = 1.0f;                     // Unit scale

    scene_instances[1].mesh = &cube_mesh;
    scene_instances[1].transform.position = (Vec3){0.8f,-0.8f, -3.5f}; // 2.5 units away
    scene_instances[1].transform.rotation = (Vec3){-0.5f, 0.4f, 0.0f}; // Rotated slightly
    scene_instances[1].transform.scale    = 0.6f;                     // Unit scale

    scene_instances[2].mesh = &cube_mesh;
    scene_instances[2].transform.position = (Vec3){0.8f,0.8f, -3.5f}; // 2.5 units away
    scene_instances[2].transform.rotation = (Vec3){0.0f, 0.4f, 0.0f}; // Rotated slightly
    scene_instances[2].transform.scale    = 1.0f;                     // Unit scale

    // 4. Setup Camera View Matrix
    Mat4 view_matrix = Pixel_Mat4Identity();

    // 5. Setup Perspective Projection Matrix
    float fov = 60;
    float fov_rad = fov * (PIXEL_PI/180.0);
    float aspect = (float)WIDTH / (float)HEIGHT;
    Mat4 proj_matrix = Pixel_Mat4PerspectiveProjectionMatrix(fov_rad, aspect, 0.1f, 100.0f);

    // 6. Render the Scene matching your exact method signature (pointer + count)
    Pixel_RenderScene(canvas, scene_instances, instance_count, view_matrix, proj_matrix);

    // 7. Swap buffers and save output
    Pixel_SwapBuffers(canvas);
    Pixel_CreateDirectories("./images");
    Pixel_SaveBMP(canvas, "./images/Cube3D_colored.bmp");

    std::cout << "Successfully rendered 3D cube and saved to ./images/Cube3D_colored.bmp\n";

    Pixel_Destroy(canvas);
    return 0;
}
