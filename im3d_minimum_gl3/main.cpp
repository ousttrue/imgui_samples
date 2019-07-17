#include "win32_window.h"
#include "wgl_context.h"
#include "gl3_renderer.h"
#include "orbit_camera.h"
#include "im3d_impl.h"
#include "im3d_impl_gl3.h"
#include <im3d.h>
#include <plog/Log.h>
#include <plog/Appenders/DebugOutputAppender.h>

int main(int argc, char **argv)
{
    static plog::DebugOutputAppender<plog::TxtFormatter> debugOutputAppender;
    plog::init(plog::verbose, &debugOutputAppender);

    Win32Window window;
    auto hwnd = window.Create(640, 480, L"im3d_minimum_gl3");
    if (!hwnd)
    {
        return 1;
    }

    WGLContext wgl;
    if (!wgl.Create(hwnd, 3, 0))
    {
        return 2;
    }

    GL3Renderer renderer;
    Im3dImplGL3 im3dImplGL3("#version 140");

    auto world = amth::IdentityMatrix();

    OrbitCamera camera;

    while (window.IsRunning())
    {
        // camera update
        auto &windowState = window.GetState();
        camera.WindowInput(windowState);

        Im3d_Impl_NewFrame(&camera.state, &windowState);
        // process gizmo, not draw, build draw list.
        Im3d::Gizmo("GizmoUnified", world.data());
        Im3d::EndFrame();

        // render
        renderer.NewFrame(windowState.Width, windowState.Height);              // setViewPort & clear background
        renderer.DrawTeapot(camera.state.viewProjection.data(), world.data()); // use manipulated world
        im3dImplGL3.Draw(camera.state.viewProjection.data());

        // transfer backbuffer
        wgl.Present();
    }

    return 0;
}
