#ifndef PIXEL_H
#define PIXEL_H

#include <cstdint>
#include <time.h>
#ifdef __cplusplus
extern "C" {
#endif


#define MAX_READ_BUFFER_SIZE 4096

typedef struct {
    struct timespec start_time;
} PixelTimer;

PixelTimer Pixel_TimerStart(void);
float Pixel_TimerElapsedMs(PixelTimer* timer);

// -----------------------------------------------------------------
// Some Helper Methods useful for testing the code or building it
// -----------------------------------------------------------------
void Pixel_SwapInt(uint32_t *value1, uint32_t *value2);
void Pixel_CreateDirectories(const char *path);

// -----------------------------------------------------------------
// Vector and Maths Related stuff
// -----------------------------------------------------------------

#define PIXEL_PI 3.14159265359
#define PIXEL_PIBY2 1.570796326795


// Simple Linear Interpolation function for the Pixel Library
float Pixel_Max(float val1, float val2);
float Pixel_Min(float val1, float val2);
float Pixel_Lerp(float start, float end, float alpha);

typedef struct {
    float x;
    float y;
} Vec2;

typedef struct {
  float x;
  float y;
  float z;
} Vec3;

typedef struct {
  float x;
  float y;
  float z;
  float w;
} Vec4;

typedef struct {
  // 16 floats, stored in column-major order:
  // m[0]  m[4]  m[8]  m[12]
  // m[1]  m[5]  m[9]  m[13]
  // m[2]  m[6]  m[10] m[14]
  // m[3]  m[7]  m[11] m[15]
  float m[16];
} Mat4;

Vec3 Pixel_Vec3Add(const Vec3 vector1, const Vec3 vector2);
Vec3 Pixel_Vec3Sub(const Vec3 vector1, const Vec3 vector2);
Vec3 Pixel_Vec3Norm(const Vec3 vector);
Vec3 Pixel_Vec3Cross(const Vec3 vector1, const Vec3 vector2);
Vec3 Pixel_Vec3Mul(const Vec3 vector1, float scale);
float Pixel_Vec3EdgeCross(const Vec3 vector1, const Vec3 vector2,
                          const Vec3 vector3);
float Pixel_Vec3Dot(const Vec3 vector1, const Vec3 vector2);
float Pixel_Vec3Len(const Vec3 vector);
void Pixel_Vec3Swap(Vec3 *vector1, Vec3 *vector2);

Vec4 Pixel_Vec3ToVec4(Vec3 vector);
void Pixel_Vec3ToVec4Ref(Vec3 source, Vec4 *dest);
Vec3 Pixel_Vec4ToVec3(Vec4 vector);

Mat4 Pixel_Mat4Identity();
Mat4 Pixel_Mat4Mul(Mat4 mat1, Mat4 mat2);
Vec4 Pixel_Mat4MulVec4(Mat4 mat, Vec4 vec);
Vec3 Pixel_Mat4MulVec3(Mat4 mat, Vec3 vec);
Mat4 Pixel_Mat4Transpose(Mat4 mat);
Mat4 Pixel_Mat4Inverse(Mat4 mat);

// Transformation Matrices
Mat4 Pixel_Mat4ScaleMatrix(Vec3 scale_factor);
Mat4 Pixel_Mat4TranslationMatrix(Vec3 translation_values);
Mat4 Pixel_Mat4RotationMatrixX(float angle);
Mat4 Pixel_Mat4RotationMatrixY(float angle);
Mat4 Pixel_Mat4RotationMatrixZ(float angle);
Mat4 Pixel_Mat4RotationMatrixXYZ(Vec3 angles_xyz);
Mat4 Pixel_Mat4ViewMatrixTRS(Vec3 position, Vec3 angles_XYZ);
Mat4 Pixel_Mat4PerspectiveProjectionMatrix(float fov_radians,
                                           float aspect_ratio, float near_plane,
                                           float far_plane);
Mat4 Pixel_ComputeNormalMatrix(Mat4 mv_matrix);

// -----------------------------------------------------------------
// Defining Color related structs and methods
// -----------------------------------------------------------------
typedef struct {
  float r;
  float g;
  float b;
  float a;
} Color;

#define LIGHTGRAY (Color){200, 200, 200, 255} // Light Gray
#define GRAY (Color){130, 130, 130, 255}      // Gray
#define DARKGRAY (Color){80, 80, 80, 255}     // Dark Gray
#define YELLOW (Color){253, 249, 0, 255}      // Yellow
#define GOLD (Color){255, 203, 0, 255}        // Gold
#define ORANGE (Color){255, 161, 0, 255}      // Orange
#define PINK (Color){255, 109, 194, 255}      // Pink
#define RED (Color){230, 41, 55, 255}         // Red
#define MAROON (Color){190, 33, 55, 255}      // Maroon
#define GREEN (Color){0, 228, 48, 255}        // Green
#define LIME (Color){0, 158, 47, 255}         // Lime
#define DARKGREEN (Color){0, 117, 44, 255}    // Dark Green
#define SKYBLUE (Color){102, 191, 255, 255}   // Sky Blue
#define BLUE (Color){0, 121, 241, 255}        // Blue
#define DARKBLUE (Color){0, 82, 172, 255}     // Dark Blue
#define PURPLE (Color){200, 122, 255, 255}    // Purple
#define VIOLET (Color){135, 60, 190, 255}     // Violet
#define DARKPURPLE (Color){112, 31, 126, 255} // Dark Purple
#define BEIGE (Color){211, 176, 131, 255}     // Beige
#define BROWN (Color){127, 106, 79, 255}      // Brown
#define DARKBROWN (Color){76, 63, 47, 255}    // Dark Brown
#define WHITE (Color){255, 255, 255, 255}     // White
#define BLACK (Color){0, 0, 0, 255}           // Black

uint32_t Pixel_Color2Uint32(const Color color);
Color Pixel_Uint322Color(const uint32_t color);
Color Pixel_ColorScale(const Color color, float brightness);
Color Pixel_ColorBlend(const Color color1, const Color color2, float alpha);
Color Pixel_AlphaBlend(const Color source, const Color destination);
uint32_t Pixel_AlphaBlendUint32(const uint32_t source, const uint32_t destination);



// -----------------------------------------------------------------
// Rendering and PixelContext based structs
// -----------------------------------------------------------------
typedef enum {
  PIXEL_VERTEX = 0, // Drawing only the points and vertices
  PIXEL_WIREFRAME,  // Drawing the points and lines connecting them
  PIXEL_SOLID, // Drawing the solid colors ignoring the shading and lighting
  PIXEL_RENDER // Total rendering mode
} DrawMode;

typedef enum {
    PIXEL_NEAREST = 0,
    PIXEL_BILINEAR
} FilterMode;

// 1. Texture Struct Definition
typedef struct {
    uint32_t *pixels; // RGBA pixel data buffer
    int width;
    int height;
} PixelTexture;

Color Pixel_SampleTexture(PixelTexture *texture, float x, float y, FilterMode filter_mode);

typedef struct {
    // Benchmarking values
    int vertex_count;
    int triangle_count;
    int drawn_traingle_count;
    float scene_rendering;
    float mesh_rendering;
    float light_transformation;
    float vertex_out_with_lighting;
    float triangle_rendering;

    // Triangle Rasterixation Specific Benchmarks
    float bounding_box;
    float edge_cross_setup;
    float attribute_interpolation;

    int pixel_tested;
    int pixel_inside;
    int pixel_depth_failed;
    int pixel_writtern;
} Benchmark;

// Generic vertex output passed to the rasterizer after vertex shading
typedef struct {
    Vec3 screen_pos;    // Screen space position (x, y, z)
    Vec3 view_pos;      // View space position
    Vec3 view_normal;   // View space normal
    Vec4 clip_pos;      // Clip space position (x, y, z, w) for frustum culling
    float clip_w;
    float inv_w;
    Color light_factor;  // Pre-calculated lighting accumulation (RGB)
    Vec2 uv;            // Texture coordinates
    Color base_color;   // Base vertex/material color
} VertexOutput;

typedef struct {
  uint32_t width;
  uint32_t height;
  // Buffers and depth buffers with swap buffers
  uint32_t *front_buffer;
  uint32_t *back_buffer;
  float *depth_buffer;
  // Different rendering mode used in the drawing functions and rendering loop
  // for controlling how the primitives and triangles are drawn
  DrawMode drawing_mode;
  FilterMode filter_mode;
  bool cull_backface;
  bool enable_lighting;
  bool enable_frustum_culling; // Triangle frustum culling toggle
  Benchmark benchmark;

  VertexOutput *out_vertices;
  int out_vertices_capacity;
} PixelContext;

// -----------------------------------------------------------------
// Initialization and Destroying methods
// -----------------------------------------------------------------
PixelContext *Pixel_Init(uint32_t *external_buffer, uint32_t width,
                         uint32_t height);
void Pixel_Destroy(PixelContext *ctx);
void Pixel_SwapBuffers(PixelContext *ctx);
void Pixel_SetDrawMode(PixelContext *ctx, DrawMode draw_mode);
void Pixel_Clear(PixelContext *ctx, uint32_t color);
void Pixel_PutPixel(PixelContext *ctx, uint32_t x, uint32_t y, uint32_t color);
void Pixel_SaveBMP(PixelContext *ctx, const char *filename);
void Pixel_PrintBenchmark(PixelContext* ctx);
void Pixel_SetFrustumCulling(PixelContext *ctx, bool enable);


// -------------------------------------------------------------------------------
// Simple 8x8 Bitmap font rendering for primitive text Drawing (Only ASCII Values)
// -------------------------------------------------------------------------------
extern unsigned char PIXEL_DEFAULT_FONT[128][8];

void Pixel_PutChar(PixelContext* ctx, const char c, int cx, int cy, const unsigned char font[128][8], Color color, int scale);
void Pixel_PutText(PixelContext* ctx, int tx, int ty, const unsigned char font[128][8], Color color, int scale, const char* format, ...);



// -----------------------------------------------------------------
// Simple primitives
// -----------------------------------------------------------------

// 1. Lines
void Pixel_DrawLine(PixelContext *ctx, uint32_t x0, uint32_t y0, uint32_t x1,
                    uint32_t y1, Color color);
void Pixel_DrawLineV(PixelContext *ctx, Vec3 point1, Vec3 point2, Color color);

// 2. Arcs
void Pixel_PlotArcPixel(PixelContext *ctx, int xc, int yc, int x, int y,
                        float start_rad, float end_rad, uint32_t color);
void Pixel_DrawArc(PixelContext *ctx, int xc, int yc, int r, float start_angle,
                   float end_angle, Color color);
void Pixel_DrawArcV(PixelContext *ctx, Vec3 center, float r, float start_angle,
                    float end_angle, Color color);
void Pixel_DrawArcDegree(PixelContext *ctx, int xc, int yc, int r,
                         float start_angle, float end_angle, Color color);

// 3.Circles
void Pixel_DrawCircle(PixelContext *ctx, float xc, float yc, float r,
                      Color color);
void Pixel_DrawCircleV(PixelContext *ctx, Vec3 center, float r, Color color);
void Pixel_DrawScanLine(PixelContext *ctx, int start_x, int end_x, int y,
                        Color color);
void Pixel_DrawFilledCircle(PixelContext *ctx, int xc, int yc, int r,
                            Color color);
void Pixel_DrawFilledCircleV(PixelContext *ctx, Vec3 center, int r,
                             Color color);

// Rectangles
void Pixel_DrawRectangle(PixelContext *ctx, int x0, int y0, uint32_t width,
                         uint32_t height, Color color);
void Pixel_DrawFilledRectangle(PixelContext *ctx, int x0, int y0,
                               uint32_t width, uint32_t height, Color color);
void Pixel_DrawRectangleTexture(PixelContext *ctx, int x0, int y0, uint32_t width, uint32_t height, PixelTexture *texture);

// Triangles
void Pixel_DrawTriangleLines(PixelContext *ctx, int x0, int y0, int x1, int y1,
                             int x2, int y2, Color color);
void Pixel_DrawTriangleShaded(PixelContext *ctx, int x0, int y0, int x1, int y1,
                              int x2, int y2, Color color0, Color color1,
                              Color color2, float h0, float h1, float h2);
void Pixel_DrawTriangleTexture(PixelContext *ctx, int x0, int y0, int x1, int y1,
                              int x2, int y2, PixelTexture *texture, float u0, float v0, float u1, float v1, float u2, float v2, FilterMode filter_mode);
void Pixel_DrawTriangleFilled(PixelContext *ctx, int x0, int y0, int x1, int y1,
                              int x2, int y2, Color color);
void Pixel_DrawTriangleFilledV(PixelContext *ctx, Vec3 point0, Vec3 point2,
                               Vec3 point3, Color color);
void Pixel_DrawTriangleDepthColor(PixelContext *ctx, int x0, int y0, int x1, int y1,
                              int x2, int y2, Color color0, Color color1,
                              Color color2, float z0, float z1, float z2);


// -----------------------------------------------------------------
// 3D Rendering Related stuff (Vertex, Mesh, )
// -----------------------------------------------------------------

typedef struct {
  Vec3 coordinate;
  Vec3 normal;
  Color color;
  Vec2 uv;
} PixelVertex;

typedef struct {
  int vertices[3];
  uint32_t color;
} PixelTriangle;




typedef struct {
  PixelVertex *vertices;
  PixelTriangle *triangles;
  int vertex_count;
  int triangle_count;
  PixelTexture *texture; // Optional: can be NULL if untextured
  bool has_texture;
} PixelMesh;

typedef struct {
  Vec3 position;
  Vec3 rotation;
  float scale;
} PixelTransform;

typedef enum {
    AMBIENT_LIGHT = 0,
    POINT_LIGHT,
    DIRECTIONAL_LIGHT
}PixelLightType;

typedef struct {
    PixelLightType type;       // 0 = Ambient, 1 = Point, 2 = Directional
    Vec3 position;  // View space position (for point lights)
    Vec3 direction; // View space normalized direction (for directional lights)
    Color color;    // Light color/intensity tint
    float intensity;// Base multiplier
} Light;

typedef struct {
    Color ambient;
    Color diffuse;
    Color specular;
    float shininess; // Specular exponent (alpha)
} Material;



typedef struct {
    int x_min, y_min;
    int x_max, y_max;
} PixelBoundingBox;

typedef struct {
    float w0_col, w1_col, w2_col;
    float w0_row, w1_row, w2_row;
} PixelBaryWeightDiff;

typedef struct {
    float depth;
    Color light_color;
    Color base;
    Vec2 uv;
} PixelInterpolatedAttributes;



// 2. Updated PixelInstance Struct (Containing Mesh, Transform, Material, and Texture)
typedef struct {
    PixelMesh *mesh;
    PixelTransform transform;
    Material material;
} PixelInstance;

// 3. Scene Struct Definition (Containing Instances and Lights)
typedef struct {
    PixelInstance *instances;
    int instance_count;
    Light *lights;
    int light_count;
} PixelScene;


void Pixel_RenderScene(PixelContext *ctx, const PixelScene *scene, Mat4 view, Mat4 projection);


// -----------------------------------------------------------------
// Simple Primitive 3D Shapes
// -----------------------------------------------------------------

PixelMesh CreateCubeMesh();
PixelMesh CreateSphereMesh(float radius, int lats, int longs);
PixelMesh Pixel_LoadObjMesh(const char *file_name);




#ifdef __cplusplus
}
#endif

#endif // PIXEL_H
