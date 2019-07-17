#include "win32_window.h"
#include "dx11_context.h"
#include "dx11_renderer.h"
#include "dx11_im3d.h"
#include "orbit_camera.h"
#include "im3d_gui.h"
#include <im3d.h>
#include <plog/Log.h>
#include <plog/Appenders/DebugOutputAppender.h>

int main(int argc, char **argv)
{
    static plog::DebugOutputAppender<plog::TxtFormatter> debugOutputAppender;
    plog::init(plog::verbose, &debugOutputAppender);

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

    DX11Im3d dx11im3d;

    auto world = amth::IdentityMatrix();

    OrbitCamera camera;

    while (window.IsRunning())
    {
        // camera update
        auto &state = window.GetState();
        camera.SetViewport(0, 0, state.Width, state.Height);
        camera.MouseInput(state.Mouse);
        camera.state.CalcViewProjection();

        // render
        auto deviceContext = dx11.NewFrame(state.Width, state.Height); // setViewPort & clear background
        Im3d_Impl_NewFrame(&camera.state, &state);

        // process gizmo, not draw, build draw list.
        Im3d::Gizmo("GizmoUnified", world.data());

        // use manipulated world
        renderer.DrawTeapot(deviceContext, camera.state.viewProjection.data(), world.data());

        Im3d::EndFrame();

        // draw gizmo
        dx11im3d.Draw(deviceContext, camera.state.viewProjection.data());

        // transfer backbuffer
        dx11.Present();
    }

    return 0;
}
