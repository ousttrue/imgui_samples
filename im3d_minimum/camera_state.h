#pragma once
#include <array>

namespace camera
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

struct CameraState
{
    void *userData = nullptr;
    uint32_t UserDataAsUInt() const
    {
        return (uint32_t)(uint64_t)userData;
    }

    std::array<float, 4> clearColor = {0, 0.2f, 0.4f, 1.0f};
    float clearDepth = 1.0f;

    // projection
    float fovYRadians = 30.0f / 180.0f * 3.14f;
    std::array<float, 16> projection;

    // view
    int viewportX =0;
    int viewportY =0;
    int viewportWidth = 1;
    int viewportHeight = 1;
    std::array<float, 16> view;
    std::array<float, 16> viewInverse;

    // mult
    std::array<float, 16> viewProjection;
    void CalcViewProjection()
    {
        viewProjection = Mult(view, projection);
    }

    std::array<float, 16> CalcModelViewProjection(const std::array<float, 16> &m) const
    {
        return Mult(m, viewProjection);
    }
};
/*
struct RenderTargetInfo
{
public:
    uint32_t CameraID = 0;
    dxm::Matrix Projection = dxm::Matrix::Identity;
    dxm::Matrix View = dxm::Matrix::Identity;
    dxm::Matrix View2 = dxm::Matrix::Identity;
    DirectX::XMINT4 Viewport = {0, 0, 100, 100};
    dxm::Vec4 ClearColor = dxm::Vec4::Zero;
    float ClearDepth = 1.0f;

    const dxm::Matrix &CalcMvp(const dxm::Matrix &model) const
    {
        // glm::mat4 mvp = projection * view * model;
        auto m = model.Load();
        auto v = View.Load();
        auto p = Projection.Load();
        m_mvp = DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(m, v), p);
        return m_mvp;
    }

private:
    mutable dxm::Matrix m_mvp = dxm::Matrix::Identity;
};
*/

} // namespace camera