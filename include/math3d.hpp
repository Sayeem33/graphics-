#pragma once
#ifndef MATH3D_HPP
#define MATH3D_HPP

#include <cmath>
#include <iostream>
#include <algorithm>

namespace m3d {

constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 6.28318530717958647692f;
constexpr float DEG2RAD = PI / 180.0f;
constexpr float RAD2DEG = 180.0f / PI;

inline float radians(float deg) { return deg * DEG2RAD; }
inline float degrees(float rad) { return rad * RAD2DEG; }
inline float clamp(float v, float minVal, float maxVal) {
    return std::max(minVal, std::min(maxVal, v));
}

// ==========================================
// Vector2
// ==========================================
struct Vec2 {
    float x{0.0f}, y{0.0f};

    Vec2() = default;
    Vec2(float _x, float _y) : x(_x), y(_y) {}
    explicit Vec2(float s) : x(s), y(s) {}

    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
    Vec2 operator/(float s) const { return {x / s, y / s}; }
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    Vec2& operator-=(const Vec2& o) { x -= o.x; y -= o.y; return *this; }
    Vec2& operator*=(float s) { x *= s; y *= s; return *this; }
    Vec2& operator/=(float s) { x /= s; y /= s; return *this; }

    float lengthSq() const { return x * x + y * y; }
    float length() const { return std::sqrt(lengthSq()); }
};

// ==========================================
// Vector3
// ==========================================
struct Vec3 {
    float x{0.0f}, y{0.0f}, z{0.0f};

    Vec3() = default;
    Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    explicit Vec3(float s) : x(s), y(s), z(s) {}

    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vec3 operator*(const Vec3& o) const { return {x * o.x, y * o.y, z * o.z}; }
    Vec3 operator/(float s) const { return {x / s, y / s, z / s}; }
    Vec3 operator-() const { return {-x, -y, -z}; }

    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    Vec3& operator-=(const Vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
    Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
    Vec3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }

    float lengthSq() const { return x * x + y * y + z * z; }
    float length() const { return std::sqrt(lengthSq()); }

    Vec3 normalized() const {
        float l = length();
        if (l > 1e-6f) return *this / l;
        return {0.0f, 0.0f, 0.0f};
    }
};

inline Vec3 operator*(float s, const Vec3& v) { return v * s; }
inline float dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}
inline Vec3 lerp(const Vec3& a, const Vec3& b, float t) {
    return a + (b - a) * t;
}
inline float distance(const Vec3& a, const Vec3& b) {
    return (b - a).length();
}

// ==========================================
// Vector4
// ==========================================
struct Vec4 {
    float x{0.0f}, y{0.0f}, z{0.0f}, w{0.0f};

    Vec4() = default;
    Vec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
    Vec4(const Vec3& v, float _w) : x(v.x), y(v.y), z(v.z), w(_w) {}
    explicit Vec4(float s) : x(s), y(s), z(s), w(s) {}

    Vec3 xyz() const { return {x, y, z}; }
};

// ==========================================
// Matrix 4x4 (Column-Major for OpenGL)
// m[col * 4 + row]
// ==========================================
struct Mat4 {
    float m[16]{
        1.0f, 0.0f, 0.0f, 0.0f,  // col 0
        0.0f, 1.0f, 0.0f, 0.0f,  // col 1
        0.0f, 0.0f, 1.0f, 0.0f,  // col 2
        0.0f, 0.0f, 0.0f, 1.0f   // col 3
    };

    Mat4() = default;

    const float* data() const { return m; }
    float* data() { return m; }

    float& operator()(int row, int col) { return m[col * 4 + row]; }
    float operator()(int row, int col) const { return m[col * 4 + row]; }

    static Mat4 identity() {
        return Mat4();
    }

    // Matrix Multiplication
    Mat4 operator*(const Mat4& b) const {
        Mat4 res;
        for (int c = 0; c < 4; ++c) {
            for (int r = 0; r < 4; ++r) {
                res(r, c) = (*this)(r, 0) * b(0, c) +
                            (*this)(r, 1) * b(1, c) +
                            (*this)(r, 2) * b(2, c) +
                            (*this)(r, 3) * b(3, c);
            }
        }
        return res;
    }

