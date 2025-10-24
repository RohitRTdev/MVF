#pragma once

#include <math.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <numbers>

#define ToRadian(x) (float)(((x) * (std::numbers::pi) / 180.0f))
#define ToDegree(x) (float)(((x) * 180.0f / (std::numbers::pi)))

inline float RandomFloat() {
	float Max = RAND_MAX;
	return ((float) rand() / Max);
}

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
    float Dot(const Vector3f& v) const;
    Vector3f Cross(const Vector3f& v) const;
    Vector3f& Normalize();
    float dist(const Vector3f& other);
    float length();
    void Print() const;
};

struct Vector4f {
    float x;
    float y;
    float z;
    float w;
    Vector4f();
    Vector4f(float _x, float _y, float _z, float _w);
    void Print() const;
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
    static Matrix4f CreateIdentity();
    
    Matrix4f();
    Matrix4f(float a00, float a01, float a02, float a03,
             float a10, float a11, float a12, float a13,
             float a20, float a21, float a22, float a23,
             float a30, float a31, float a32, float a33);
    void SetZero();
    Matrix4f Transpose() const;
    void InitIdentity();
    Matrix4f operator*(const Matrix4f& Right) const;
    Vector4f operator*(const Vector4f& v) const;
    Vector3f operator*(const Vector3f& v) const; 
    operator const float*() const;
    void Print() const;
    float Determinant() const;
    Matrix4f& Inverse();
    void InitScaleTransform(float ScaleX, float ScaleY, float ScaleZ);
    void InitRotateTransform(float RotateX, float RotateY, float RotateZ);
    void InitAxisRotateTransform(const Vector3f& axis, float angle);
    void InitTranslationTransform(float x, float y, float z);
    void InitCameraTransform(const Vector3f& Position, const Vector3f& Target, const Vector3f& Up);
    void InitPersProjTransform(const PersProjInfo& p);
    void InitOrthoProjTransform(const OrthoProjInfo& p); 
    Vector3f DirTransform(const Vector3f& v) const; 
};