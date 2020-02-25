#include "orbit_camera.h"
#define _USE_MATH_DEFINES
#include <math.h>

void OrbitCamera::CalcView()
{
    auto yaw = amth::YawMatrix(yawRadians);
    auto pitch = amth::PitchMatrix(pitchRadians);
    auto yawPitch = amth::Mult(yaw, pitch);
    auto t = amth::TranslationMatrix(-shiftX, -shiftY, -shiftZ);
    state.view = amth::Mult(yawPitch, t);

    t[12] *= -1;
    t[13] *= -1;
    t[14] *= -1;
    amth::Transpose(yawPitch);
    state.viewInverse = amth::Mult(t, yawPitch);
}

void OrbitCamera::CalcPerspective()
{
#if 0
    amth::PerspectiveRHGL(state.projection.data(), state.fovYRadians, aspectRatio, zNear, zFar);
#else
    amth::PerspectiveRHDX(state.projection.data(), state.fovYRadians, aspectRatio, zNear, zFar);
#endif
}

void OrbitCamera::SetViewport(int x, int y, int w, int h)
{
    if (w == state.viewportWidth && h == state.viewportHeight)
    {
        return;
    }
    if (h == 0 || w == 0)
    {
        aspectRatio = 1.0f;
    }
    else
    {
        aspectRatio = w / (float)h;
    }
    state.viewportX = x;
    state.viewportY = y;
    state.viewportWidth = w;
    state.viewportHeight = h;
    CalcPerspective();
}

void OrbitCamera::WindowInput(const screenstate::ScreenState &window)
{
    SetViewport(0, 0, window.Width, window.Height);

    if (prevMouseX != -1 && prevMouseY != -1)
    {
        auto deltaX = window.MouseX - prevMouseX;
        auto deltaY = window.MouseY - prevMouseY;

        if (window.Has(screenstate::MouseButtonFlags::RightDown))
        {
            const auto FACTOR = 1.0f / 180.0f * 1.7f;
            yawRadians -= deltaX * FACTOR;
            pitchRadians += deltaY * FACTOR;
        }
        if (window.Has(screenstate::MouseButtonFlags::MiddleDown))
        {
            shiftX -= deltaX / (float)state.viewportHeight * shiftZ;
            shiftY += deltaY / (float)state.viewportHeight * shiftZ;
        }
        if (window.Has(screenstate::WheelPlus))
        {
            shiftZ *= 0.9f;
        }
        else if (window.Has(screenstate::WheelMinus))
        {
            shiftZ *= 1.1f;
        }
    }
    prevMouseX = window.MouseX;
    prevMouseY = window.MouseY;
    CalcView();
    state.CalcViewProjection();
}