    Vec4 operator*(const Vec4& v) const {
        return {
            (*this)(0, 0) * v.x + (*this)(0, 1) * v.y + (*this)(0, 2) * v.z + (*this)(0, 3) * v.w,
            (*this)(1, 0) * v.x + (*this)(1, 1) * v.y + (*this)(1, 2) * v.z + (*this)(1, 3) * v.w,
            (*this)(2, 0) * v.x + (*this)(2, 1) * v.y + (*this)(2, 2) * v.z + (*this)(2, 3) * v.w,
            (*this)(3, 0) * v.x + (*this)(3, 1) * v.y + (*this)(3, 2) * v.z + (*this)(3, 3) * v.w
        };
    }

    Vec3 transformPoint(const Vec3& p) const {
        Vec4 res = (*this) * Vec4(p, 1.0f);
        if (std::abs(res.w) > 1e-6f) {
            return {res.x / res.w, res.y / res.w, res.z / res.w};
        }
        return res.xyz();
    }

    Vec3 transformVector(const Vec3& v) const {
        Vec4 res = (*this) * Vec4(v, 0.0f);
        return res.xyz();
    }

    // ------------------------------------------
    // 1. Translation Matrix
    // ------------------------------------------
    static Mat4 translate(const Vec3& t) {
        Mat4 res;
        res(0, 3) = t.x;
        res(1, 3) = t.y;
        res(2, 3) = t.z;
        return res;
    }
    static Mat4 translate(float tx, float ty, float tz) {
        return translate(Vec3(tx, ty, tz));
    }

    // ------------------------------------------
    // 2. Scaling Matrix
    // ------------------------------------------
    static Mat4 scale(const Vec3& s) {
        Mat4 res;
        res(0, 0) = s.x;
        res(1, 1) = s.y;
        res(2, 2) = s.z;
        return res;
    }
    static Mat4 scale(float sx, float sy, float sz) {
        return scale(Vec3(sx, sy, sz));
    }
    static Mat4 scale(float s) {
        return scale(Vec3(s, s, s));
    }

    // ------------------------------------------
    // 3. Rotation Matrices
    // ------------------------------------------
    static Mat4 rotateX(float angleRad) {
        Mat4 res;
        float c = std::cos(angleRad);
        float s = std::sin(angleRad);
        res(1, 1) = c;  res(1, 2) = -s;
        res(2, 1) = s;  res(2, 2) = c;
        return res;
    }

    static Mat4 rotateY(float angleRad) {
        Mat4 res;
        float c = std::cos(angleRad);
        float s = std::sin(angleRad);
        res(0, 0) = c;   res(0, 2) = s;
        res(2, 0) = -s;  res(2, 2) = c;
        return res;
    }

    static Mat4 rotateZ(float angleRad) {
        Mat4 res;
        float c = std::cos(angleRad);
        float s = std::sin(angleRad);
        res(0, 0) = c;  res(0, 1) = -s;
        res(1, 0) = s;  res(1, 1) = c;
        return res;
    }

    static Mat4 rotate(float angleRad, Vec3 axis) {
        axis = axis.normalized();
        float c = std::cos(angleRad);
        float s = std::sin(angleRad);
        float t = 1.0f - c;

        Mat4 res;
        res(0, 0) = t * axis.x * axis.x + c;
        res(0, 1) = t * axis.x * axis.y - s * axis.z;
        res(0, 2) = t * axis.x * axis.z + s * axis.y;

        res(1, 0) = t * axis.x * axis.y + s * axis.z;
        res(1, 1) = t * axis.y * axis.y + c;
        res(1, 2) = t * axis.y * axis.z - s * axis.x;

        res(2, 0) = t * axis.x * axis.z - s * axis.y;
        res(2, 1) = t * axis.y * axis.z + s * axis.x;
        res(2, 2) = t * axis.z * axis.z + c;

        return res;
    }

    // ------------------------------------------
    // 4. Shearing Matrices (Requirement Demonstration)
    // Shear along axes:
    // shearX(sy, sz): x' = x + sy*y + sz*z
    // shearY(sx, sz): y' = y + sx*x + sz*z
    // shearZ(sx, sy): z' = z + sx*x + sy*y
    // ------------------------------------------
    static Mat4 shearX(float sy, float sz) {
        Mat4 res;
        res(0, 1) = sy;
        res(0, 2) = sz;
        return res;
    }

