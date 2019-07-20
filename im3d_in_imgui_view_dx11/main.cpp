#include "win32_window.h"
#include "dx11_context.h"
#include "dx11_renderer.h"
#include "orbit_camera.h"
#include "im3d_impl.h"
#include "im3d_impl_dx11.h"
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
    Im3dImplDx11 im3dImplDx11;

    auto world = amth::IdentityMatrix();
    OrbitCamera camera;

    while (window.IsRunning())
    {
        // get window state and mouse input
        auto &windowState = window.GetState();
        // camera update
        camera.WindowInput(windowState);

        //
        // gizmo update
        //
        Im3d_Impl_NewFrame(&camera.state, &windowState);
        // process gizmo, not draw, build draw list.
        Im3d::Gizmo("GizmoUnified", world.data());
        Im3d::EndFrame();

        //
        // render
        //
        // setViewPort & clear background
        auto deviceContext = dx11.NewFrame(windowState.Width, windowState.Height); 
        // use manipulated world
        renderer.DrawTeapot(deviceContext, camera.state.viewProjection.data(), world.data());
        // draw gizmo
        im3dImplDx11.Draw(deviceContext, camera.state.viewProjection.data());
        // transfer backbuffer
        dx11.Present();
    }

    return 0;
}
