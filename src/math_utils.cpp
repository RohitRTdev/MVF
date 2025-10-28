#include "math_utils.h"
#include <cstring>
#include <cmath>

Vector2f::Vector2f() {}
Vector2f::Vector2f(float _x, float _y) : x(_x), y(_y) {}

Vector3f::Vector3f() {}
Vector3f::Vector3f(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
Vector3f::Vector3f(float f) : x(f), y(f), z(f) {}

Vector3f& Vector3f::operator+=(const Vector3f& r) {
    x += r.x; y += r.y; z += r.z;
    return *this;
}

Vector3f& Vector3f::operator-=(const Vector3f& r) {
    x -= r.x; y -= r.y; z -= r.z;
    return *this;
}

Vector3f& Vector3f::operator*=(float f) {
    x *= f; y *= f; z *= f;
    return *this;
}

Vector3f::operator const float*() const { return &x; }

float Vector3f::dot(const Vector3f& v) const {
    return x * v.x + y * v.y + z * v.z;
}

Vector3f Vector3f::operator-() const {
    return Vector3f(-x, -y, -z);
}

Vector3f Vector3f::operator-(const Vector3f& r) const {
    return Vector3f(x - r.x, y - r.y, z - r.z);
}

Vector3f Vector3f::operator+(const Vector3f& r) const {
    return Vector3f(x + r.x, y + r.y, z + r.z);
}
    
Vector3f Vector3f::operator*(float f) const {
    return Vector3f(x * f, y * f, z * f);
}

Vector3f Vector3f::cross(const Vector3f& v) const {
    return Vector3f(
        y * v.z - z * v.y,
        z * v.x - x * v.z,
        x * v.y - y * v.x
    );
}

Vector3f& Vector3f::normalize() {
    float Length = sqrtf(x * x + y * y + z * z);
    x /= Length; y /= Length; z /= Length;
    return *this;
}

float Vector3f::dist(const Vector3f& other) {
    float diffX = x - other.x;
    float diffY = y - other.y;
    float diffZ = z - other.z;
    return sqrtf(diffX * diffX + diffY * diffY + diffZ * diffZ);
}

float Vector3f::length() {
    return sqrtf(x * x + y * y + z * z);
}

void Vector3f::print() const {
    printf("(%.02f, %.02f, %.02f)", x, y, z);
}

Vector4f::Vector4f() {}
Vector4f::Vector4f(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
void Vector4f::print() const {
    printf("(%.02f, %.02f, %.02f, %.02f)", x, y, z, w);
}

PersProjInfo::PersProjInfo() {}
PersProjInfo::PersProjInfo(float _FOV, float _Width, float _Height, float _zNear, float _zFar)
    : FOV(_FOV), Width(_Width), Height(_Height), zNear(_zNear), zFar(_zFar) {}

Matrix4f::Matrix4f() {}

Matrix4f::Matrix4f(float a00, float a01, float a02, float a03,
                   float a10, float a11, float a12, float a13,
                   float a20, float a21, float a22, float a23,
                   float a30, float a31, float a32, float a33) {
    m[0][0] = a00; m[0][1] = a01; m[0][2] = a02; m[0][3] = a03;
    m[1][0] = a10; m[1][1] = a11; m[1][2] = a12; m[1][3] = a13;
    m[2][0] = a20; m[2][1] = a21; m[2][2] = a22; m[2][3] = a23;
    m[3][0] = a30; m[3][1] = a31; m[3][2] = a32; m[3][3] = a33;
}

void Matrix4f::set_zero() { 
    std::memset(m, 0, sizeof(m));
}

Matrix4f Matrix4f::transpose() const {
    Matrix4f n;
    for (unsigned int i = 0; i < 4; i++)
        for (unsigned int j = 0; j < 4; j++)
            n.m[i][j] = m[j][i];
    return n;
}

Matrix4f Matrix4f::create_identity() {
    Matrix4f m;
    m.init_identity();
    
    return m;
}

void Matrix4f::init_identity() {
    m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = 0.0f;
    m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = 0.0f;
    m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = 0.0f;
    m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
}

Matrix4f Matrix4f::operator*(const Matrix4f& Right) const {
    Matrix4f Ret;
    for (unsigned int i = 0; i < 4; i++)
        for (unsigned int j = 0; j < 4; j++)
            Ret.m[i][j] = m[i][0] * Right.m[0][j] +
                          m[i][1] * Right.m[1][j] +
                          m[i][2] * Right.m[2][j] +
                          m[i][3] * Right.m[3][j];
    return Ret;
}

Vector4f Matrix4f::operator*(const Vector4f& v) const {
    Vector4f r;
    r.x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w;
    r.y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w;
    r.z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w;
    r.w = m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * v.w;
    return r;
}

Vector3f Matrix4f::operator*(const Vector3f& v) const {
    Vector4f r(v.x, v.y, v.z, 1.0);
    Vector4f i = *this * r;

    return Vector3f(i.x, i.y, i.z);
}

Vector3f Matrix4f::dir_transform(const Vector3f& v) const {
    Vector4f r(v.x, v.y, v.z, 0.0);
    Vector4f i = *this * r;
    
    return Vector3f(i.x, i.y, i.z);
}


Matrix4f::operator const float*() const { 
    return &(m[0][0]);
}

void Matrix4f::print() const {
    for (int i = 0; i < 4; i++)
        printf("%6.2f %6.2f %6.2f %6.2f\n", m[i][0], m[i][1], m[i][2], m[i][3]);
}

float Matrix4f::determinant() const {
    return m[0][0] * m[1][1] * m[2][2] * m[3][3] - m[0][0] * m[1][1] * m[2][3] * m[3][2] + m[0][0] * m[1][2] * m[2][3] * m[3][1] - m[0][0] * m[1][2] * m[2][1] * m[3][3]
        + m[0][0] * m[1][3] * m[2][1] * m[3][2] - m[0][0] * m[1][3] * m[2][2] * m[3][1] - m[0][1] * m[1][2] * m[2][3] * m[3][0] + m[0][1] * m[1][2] * m[2][0] * m[3][3]
        - m[0][1] * m[1][3] * m[2][0] * m[3][2] + m[0][1] * m[1][3] * m[2][2] * m[3][0] - m[0][1] * m[1][0] * m[2][2] * m[3][3] + m[0][1] * m[1][0] * m[2][3] * m[3][2]
        + m[0][2] * m[1][3] * m[2][0] * m[3][1] - m[0][2] * m[1][3] * m[2][1] * m[3][0] + m[0][2] * m[1][0] * m[2][1] * m[3][3] - m[0][2] * m[1][0] * m[2][3] * m[3][1]
        + m[0][2] * m[1][1] * m[2][3] * m[3][0] - m[0][2] * m[1][1] * m[2][0] * m[3][3] - m[0][3] * m[1][0] * m[2][1] * m[3][2] + m[0][3] * m[1][0] * m[2][2] * m[3][1]
        - m[0][3] * m[1][1] * m[2][2] * m[3][0] + m[0][3] * m[1][1] * m[2][0] * m[3][2] - m[0][3] * m[1][2] * m[2][0] * m[3][1] + m[0][3] * m[1][2] * m[2][1] * m[3][0];
}

Matrix4f& Matrix4f::inverse() {
    float inv[16], det;
    float* m = &this->m[0][0];

    inv[0] = m[5]  * m[10] * m[15] - 
             m[5]  * m[11] * m[14] - 
             m[9]  * m[6]  * m[15] + 
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] - 
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] + 
              m[4]  * m[11] * m[14] + 
              m[8]  * m[6]  * m[15] - 
              m[8]  * m[7]  * m[14] - 
              m[12] * m[6]  * m[11] + 
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] - 
             m[4]  * m[11] * m[13] - 
             m[8]  * m[5] * m[15] + 
             m[8]  * m[7] * m[13] + 
             m[12] * m[5] * m[11] - 
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] + 
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] - 
               m[8]  * m[6] * m[13] - 
               m[12] * m[5] * m[10] + 
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] + 
              m[1]  * m[11] * m[14] + 
              m[9]  * m[2] * m[15] - 
              m[9]  * m[3] * m[14] - 
              m[13] * m[2] * m[11] + 
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] - 
             m[0]  * m[11] * m[14] - 
             m[8]  * m[2] * m[15] + 
             m[8]  * m[3] * m[14] + 
             m[12] * m[2] * m[11] - 
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] + 
              m[0]  * m[11] * m[13] + 
              m[8]  * m[1] * m[15] - 
              m[8]  * m[3] * m[13] - 
              m[12] * m[1] * m[11] + 
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] - 
              m[0]  * m[10] * m[13] - 
              m[8]  * m[1] * m[14] + 
              m[8]  * m[2] * m[13] + 
              m[12] * m[1] * m[10] - 
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] - 
             m[1]  * m[7] * m[14] - 
             m[5]  * m[2] * m[15] + 
             m[5]  * m[3] * m[14] + 
             m[13] * m[2] * m[7] - 
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] + 
              m[0]  * m[7] * m[14] + 
              m[4]  * m[2] * m[15] - 
              m[4]  * m[3] * m[14] - 
              m[12] * m[2] * m[7] + 
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] - 
              m[0]  * m[7] * m[13] - 
              m[4]  * m[1] * m[15] + 
              m[4]  * m[3] * m[13] + 
              m[12] * m[1] * m[7] - 
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] + 
               m[0]  * m[6] * m[13] + 
               m[4]  * m[1] * m[14] - 
               m[4]  * m[2] * m[13] - 
               m[12] * m[1] * m[6] + 
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + 
              m[1] * m[7] * m[10] + 
              m[5] * m[2] * m[11] - 
              m[5] * m[3] * m[10] - 
              m[9] * m[2] * m[7] + 
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - 
             m[0] * m[7] * m[10] - 
             m[4] * m[2] * m[11] + 
             m[4] * m[3] * m[10] + 
             m[8] * m[2] * m[7] - 
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + 
               m[0] * m[7] * m[9] + 
               m[4] * m[1] * m[11] - 
               m[4] * m[3] * m[9] - 
               m[8] * m[1] * m[7] + 
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - 
              m[0] * m[6] * m[9] - 
              m[4] * m[1] * m[10] + 
              m[4] * m[2] * m[9] + 
              m[8] * m[1] * m[6] - 
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0) {
        // Singular matrix, can't invert
        return *this;
    }

    det = 1.0f / det;

    for (int i = 0; i < 16; i++)
        ((float*)m)[i] = inv[i] * det;

    return *this;
}

