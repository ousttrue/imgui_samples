#include <Win32Window.h>
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

    screenstate::Win32Window window(L"im3d_minimum_dx11 class");
    auto hwnd = window.Create(L"im3d_minimum_dx11");
    if (!hwnd)
    {
        return 1;
    }
    window.Show();

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

    // window state and mouse input
    screenstate::ScreenState state;
    while (window.Update(&state))
    {
        // camera update
        camera.WindowInput(state);

        //
        // gizmo update
        //
        Im3d_Impl_NewFrame(&camera.state, &state);
        // process gizmo, not draw, build draw list.
        Im3d::GizmoTranslation("GizmoUnified", world.data()+12);
        Im3d::EndFrame();

        //
        // render
        //
        // setViewPort & clear background
        auto deviceContext = dx11.NewFrame(state.Width, state.Height); 
        // use manipulated world
        renderer.DrawTeapot(deviceContext, camera.state.viewProjection.data(), world.data());
        // draw gizmo
        im3dImplDx11.Draw(deviceContext, camera.state.viewProjection.data());
        // transfer backbuffer
        dx11.Present();
    }

    return 0;
}