    static Mat4 shearY(float sx, float sz) {
        Mat4 res;
        res(1, 0) = sx;
        res(1, 2) = sz;
        return res;
    }

    static Mat4 shearZ(float sx, float sy) {
        Mat4 res;
        res(2, 0) = sx;
        res(2, 1) = sy;
        return res;
    }

    static Mat4 shear(float sXY, float sXZ, float sYX, float sYZ, float sZX, float sZY) {
        Mat4 res;
        res(0, 1) = sXY; res(0, 2) = sXZ;
        res(1, 0) = sYX; res(1, 2) = sYZ;
        res(2, 0) = sZX; res(2, 1) = sZY;
        return res;
    }

    // ------------------------------------------
    // 5. Projection Matrices
    // ------------------------------------------
    static Mat4 perspective(float fovYRad, float aspect, float zNear, float zFar) {
        Mat4 res;
        float tanHalfFov = std::tan(fovYRad * 0.5f);
        
        // Zero out identity diagonal
        res(0, 0) = 1.0f / (aspect * tanHalfFov);
        res(1, 1) = 1.0f / tanHalfFov;
        res(2, 2) = -(zFar + zNear) / (zFar - zNear);
        res(2, 3) = -(2.0f * zFar * zNear) / (zFar - zNear);
        res(3, 2) = -1.0f;
        res(3, 3) = 0.0f;
        return res;
    }

    static Mat4 ortho(float left, float right, float bottom, float top, float zNear, float zFar) {
        Mat4 res;
        res(0, 0) = 2.0f / (right - left);
        res(1, 1) = 2.0f / (top - bottom);
        res(2, 2) = -2.0f / (zFar - zNear);
        res(0, 3) = -(right + left) / (right - left);
        res(1, 3) = -(top + bottom) / (top - bottom);
        res(2, 3) = -(zFar + zNear) / (zFar - zNear);
        return res;
    }

    // ------------------------------------------
    // 6. Camera LookAt Matrix
    // ------------------------------------------
    static Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
        Vec3 f = (center - eye).normalized();
        Vec3 s = cross(f, up).normalized();
        Vec3 u = cross(s, f);

