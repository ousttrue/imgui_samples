#include "win32_window.h"
#include "dx11_context.h"
#include "dx11_renderer.h"
#include "orbit_camera.h"
#include "im3d_gui.h"

int main(int argc, char **argv)
{
    Win32Window window;
    auto hwnd = window.Create(640, 480, L"im3d_minimum_dx11");
    if (!hwnd)
    {
        return 1;
    }

    DX11Context dx11;
    auto device = dx11.Create(hwnd);
    if (!device)
    {
        return 2;
    }

    DX11Renderer renderer;
    if(renderer.Create(device)){
        return 3;
    }

    auto world = amth::IdentityMatrix();

    OrbitCamera camera;

    Im3dGui gizmo;
    if (!gizmo.Initialize())
    {
        return 4;
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
        gizmo.Manipulate(world.data());                                        // process gizmo, not draw, build draw list.
        renderer.DrawTeapot(camera.state.viewProjection.data(), world.data()); // use manipulated world
        gizmo.Draw(camera.state.viewProjection.data());                        // draw gizmo

        // transfer backbuffer
        dx11.Present();
    }

    return 0;
}
