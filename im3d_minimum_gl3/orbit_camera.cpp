#include "orbit_camera.h"
#define _USE_MATH_DEFINES
#include <math.h>

void OrbitCamera::CalcView()
{
    auto ys = (float)sin(yawRadians);
    auto yc = (float)cos(yawRadians);
    std::array<float, 16> yaw = {
        yc,
        0,
        ys,
        0,

        0,
        1,
        0,
        0,

        -ys,
        0,
        yc,
        0,

        0,
        0,
        0,
        1,
    };

    auto ps = (float)sin(pitchRadians);
    auto pc = (float)cos(pitchRadians);
    std::array<float, 16> pitch = {
        1,
        0,
        0,
        0,

        0,
        pc,
        ps,
        0,

        0,
        -ps,
        pc,
        0,

        0,
        0,
        0,
        1,
    };

    std::array<float, 16> t = {
        1,
        0,
        0,
        0,
        0,
        1,
        0,
        0,
        0,
        0,
        1,
        0,
        -shiftX,
        -shiftY,
        -shiftZ,
        1,
    };

    auto yawPitch = camera::Mult(yaw, pitch);
    state.view = camera::Mult(yawPitch, t);

    t[12] *= -1;
    t[13] *= -1;
    t[14] *= -1;
    camera::Transpose(yawPitch);
    state.viewInverse = camera::Mult(t, yawPitch);
}

void OrbitCamera::CalcPerspective()
{
    const float f = static_cast<float>(1.0f / tan(state.fovYRadians / 2.0));

    state.projection[0] = f / aspectRatio;
    state.projection[1] = 0.0f;
    state.projection[2] = 0.0f;
    state.projection[3] = 0.0f;

    state.projection[4] = 0.0f;
    state.projection[5] = f;
    state.projection[6] = 0.0f;
    state.projection[7] = 0.0f;

    state.projection[8] = 0.0f;
    state.projection[9] = 0.0f;
    state.projection[10] = (zNear + zFar) / (zNear - zFar);
    state.projection[11] = -1;

    state.projection[12] = 0.0f;
    state.projection[13] = 0.0f;
    state.projection[14] = (2 * zFar * zNear) / (zNear - zFar);
    state.projection[15] = 0.0f;
}

void OrbitCamera::SetScreenSize(float w, float h)
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
    state.viewportWidth = w;
    state.viewportHeight = h;
    CalcPerspective();
}

void OrbitCamera::MouseInput(const MouseState &mouse)
{
    if (prevMouseX != -1 && prevMouseY != -1)
    {
        auto deltaX = mouse.X - prevMouseX;
        auto deltaY = mouse.Y - prevMouseY;

        if (mouse.IsDown(ButtonFlags::Right))
        {
            const auto FACTOR = 1.0f / 180.0f * 1.7f;
            yawRadians -= deltaX * FACTOR;
            pitchRadians += deltaY * FACTOR;
        }
        if (mouse.IsDown(ButtonFlags::Middle))
        {
            shiftX -= deltaX / (float)state.viewportHeight * shiftZ;
            shiftY += deltaY / (float)state.viewportHeight * shiftZ;
        }
        if (mouse.Wheel > 0)
        {
            shiftZ *= 0.9f;
        }
        else if (mouse.Wheel < 0)
        {
            shiftZ *= 1.1f;
        }
    }
    prevMouseX = mouse.X;
    prevMouseY = mouse.Y;
    CalcView();
}
