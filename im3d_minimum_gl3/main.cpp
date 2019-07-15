#include "win32_window.h"
#include "wgl_context.h"
#include "gl3_renderer.h"
#include "orbit_camera.h"
#include "im3d_gui.h"

int main(int argc, char **argv)
{
    Win32Window window;
    if (!window.Create(640, 480, L"im3d_minimum_gl3"))
    {
        return 1;
    }
    auto hwnd = window.GetState().Handle;

    WGLContext wgl;
    if (!wgl.Create(hwnd, 3, 0))
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
    if(!gizmo.Initialize()){
        return 3;
    }

    float lastTime = 0;

    while (window.IsRunning())
    {
        // camera update
        auto &state = window.GetState();
        camera.SetViewport(0, 0, state.Width, state.Height);
        camera.MouseInput(state.Mouse);
        camera.state.CalcViewProjection();
        auto time = window.GetTimeSeconds();
        auto deltaTime = 0.0016f;
        if (lastTime > 0)
        {
            deltaTime = time - lastTime;
        }
        lastTime = time;

        // render
        renderer.NewFrame(state.Width, state.Height); // setViewPort & clear background
        gizmo.NewFrame(&camera.state, &state.Mouse, deltaTime);
        gizmo.Manipulate(world); // process gizmo, not draw, build draw list.
        renderer.DrawTeapot(camera.state.viewProjection.data(), world); // use manipulated world
        gizmo.Draw(camera.state.viewProjection.data()); // draw gizmo

        // transfer backbuffer
        wgl.Present();
    }

    return 0;
}
