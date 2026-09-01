#include "pixel.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

// Temporary structures used only during parsing
typedef struct {
    float x, y, z;
} TempVec3;

typedef struct {
    float x, y;
} TempVec2;

typedef struct {
    int v_idx;
    int vt_idx;
    int vn_idx;
} ObjIndex;

PixelMesh Pixel_LoadObjMesh(const char *file_name) {
    FILE *file = fopen(file_name, "r");
    if (!file) {
        printf("[Error] Failed to open OBJ file: %s\n", file_name);
        return (PixelMesh){nullptr, nullptr, 0, 0, nullptr, false};
    }

    std::vector<TempVec3> temp_positions;
    std::vector<TempVec2> temp_texcoords;
    std::vector<TempVec3> temp_normals;

    // We'll store parsed faces as lists of indices
    // To handle arbitrary polygons (quads, etc.), we can triangulate them on the fly
    std::vector<PixelVertex> loaded_vertices;
    std::vector<PixelTriangle> loaded_triangles;

    char line_header[128];
    while (fscanf(file, "%s", line_header) != EOF) {
        if (strcmp(line_header, "v") == 0) {
            TempVec3 v;
            fscanf(file, "%f %f %f\n", &v.x, &v.y, &v.z);
            temp_positions.push_back(v);
        } else if (strcmp(line_header, "vt") == 0) {
            TempVec2 vt;
            fscanf(file, "%f %f\n", &vt.x, &vt.y);
            // Invert V coordinate if your renderer/texture space expects top-left origin (standard for many software renderers)
            vt.y = 1.0f - vt.y;
            temp_texcoords.push_back(vt);
        } else if (strcmp(line_header, "vn") == 0) {
            TempVec3 vn;
            fscanf(file, "%f %f %f\n", &vn.x, &vn.y, &vn.z);
            temp_normals.push_back(vn);
        } else if (strcmp(line_header, "f") == 0) {
            char line_buffer[1024];
            if (!fgets(line_buffer, sizeof(line_buffer), file)) break;

            ObjIndex face_indices[4];
            int vertex_count = 0;

            // Parse face elements (can be v, v//vn, v/vt/vn, or v/vt)
            char *token = strtok(line_buffer, " \t\r\n");
            while (token != NULL && vertex_count < 4) {
                ObjIndex idx = {0, 0, 0};

                if (strstr(token, "//")) {
                    // Format: v//vn
                    sscanf(token, "%d//%d", &idx.v_idx, &idx.vn_idx);
                } else if (sscanf(token, "%d/%d/%d", &idx.v_idx, &idx.vt_idx, &idx.vn_idx) == 3) {
                    // Format: v/vt/vn
                } else if (sscanf(token, "%d/%d", &idx.v_idx, &idx.vt_idx) == 2) {
                    // Format: v/vt
                } else {
                    // Format: v
                    sscanf(token, "%d", &idx.v_idx);
                }

                // Handle relative indexing (negative indices in OBJ)
                if (idx.v_idx < 0) idx.v_idx = (int)temp_positions.size() + idx.v_idx + 1;
                if (idx.vt_idx < 0) idx.vt_idx = (int)temp_texcoords.size() + idx.vt_idx + 1;
                if (idx.vn_idx < 0) idx.vn_idx = (int)temp_normals.size() + idx.vn_idx + 1;

                face_indices[vertex_count++] = idx;
                token = strtok(NULL, " \t\r\n");
            }

            // Helper lambda/block to push a vertex into our final mesh buffers,
            // avoiding exact duplicates or simply creating a direct stream.
            // Here we map unique attribute combinations into PixelVertex array.
            auto get_or_create_vertex = [&](ObjIndex idx) -> int {
                PixelVertex new_vert;

                // Position
                if (idx.v_idx > 0 && idx.v_idx <= (int)temp_positions.size()) {
                    TempVec3 p = temp_positions[idx.v_idx - 1];
                    new_vert.coordinate = {p.x, p.y, p.z};
                } else {
                    new_vert.coordinate = {0, 0, 0};
                }

                // Normal
                if (idx.vn_idx > 0 && idx.vn_idx <= (int)temp_normals.size()) {
                    TempVec3 n = temp_normals[idx.vn_idx - 1];
                    new_vert.normal = {n.x, n.y, n.z};
                } else {
                    new_vert.normal = {0, 1, 0}; // Default upward normal
                }

                // Texture Coordinates
                if (idx.vt_idx > 0 && idx.vt_idx <= (int)temp_texcoords.size()) {
                    TempVec2 t = temp_texcoords[idx.vt_idx - 1];
                    new_vert.uv = {t.x, t.y};
                } else {
                    new_vert.uv = {0, 0};
                }

                // Default vertex base color (White, tinted by material later)
                new_vert.color = WHITE;

                loaded_vertices.push_back(new_vert);
                return (int)loaded_vertices.size() - 1;
            };

            // Triangulate the face (supports triangles and simple quads)
            if (vertex_count >= 3) {
                int i0 = get_or_create_vertex(face_indices[0]);
                int i1 = get_or_create_vertex(face_indices[1]);
                int i2 = get_or_create_vertex(face_indices[2]);
                loaded_triangles.push_back((PixelTriangle){{i0, i1, i2}, 0});

                if (vertex_count == 4) {
                    // Quad split into second triangle (0 -> 2 -> 3)
                    int i3 = get_or_create_vertex(face_indices[3]);
                    loaded_triangles.push_back((PixelTriangle){{i0, i2, i3}, 0});
                }
            }
        }
    }

    fclose(file);

    // Allocate final C-style arrays required by PixelMesh structure
    PixelMesh mesh;
    mesh.vertex_count = (int)loaded_vertices.size();
    mesh.triangle_count = (int)loaded_triangles.size();

    mesh.vertices = (PixelVertex *)malloc(mesh.vertex_count * sizeof(PixelVertex));
    mesh.triangles = (PixelTriangle *)malloc(mesh.triangle_count * sizeof(PixelTriangle));

    memcpy(mesh.vertices, loaded_vertices.data(), mesh.vertex_count * sizeof(PixelVertex));
    memcpy(mesh.triangles, loaded_triangles.data(), mesh.triangle_count * sizeof(PixelTriangle));

    mesh.texture = nullptr;
    mesh.has_texture = false;

    printf("[Success] Loaded OBJ mesh: %s (%d vertices, %d triangles)\n", file_name, mesh.vertex_count, mesh.triangle_count);
    return mesh;
}
