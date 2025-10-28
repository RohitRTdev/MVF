#pragma once

#include <math.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <numbers>

#define ToRadian(x) (float)(((x) * (std::numbers::pi) / 180.0f))
#define ToDegree(x) (float)(((x) * 180.0f / (std::numbers::pi)))

inline float random_float() {
	float Max = RAND_MAX;
	return ((float) rand() / Max);
}

#pragma pack(push, 1)
struct Vector2i {
    int x;
    int y;
};

struct Vector2f {
    float x;
    float y;
    Vector2f();
    Vector2f(float _x, float _y);
};

struct Vector3f {
    float x;
    float y;
    float z;
    Vector3f();
    Vector3f(float _x, float _y, float _z);
    Vector3f(float f);
    Vector3f& operator+=(const Vector3f& r);
    Vector3f& operator-=(const Vector3f& r);
    Vector3f& operator*=(float f);
    Vector3f operator*(float f) const;
    operator const float*() const;
    Vector3f operator+(const Vector3f& r) const;
    Vector3f operator-(const Vector3f& r) const;
    Vector3f operator-() const;
    float dot(const Vector3f& v) const;
    Vector3f cross(const Vector3f& v) const;
    Vector3f& normalize();
    float dist(const Vector3f& other);
    float length();
    void print() const;
};

struct Vector4f {
    float x;
    float y;
    float z;
    float w;
    Vector4f();
    Vector4f(float _x, float _y, float _z, float _w);
    void print() const;
};

struct PersProjInfo {
    float FOV;
    float Width;
    float Height;
    float zNear;
    float zFar;
    PersProjInfo();
    PersProjInfo(float _FOV, float _Width, float _Height, float _zNear, float _zFar);
};

struct OrthoProjInfo {
    float bottom;
    float top;
    float left;
    float right;
    float zNear;
    float zFar;
};

struct Matrix4f {
    float m[4][4];
    static Matrix4f create_identity();
    
    Matrix4f();
    Matrix4f(float a00, float a01, float a02, float a03,
             float a10, float a11, float a12, float a13,
             float a20, float a21, float a22, float a23,
             float a30, float a31, float a32, float a33);
    void set_zero();
    Matrix4f transpose() const;
    void init_identity();
    Matrix4f operator*(const Matrix4f& Right) const;
    Vector4f operator*(const Vector4f& v) const;
    Vector3f operator*(const Vector3f& v) const; 
    operator const float*() const;
    void print() const;
    float determinant() const;
    Matrix4f& inverse();
    void init_scale_transform(float ScaleX, float ScaleY, float ScaleZ);
    void init_rotate_transform(float RotateX, float RotateY, float RotateZ);
    void init_axis_rotate_transform(const Vector3f& axis, float angle);
    void init_translation_transform(float x, float y, float z);
    void init_camera_transform(const Vector3f& Position, const Vector3f& Target, const Vector3f& Up);
    void init_pers_proj_transform(const PersProjInfo& p);
    void init_ortho_proj_transform(const OrthoProjInfo& p); 
    Vector3f dir_transform(const Vector3f& v) const; 
};
#pragma pack(pop)