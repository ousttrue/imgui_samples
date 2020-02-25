#pragma once

#ifndef IM3D_ASSERT
#include <cassert>
#define IM3D_ASSERT(e) assert(e)
#endif

#ifndef IM3D_VERTEX_ALIGNMENT
#define IM3D_VERTEX_ALIGNMENT 4
#endif

namespace Im3d
{
struct Vec2;
struct Vec3;
struct Vec4;
struct Mat3;
struct Mat4;
struct Color;
typedef unsigned int U32;
typedef U32 Id;
// constexpr Id Id_Invalid = 0;

struct Vec2
{
    float x, y;
    Vec2() {}
    Vec2(float _xy) : x(_xy), y(_xy) {}
    Vec2(float _x, float _y) : x(_x), y(_y) {}
    operator float *() { return &x; }
    operator const float *() const { return &x; }
#ifdef IM3D_VEC2_APP
    IM3D_VEC2_APP
#endif
};
struct Vec3
{
    float x, y, z;
    Vec3() {}
    Vec3(float _xyz) : x(_xyz), y(_xyz), z(_xyz) {}
    Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    Vec3(const Vec2 &_xy, float _z) : x(_xy.x), y(_xy.y), z(_z) {}
    Vec3(const Vec4 &_v); // discards w
    operator float *() { return &x; }
    operator const float *() const { return &x; }
#ifdef IM3D_VEC3_APP
    IM3D_VEC3_APP
#endif
};
struct Vec4
{
    float x, y, z, w;
    Vec4() {}
    Vec4(float _xyzw) : x(_xyzw), y(_xyzw), z(_xyzw), w(_xyzw) {}
    Vec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
    Vec4(const Vec3 &_xyz, float _w) : x(_xyz.x), y(_xyz.y), z(_xyz.z), w(_w) {}
    Vec4(Color _rgba);
    operator float *() { return &x; }
    operator const float *() const { return &x; }
#ifdef IM3D_VEC4_APP
    IM3D_VEC4_APP
#endif
};
struct Mat3
{
    float m[3 * 3]; // column-major unless IM3D_MATRIX_ROW_MAJOR defined
    Mat3() {}
    Mat3(float _diagonal);
    Mat3(
        float m00, float m01, float m02,
        float m10, float m11, float m12,
        float m20, float m21, float m22);
    Mat3(const Vec3 &_colX, const Vec3 &_colY, const Vec3 &_colZ);
    Mat3(const Mat4 &_mat4); // extract upper 3x3
    operator float *() { return m; }
    operator const float *() const { return m; }

    Vec3 getCol(int _i) const;
    Vec3 getRow(int _i) const;
    void setCol(int _i, const Vec3 &_v);
    void setRow(int _i, const Vec3 &_v);

    Vec3 getScale() const;
    void setScale(const Vec3 &_scale);

    float operator()(int _row, int _col) const
    {
#ifdef IM3D_MATRIX_ROW_MAJOR
        int i = _row * 3 + _col;
#else
        int i = _col * 3 + _row;
#endif
        return m[i];
    }
    float &operator()(int _row, int _col)
    {
#ifdef IM3D_MATRIX_ROW_MAJOR
        int i = _row * 3 + _col;
#else
        int i = _col * 3 + _row;
#endif
        return m[i];
    }

#ifdef IM3D_MAT3_APP
    IM3D_MAT3_APP
#endif
};
struct Mat4
{
    float m[4 * 4]; // column-major unless IM3D_MATRIX_ROW_MAJOR defined
    Mat4() {}
    Mat4(float _diagonal);
    Mat4(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30 = 0.0f, float m31 = 0.0f, float m32 = 0.0f, float m33 = 1.0f);
    Mat4(const Mat3 &_mat3);
    Mat4(const Vec3 &_translation, const Mat3 &_rotation, const Vec3 &_scale);
    operator float *() { return m; }
    operator const float *() const { return m; }

    Vec4 getCol(int _i) const;
    Vec4 getRow(int _i) const;
    void setCol(int _i, const Vec4 &_v);
    void setRow(int _i, const Vec4 &_v);

    Vec3 getTranslation() const;
    void setTranslation(const Vec3 &_translation);
    Mat3 getRotation() const;
    void setRotation(const Mat3 &_rotation);
    Vec3 getScale() const;
    void setScale(const Vec3 &_scale);

    float operator()(int _row, int _col) const
    {
#ifdef IM3D_MATRIX_ROW_MAJOR
        int i = _row * 4 + _col;
#else
        int i = _col * 4 + _row;
#endif
        return m[i];
    }
    float &operator()(int _row, int _col)
    {
#ifdef IM3D_MATRIX_ROW_MAJOR
        int i = _row * 4 + _col;
#else
        int i = _col * 4 + _row;
#endif
        return m[i];
    }

#ifdef IM3D_MAT4_APP
    IM3D_MAT4_APP
#endif
};
struct Color
{
    U32 v; // rgba8 (MSB = r)
    constexpr Color() : v(0) {}
    constexpr Color(U32 _rgba) : v(_rgba) {}
    Color(const Vec4 &_rgba);
    Color(const Vec3 &_rgb, float _alpha);
    Color(float _r, float _g, float _b, float _a = 1.0f);

    operator U32() const { return v; }

    void set(int _i, float _val)
    {
        _i *= 8;
        U32 mask = 0xff << _i;
        v = (v & ~mask) | ((U32)(_val * 255.0f) << _i);
    }
    void setR(float _val) { set(3, _val); }
    void setG(float _val) { set(2, _val); }
    void setB(float _val) { set(1, _val); }
    void setA(float _val) { set(0, _val); }

    float get(int _i) const
    {
        _i *= 8;
        U32 mask = 0xff << _i;
        return (float)((v & mask) >> _i) / 255.0f;
    }
    float getR() const { return get(3); }
    float getG() const { return get(2); }
    float getB() const { return get(1); }
    float getA() const { return get(0); }
};

constexpr Color Color_Black = Color(0x000000ff);
constexpr Color Color_White = Color(0xffffffff);
constexpr Color Color_Red = Color(0xff0000ff);
constexpr Color Color_Green = Color(0x00ff00ff);
constexpr Color Color_Blue = Color(0x0000ffff);
constexpr Color Color_Magenta = Color(0xff00ffff);
constexpr Color Color_Yellow = Color(0xffff00ff);
constexpr Color Color_Cyan = Color(0x00ffffff);

constexpr Color Color_GizmoHighlight = Color(0xffc745ff);

} // namespace Im3d
