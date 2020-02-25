#pragma once
#include "im3d_math.h"
#include <cstdlib>

namespace Im3d
{

constexpr Color Color_GizmoHighlight = Color(0xffc745ff);

inline static float Snap(float _val, float _snap)
{
    if (_snap > 0.0f)
    {
        return floorf(_val / _snap) * _snap;
    }
    return _val;
}

inline static Vec3 Snap(const Vec3 &_val, float _snap)
{
    if (_snap > 0.0f)
    {
        return Vec3(floorf(_val.x / _snap) * _snap, floorf(_val.y / _snap) * _snap, floorf(_val.z / _snap) * _snap);
    }
    return _val;
}

inline static Vec3 Snap(const Vec3 &_pos, const Plane &_plane, float _snap)
{
    if (_snap > 0.0f)
    {
        // get basis vectors on the plane
        Mat3 basis = AlignZ(_plane.m_normal);
        Vec3 i = basis.getCol(0);
        Vec3 j = basis.getCol(1);

        // decompose _pos in terms of the basis vectors
        i = i * Dot(_pos, i);
        j = j * Dot(_pos, j);

        // snap the vector lengths
        float ilen = Length(i);
        float jlen = Length(j);

        if (ilen < 1e-7f || jlen < 1e-7f)
        { // \hack prevent DBZ when _pos is 0
            return _pos;
        }

        i = i / ilen;
        ilen = floorf(ilen / _snap) * _snap;
        i = i * ilen;
        j = j / jlen;
        jlen = floorf(jlen / _snap) * _snap;
        j = j * jlen;

        return i + j;
    }
    return _pos;
}
} // namespace Im3d
