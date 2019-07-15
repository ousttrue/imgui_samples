#pragma once
#include <array>

namespace amth
{
inline float dot(const float *row, const float *col)
{
    auto a = row[0] * col[0];
    auto b = row[1] * col[4];
    auto c = row[2] * col[8];
    auto d = row[3] * col[12];
    auto value = a + b + c + d;
    return value;
}

inline std::array<float, 16> Mult(const std::array<float, 16> &l, const std::array<float, 16> &r)
{
    auto _11 = dot(&l[0], &r[0]);
    auto _12 = dot(&l[0], &r[1]);
    auto _13 = dot(&l[0], &r[2]);
    auto _14 = dot(&l[0], &r[3]);

    auto _21 = dot(&l[4], &r[0]);
    auto _22 = dot(&l[4], &r[1]);
    auto _23 = dot(&l[4], &r[2]);
    auto _24 = dot(&l[4], &r[3]);

    auto _31 = dot(&l[8], &r[0]);
    auto _32 = dot(&l[8], &r[1]);
    auto _33 = dot(&l[8], &r[2]);
    auto _34 = dot(&l[8], &r[3]);

    auto _41 = dot(&l[12], &r[0]);
    auto _42 = dot(&l[12], &r[1]);
    auto _43 = dot(&l[12], &r[2]);
    auto _44 = dot(&l[12], &r[3]);

    return std::array<float, 16>{
        _11,
        _12,
        _13,
        _14,
        _21,
        _22,
        _23,
        _24,
        _31,
        _32,
        _33,
        _34,
        _41,
        _42,
        _43,
        _44,
    };
}

inline void Transpose(std::array<float, 16> &m)
{
    std::swap(m[1], m[4]);
    std::swap(m[2], m[8]);
    std::swap(m[3], m[12]);
    std::swap(m[6], m[9]);
    std::swap(m[7], m[13]);
    std::swap(m[11], m[14]);
}

inline std::array<float, 16> IdentityMatrix()
{
    return std::array<float, 16>{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };
}

} // namespace amth
