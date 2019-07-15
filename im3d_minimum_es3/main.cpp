#include "win32_window.h"
#include "es3_context.h"
#include "gl3_renderer.h"
#include "orbit_camera.h"
#include "im3d_gui.h"

int main(int argc, char **argv)
{
    Win32Window window;
    if (!window.Create(640, 480, L"im3d_minimum_es3"))
    {
        return 1;
    }
    auto hwnd = window.GetState().Handle;

    ES3Context egl;
    if (!egl.Create(hwnd))
    {
        return 2;
    }

    GL3Renderer renderer;

    auto world = amth::IdentityMatrix();

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
        gizmo.Manipulate(world.data()); // process gizmo, not draw, build draw list.
        renderer.DrawTeapot(camera.state.viewProjection.data(), world.data()); // use manipulated world
        gizmo.Draw(camera.state.viewProjection.data()); // draw gizmo

        // transfer backbuffer
        egl.Present();
    }

    return 0;
}
