#pragma once
#include "array_math.h"

namespace camera
{

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
        viewProjection = amth::Mult(view, projection);
    }

    std::array<float, 16> CalcModelViewProjection(const std::array<float, 16> &m) const
    {
        return amth::Mult(m, viewProjection);
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