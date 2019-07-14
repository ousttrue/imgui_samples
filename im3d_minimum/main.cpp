#include "win32_window.h"
#include "wgl_context.h"
#include "gl3_renderer.h"
#include "orbit_camera.h"
#include "im3d_gui.h"

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

    Im3dGui gizmo;

    float lastTime = 0;

    while (window.IsRunning())
    {
        // camera update
        int w, h;
        std::tie(w, h) = window.GetSize();
        camera.SetScreenSize((float)w, (float)h);
        auto mouse = window.GetMouseState();
        camera.MouseInput(mouse);
        camera.state.CalcViewProjection();
        auto time = window.GetTimeSeconds();
        auto deltaTime = 0.0016f;
        if (lastTime > 0)
        {
            deltaTime = time - lastTime;
        }
        lastTime = time;

        // render
        renderer.NewFrame(w, h); // setViewPort & clear background
        gizmo.NewFrame(&camera.state, &mouse, deltaTime);
        gizmo.Manipulate(world);
        renderer.DrawTeapot(camera.state.viewProjection.data(), world); // use manipulated world
        gizmo.Draw(camera.state.viewProjection.data(), w, h);

        // transfer backbuffer
        wgl.Present();
    }

    return 0;
}