void Matrix4f::init_scale_transform(float ScaleX, float ScaleY, float ScaleZ) {
    init_identity();
    m[0][0] = ScaleX;
    m[1][1] = ScaleY;
    m[2][2] = ScaleZ;
}

void Matrix4f::init_rotate_transform(float RotateX, float RotateY, float RotateZ) {
    Matrix4f rx, ry, rz;
    float x = ToRadian(RotateX);
    float y = ToRadian(RotateY);
    float z = ToRadian(RotateZ);

    rx.init_identity();
    ry.init_identity();
    rz.init_identity();

    rx.m[1][1] = cosf(x); rx.m[1][2] = -sinf(x);
    rx.m[2][1] = sinf(x); rx.m[2][2] = cosf(x);

    ry.m[0][0] = cosf(y); ry.m[0][2] = sinf(y);
    ry.m[2][0] = -sinf(y); ry.m[2][2] = cosf(y);

    rz.m[0][0] = cosf(z); rz.m[0][1] = -sinf(z);
    rz.m[1][0] = sinf(z); rz.m[1][1] = cosf(z);

    *this = rz * ry * rx;
}

void Matrix4f::init_axis_rotate_transform(const Vector3f& axis, float angle) {
    float x = axis.x, y = axis.y, z = axis.z;
    float c = cosf(angle);
    float s = sinf(angle);
    float t = 1.0f - c;

    m[0][0] = t * x * x + c;
    m[0][1] = t * x * y - s * z;
    m[0][2] = t * x * z + s * y;
    m[0][3] = 0.0f;

    m[1][0] = t * x * y + s * z;
    m[1][1] = t * y * y + c;
    m[1][2] = t * y * z - s * x;
    m[1][3] = 0.0f;

    m[2][0] = t * x * z - s * y;
    m[2][1] = t * y * z + s * x;
    m[2][2] = t * z * z + c;
    m[2][3] = 0.0f;

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}

