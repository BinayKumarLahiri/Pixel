#include "../../src/pixel.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <chrono>
#include <gl/gl.h>

#define WIDTH  600
#define HEIGHT 500

int main() {
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Get primary monitor to open in full screen mode
    GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary_monitor);

    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Pixel Library - Live Render Studio", primary_monitor, NULL);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable VSync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Initialize Pixel Software Renderer Context
    uint32_t* pixel_buffer = new uint32_t[WIDTH * HEIGHT];
    PixelContext *canvas = Pixel_Init(pixel_buffer, WIDTH, HEIGHT);

    float rect_h = 200.0f;

    PixelMesh sphere_mesh = CreateSphereMesh(0.5f, 64, 64);
    Material shiny_material = {
        RED,    // Ambient (slight bluish tint in shadow)
        RED,   // Diffuse (Base Blue color)
        WHITE, // Specular (White highlight flare)
        256.0f                 // Shininess exponent
    };

    PixelInstance sphere_instance;
    sphere_instance.mesh = &sphere_mesh;
    sphere_instance.transform.position = (Vec3){0.0f, 0.0f, -2.5f};
    sphere_instance.transform.rotation = (Vec3){0.0f, 0.8f, 0.0f};
    sphere_instance.transform.scale    = 1.0f;
    sphere_instance.material = shiny_material;

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

    Vec3 camera_position = Vec3 {0.0f, 0.0f, -0.3f};

    float fov = 60.0f;
    float fov_rad = fov * (PIXEL_PI / 180.0f);
    float aspect = (float)WIDTH / (float)HEIGHT;
    Mat4 proj_matrix = Pixel_Mat4PerspectiveProjectionMatrix(fov_rad, aspect, 0.1f, 100.0f);

    // Create an OpenGL Texture to mirror the Pixel software render buffer
    GLuint render_texture;
    glGenTextures(1, &render_texture);
    glBindTexture(GL_TEXTURE_2D, render_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int latitude = 8;
    int longitude = 8;
    float radius = 0.5f;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- 1. Software Rendering Step ---

        Mat4 view_matrix = Pixel_Mat4ViewMatrixTRS(camera_position, {0.0f, 0.0f, 0.0f});

        auto start_time = std::chrono::high_resolution_clock::now();

        Pixel_Clear(canvas, 0xff000000);
        // Pixel_DrawFilledRectangle(canvas, (int)rect_x, (int)rect_y, (uint32_t)rect_w, (uint32_t)rect_h, RED);
        Pixel_RenderScene(canvas, &scene, view_matrix, proj_matrix);
        Pixel_SwapBuffers(canvas);

        for (int i = 0; i < WIDTH * HEIGHT; ++i) {
            uint32_t argb = canvas->front_buffer[i];

            // Extract individual color channels using bit shifting and masking
            uint32_t a = (argb >> 24) & 0xFF;
            uint32_t r = (argb >> 16) & 0xFF;
            uint32_t g = (argb >>  8) & 0xFF;
            uint32_t b =  argb        & 0xFF;

            // Pack channels back into a 32-bit integer in true RGBA order
            // On little-endian systems, this places R at byte 0, G at byte 1, B at byte 2, and A at byte 3
            canvas->front_buffer[i] = (r << 24) | (g << 16) | (b << 8) | a;
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float, std::milli> software_duration = end_time - start_time;
        float software_fps = 1000.0f / software_duration.count();

        // Upload front buffer to OpenGL texture
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glBindTexture(GL_TEXTURE_2D, render_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, canvas->front_buffer);

        // Make UI occupy the entire screen workspace
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("MainDock", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);

        // Create a layout split: Left Viewport (Canvas), Right Control Panel
        float spacing = 10.0f;
        float total_width = ImGui::GetContentRegionAvail().x;
        float left_panel_width = total_width * 0.65f; // 65% for viewport, 35% for controls
        float right_panel_width = total_width - left_panel_width - spacing;

        // =====================================================================
        // LEFT-HAND SIDE: VIEWPORT PANEL (With FPS and Canvas Size in Header)
        // =====================================================================
        char header_title[128];
        float current_fps = ImGui::GetIO().Framerate;
        snprintf(header_title, sizeof(header_title), "Render Viewport | Canvas: %dx%d | Software FPS: %.1f | App FPS: %.1f", WIDTH, HEIGHT, software_fps, current_fps);

        ImGui::BeginChild("LeftViewportPanel", ImVec2(left_panel_width, ImGui::GetContentRegionAvail().y), true);
        ImGui::Text("%s", header_title);
        ImGui::Separator();

        // Center the software canvas image inside the left panel
        ImVec2 avail_size = ImGui::GetContentRegionAvail();
        float scale = Pixel_Min(avail_size.x / WIDTH, avail_size.y / HEIGHT);
        ImVec2 scaled_img_size(WIDTH * scale, HEIGHT * scale);

        ImGui::SetCursorPosX((avail_size.x - scaled_img_size.x) * 0.5f + ImGui::GetCursorPosX());
        ImGui::SetCursorPosY((avail_size.y - scaled_img_size.y) * 0.5f + ImGui::GetCursorPosY());

        ImGui::Image((ImTextureID)(intptr_t)render_texture, scaled_img_size);
        ImGui::EndChild();

        ImGui::SameLine(0, spacing);

        // =====================================================================
        // RIGHT-HAND SIDE: CONTROL PANEL
        // =====================================================================
        ImGui::BeginChild("RightControlPanel", ImVec2(right_panel_width, ImGui::GetContentRegionAvail().y), true);
        ImGui::Text("Control Panel");
        ImGui::Separator();

        ImGui::Spacing();
        ImGui::Text("Rectangle Properties:");
        ImGui::SliderFloat("CameraX", &camera_position.x, -2.0f, 20.0);
        ImGui::SliderFloat("Cameray", &camera_position.y, -2.0f, 20.0);
        ImGui::SliderFloat("Cameraz", &camera_position.z, -2.0f, 20.0);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Vertex Mode", ImVec2(-1, 30))) {
            canvas->drawing_mode = PIXEL_VERTEX;
        }

        if (ImGui::Button("Wireframe Mode", ImVec2(-1, 30))) {
            canvas->drawing_mode = PIXEL_WIREFRAME;
        }

        if (ImGui::Button("Render Mode", ImVec2(-1, 30))) {
            canvas->drawing_mode = PIXEL_RENDER;
        }

        if (ImGui::Button("Solid Mode", ImVec2(-1, 30))) {
            canvas->drawing_mode = PIXEL_SOLID;
        }

        if (ImGui::Button("Quit Application", ImVec2(-1, 30))) {
            glfwSetWindowShouldClose(window, true);
        }

        ImGui::EndChild();

        ImGui::End(); // End MainDock

        // --- Rendering UI ---
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    delete[] pixel_buffer;
    Pixel_Destroy(canvas);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
