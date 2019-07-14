#include "win32_window.h"
#include "wgl_context.h"
#include "gl3_renderer.h"
#include "orbit_camera.h"

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
        0,
        1,
    };

    OrbitCamera camera;

    while (window.IsRunning())
    {
        // camera update
        int w, h;
        std::tie(w, h) = window.GetSize();
        camera.SetScreenSize(w, h);
        auto mouse = window.GetMouseState();
        camera.MouseInput(mouse);
        camera.CalcViewProjection();

        // render
        renderer.NewFrame(w, h); // setViewPort & clear background
        renderer.DrawTeapot(camera.viewProjection.data(), world);

        // transfer backbuffer
        wgl.Present();
    }

    return 0;
}