void Matrix4f::init_translation_transform(float x, float y, float z) {
    init_identity();
    m[0][3] = x;
    m[1][3] = y;
    m[2][3] = z;
}

void Matrix4f::init_camera_transform(const Vector3f& Position, const Vector3f& Target, const Vector3f& Up) {
    Vector3f Z = Target - Position;
    Z.normalize();
    Vector3f X = Z.cross(Up);
    X.normalize();
    Vector3f Y = X.cross(Z);
    
    auto x_t = -X.dot(Position);
    auto y_t = -Y.dot(Position);
    auto z_t = Z.dot(Position);

    m[0][0] = X.x; m[0][1] = X.y; m[0][2] = X.z; m[0][3] = x_t;
    m[1][0] = Y.x; m[1][1] = Y.y; m[1][2] = Y.z; m[1][3] = y_t;
    m[2][0] = -Z.x; m[2][1] = -Z.y; m[2][2] = -Z.z; m[2][3] = z_t;
    m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
}

void Matrix4f::init_pers_proj_transform(const PersProjInfo& p) {
    float ar = p.Width / p.Height;
    float zRange = p.zFar - p.zNear;  // flip sign for clarity
    float tanHalfFOV = tanf(ToRadian(p.FOV / 2.0f));

    m[0][0] = 1.0f / (tanHalfFOV * ar);
    m[1][1] = 1.0f / tanHalfFOV;
    m[2][2] = -(p.zFar + p.zNear) / zRange;
    m[2][3] = -(2.0f * p.zFar * p.zNear) / zRange;
    m[3][2] = -1.0f;
    m[3][3] = 0.0f;
}

void Matrix4f::init_ortho_proj_transform(const OrthoProjInfo& p) {
    init_identity();
    m[0][0] = 2.0f / (p.right - p.left);
    m[1][1] = 2.0f / (p.top - p.bottom);
    m[2][2] = -2.0f / (p.zFar - p.zNear);
    m[0][3] = -(p.right + p.left) / (p.right - p.left);
    m[1][3] = -(p.top + p.bottom) / (p.top - p.bottom);
    m[2][3] = -(p.zFar + p.zNear) / (p.zFar - p.zNear);
}