        Mat4 res;
        res(0, 0) = s.x;  res(0, 1) = s.y;  res(0, 2) = s.z;  res(0, 3) = -dot(s, eye);
        res(1, 0) = u.x;  res(1, 1) = u.y;  res(1, 2) = u.z;  res(1, 3) = -dot(u, eye);
        res(2, 0) = -f.x; res(2, 1) = -f.y; res(2, 2) = -f.z; res(2, 3) = dot(f, eye);
        res(3, 0) = 0.0f; res(3, 1) = 0.0f; res(3, 2) = 0.0f; res(3, 3) = 1.0f;
        return res;
    }

    // ------------------------------------------
    // 7. Transpose & Inverse (3x3 Normal Matrix Support)
    // ------------------------------------------
    Mat4 transpose() const {
        Mat4 res;
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                res(r, c) = (*this)(c, r);
            }
        }
        return res;
    }

    // Quick inverse for general 4x4 matrix
    Mat4 inverse() const {
        const float* a = m;
        Mat4 inv;
        float* o = inv.m;

        o[0] = a[5]  * a[10] * a[15] - 
               a[5]  * a[11] * a[14] - 
               a[9]  * a[6]  * a[15] + 
               a[9]  * a[7]  * a[14] +
               a[13] * a[6]  * a[11] - 
               a[13] * a[7]  * a[10];

        o[4] = -a[4]  * a[10] * a[15] + 
                a[4]  * a[11] * a[14] + 
                a[8]  * a[6]  * a[15] - 
                a[8]  * a[7]  * a[14] - 
                a[12] * a[6]  * a[11] + 
                a[12] * a[7]  * a[10];

        o[8] = a[4]  * a[9] * a[15] - 
               a[4]  * a[11] * a[13] - 
               a[8]  * a[5] * a[15] + 
               a[8]  * a[7] * a[13] + 
               a[12] * a[5] * a[11] - 
               a[12] * a[7] * a[9];

        o[12] = -a[4]  * a[9] * a[14] + 
                 a[4]  * a[10] * a[13] +
                 a[8]  * a[5] * a[14] - 
                 a[8]  * a[6] * a[13] - 
                 a[12] * a[5] * a[10] + 
                 a[12] * a[6] * a[9];

        o[1] = -a[1]  * a[10] * a[15] + 
                a[1]  * a[11] * a[14] + 
                a[9]  * a[2] * a[15] - 
                a[9]  * a[3] * a[14] - 
                a[13] * a[2] * a[11] + 
                a[13] * a[3] * a[10];

        o[5] = a[0]  * a[10] * a[15] - 
               a[0]  * a[11] * a[14] - 
               a[8]  * a[2] * a[15] + 
               a[8]  * a[3] * a[14] + 
               a[12] * a[2] * a[11] - 
               a[12] * a[3] * a[10];

        o[9] = -a[0]  * a[9] * a[15] + 
                a[0]  * a[11] * a[13] + 
                a[8]  * a[1] * a[15] - 
                a[8]  * a[3] * a[13] - 
                a[12] * a[1] * a[11] + 
                a[12] * a[3] * a[9];

        o[13] = a[0]  * a[9] * a[14] - 
                a[0]  * a[10] * a[13] - 
                a[8]  * a[1] * a[14] + 
                a[8]  * a[2] * a[13] + 
                a[12] * a[1] * a[10] - 
                a[12] * a[2] * a[9];

        o[2] = a[1]  * a[6] * a[15] - 
               a[1]  * a[7] * a[14] - 
               a[5]  * a[2] * a[15] + 
               a[5]  * a[3] * a[14] + 
               a[13] * a[2] * a[7] - 
               a[13] * a[3] * a[6];

        o[6] = -a[0]  * a[6] * a[15] + 
                a[0]  * a[7] * a[14] + 
                a[4]  * a[2] * a[15] - 
                a[4]  * a[3] * a[14] - 
                a[12] * a[2] * a[7] + 
                a[12] * a[3] * a[6];

        o[10] = a[0]  * a[5] * a[15] - 
                a[0]  * a[7] * a[13] - 
                a[4]  * a[1] * a[15] + 
                a[4]  * a[3] * a[13] + 
                a[12] * a[1] * a[7] - 
                a[12] * a[3] * a[5];

        o[14] = -a[0]  * a[5] * a[14] + 
                 a[0]  * a[6] * a[13] + 
                 a[4]  * a[1] * a[14] - 
                 a[4]  * a[2] * a[13] - 
                 a[12] * a[1] * a[6] + 
                 a[12] * a[2] * a[5];

        o[3] = -a[1] * a[6] * a[11] + 
                a[1] * a[7] * a[10] + 
                a[5] * a[2] * a[11] - 
                a[5] * a[3] * a[10] - 
                a[9] * a[2] * a[7] + 
                a[9] * a[3] * a[6];

        o[7] = a[0] * a[6] * a[11] - 
               a[0] * a[7] * a[10] - 
               a[4] * a[2] * a[11] + 
               a[4] * a[3] * a[10] + 
               a[8] * a[2] * a[7] - 
               a[8] * a[3] * a[6];

        o[11] = -a[0] * a[5] * a[11] + 
                 a[0] * a[7] * a[9] + 
                 a[4] * a[1] * a[11] - 
                 a[4] * a[3] * a[9] - 
                 a[8] * a[1] * a[7] + 
                 a[8] * a[3] * a[5];

        o[15] = a[0] * a[5] * a[10] - 
                a[0] * a[6] * a[9] - 
                a[4] * a[1] * a[10] + 
                a[4] * a[2] * a[9] + 
                a[8] * a[1] * a[6] - 
                a[8] * a[2] * a[5];

        float det = a[0] * o[0] + a[1] * o[4] + a[2] * o[8] + a[3] * o[12];
        if (std::abs(det) < 1e-8f) {
            return Mat4::identity();
        }

        float invDet = 1.0f / det;
        for (int i = 0; i < 16; i++) {
            o[i] *= invDet;
        }
        return inv;
    }
};

} // namespace m3d

#endif // MATH3D_HPP
