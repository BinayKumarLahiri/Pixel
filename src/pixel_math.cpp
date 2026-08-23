#include "pixel.h"
#include <cmath>

// Helper to access column-major layout: col * 4 + row
#define MAT4_IDX(col, row) ((col) * 4 + (row))

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

Vec3 Pixel_Mat4MulVec3(Mat4 mat, Vec3 vec) {
    Vec4 v = Pixel_Mat4MulVec4(mat, {vec.x, vec.y, vec.z, 0});
    return {v.x, v.y, v.z};
}


// Helper to compute the determinant of a 3x3 submatrix from a 16-element column-major array
static float Pixel_Mat3DetFromMat4(const float *m, int skip_col, int skip_row) {
    float sub[3][3];
    int r_idx = 0;
    for (int r = 0; r < 4; r++) {
        if (r == skip_row) continue;
        int c_idx = 0;
        for (int c = 0; c < 4; c++) {
            if (c == skip_col) continue;
            sub[r_idx][c_idx] = m[MAT4_IDX(c, r)];
            c_idx++;
        }
        r_idx++;
    }
    return sub[0][0] * (sub[1][1] * sub[2][2] - sub[1][2] * sub[2][1]) -
           sub[0][1] * (sub[1][0] * sub[2][2] - sub[1][2] * sub[2][0]) +
           sub[0][2] * (sub[1][0] * sub[2][1] - sub[1][1] * sub[2][0]);
}

// Compute the Transpose of a column-major 4x4 matrix
Mat4 Pixel_Mat4Transpose(Mat4 mat) {
    Mat4 result;
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            result.m[MAT4_IDX(c, r)] = mat.m[MAT4_IDX(r, c)];
        }
    }
    return result;
}

// Compute the Inverse of a column-major 4x4 matrix using adjugate/cofactor method
Mat4 Pixel_Mat4Inverse(Mat4 mat) {
    Mat4 inv;
    float det = 0.0f;

    // Compute cofactor expansion / adjugate matrix
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            int sign = ((r + c) % 2 == 0) ? 1 : -1;
            float sub_det = Pixel_Mat3DetFromMat4(mat.m, c, r);

            // In a column-major inverse, the transpose of the cofactor matrix is built into placement:
            // inv.m[MAT4_IDX(r, c)] corresponds to row r, col c in mathematical terms
            inv.m[MAT4_IDX(r, c)] = (float)sign * sub_det;
        }
    }

    // Calculate full 4x4 determinant using the first column expansion
    for (int r = 0; r < 4; r++) {
        det += mat.m[MAT4_IDX(0, r)] * inv.m[MAT4_IDX(r, 0)]; // Using cofactors of column 0
    }

    // Handle degenerate/non-invertible matrices safely
    if (fabsf(det) < 0.00001f) {
        Mat4 identity = {0};
        for (int i = 0; i < 4; i++) {
            identity.m[MAT4_IDX(i, i)] = 1.0f;
        }
        return identity;
    }

    float inv_det = 1.0f / det;

    // Multiply by 1 / determinant
    for (int i = 0; i < 16; i++) {
        inv.m[i] *= inv_det;
    }

    return inv;
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
  Mat4 result = Pixel_Mat4Mul(z_rotate, Pixel_Mat4Mul(y_rotate, x_rotate));
  return result;
}

static Mat4 Pixel_Mat4RotationMatrixZYX(Vec3 angles_XYZ) {
  Mat4 x_rotate = Pixel_Mat4RotationMatrixX(angles_XYZ.x);
  Mat4 y_rotate = Pixel_Mat4RotationMatrixY(angles_XYZ.y);
  Mat4 z_rotate = Pixel_Mat4RotationMatrixZ(angles_XYZ.z);
  Mat4 result = Pixel_Mat4Mul(z_rotate, Pixel_Mat4Mul(y_rotate, x_rotate));
  return result;
}

Mat4 Pixel_Mat4ViewMatrixTRS(Vec3 position, Vec3 angles_XYZ) {
  Vec3 inverted_rotation = {-angles_XYZ.x, -angles_XYZ.y, -angles_XYZ.z};
  Vec3 inverted_translation = {-position.x, -position.y, -position.z};
  Mat4 rotation = Pixel_Mat4RotationMatrixZYX(inverted_rotation);
  Mat4 translation = Pixel_Mat4TranslationMatrix(inverted_translation);
  Mat4 view_matrix = Pixel_Mat4Mul(rotation, translation);
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

Mat4 Pixel_ComputeNormalMatrix(Mat4 mv_matrix) {
    Mat4 inv_mv = Pixel_Mat4Inverse(mv_matrix);
    return Pixel_Mat4Transpose(inv_mv);
}
