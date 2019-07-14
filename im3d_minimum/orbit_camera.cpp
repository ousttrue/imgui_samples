#include "orbit_camera.h"
#define _USE_MATH_DEFINES
#include <math.h>

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

    auto yawPitch = Mult(yaw, pitch);
    view = Mult(yawPitch, t);
}

void OrbitCamera::CalcPerspective()
{
    auto rad = fovYDegrees / 180.0f * M_PI;
    const float f = static_cast<float>(1.0f / tan(rad / 2.0));

    projection[0] = f / aspectRatio;
    projection[1] = 0.0f;
    projection[2] = 0.0f;
    projection[3] = 0.0f;

    projection[4] = 0.0f;
    projection[5] = f;
    projection[6] = 0.0f;
    projection[7] = 0.0f;

    projection[8] = 0.0f;
    projection[9] = 0.0f;
    projection[10] = (zNear + zFar) / (zNear - zFar);
    projection[11] = -1;

    projection[12] = 0.0f;
    projection[13] = 0.0f;
    projection[14] = (2 * zFar * zNear) / (zNear - zFar);
    projection[15] = 0.0f;
}

void OrbitCamera::CalcViewProjection()
{
    viewProjection = Mult(view, projection);
}

void OrbitCamera::SetScreenSize(int w, int h)
{
    if (w == screenWidth && h == screenHeight)
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
    screenWidth = w;
    screenHeight = h;
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
            shiftX -= deltaX / (float)screenHeight * shiftZ;
            shiftY += deltaY / (float)screenHeight * shiftZ;
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
