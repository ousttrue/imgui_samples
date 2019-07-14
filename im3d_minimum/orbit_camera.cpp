#include "orbit_camera.h"
#define _USE_MATH_DEFINES
#include <math.h>

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

void OrbitCamera::SetScreenSize(int w, int h)
{
    if (w == screenWidth && h == screenHeight)
    {
        return;
    }
    if (h == 0)
    {
        return;
    }
    screenWidth = w;
    screenHeight = h;
    aspectRatio = w / (float)h;
    CalcPerspective();
}
