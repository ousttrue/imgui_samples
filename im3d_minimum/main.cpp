#include "win32_window.h"
#include "wgl_context.h"
#include "gl3_renderer.h"

int main(int argc, char **argv)
{
    Win32Window window;
    if (!window.Create(640, 480, L"im3d_minimum"))
    {
        return 1;
    }

    WGLContext wgl;
    if (!wgl.Create(window.GetHandle(), 3, 0))
    {
        return 2;
    }

    GL3Renderer renderer;

    float world[] = {
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

        0,
        0,
        -2,
        1,
    };

    const float ar = 1.0f;
    const float zNear = 0.1f;
    const float zFar = 10.0f;
    const double rad = 30.0f / 180.0f * M_PI;
    const float f = static_cast<float>(1.0f / tan(rad / 2.0));

    float projection[16];
    projection[0] = f / ar;
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

    while (window.IsRunning())
    {
        renderer.BeginFrame();

        renderer.DrawTeapot(projection, world);

        renderer.EndFrame();

        wgl.Present();
    }

    return 0;
}
