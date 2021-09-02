#include "win32window.h"
#include "wgl_context.h"
#include "gl3_renderer.h"
#include "orbit_camera.h"
#include "im3d_impl.h"
#include "im3d_impl_gl3.h"
#include <im3d.h>
#include <plog/Log.h>
#include <plog/Appenders/DebugOutputAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Init.h>

int main(int argc, char **argv)
{
    static plog::DebugOutputAppender<plog::TxtFormatter> debugOutputAppender;
    plog::init(plog::verbose, &debugOutputAppender);

    screenstate::Win32Window window(L"CLASS_NAME");
    auto hwnd = window.Create(L"im3d_minimum_gl3", 640, 480);
    if (!hwnd)
    {
        return 1;
    }
    window.Show();

    WGLContext wgl;
    if (!wgl.Create(hwnd, 3, 0))
    {
        return 2;
    }

    GL3Renderer renderer;
    Im3dImplGL3 im3dImplGL3("#version 140");

    auto world = amth::IdentityMatrix();

    OrbitCamera camera;

    screenstate::ScreenState state;
    while (window.Update(&state))
    {
        // camera update
        camera.WindowInput(state);

        Im3d_Impl_NewFrame(&camera.state, &state);
        // process gizmo, not draw, build draw list.
        Im3d::Gizmo("GizmoUnified", world.data());
        Im3d::EndFrame();

        // render
        renderer.NewFrame(state.Width, state.Height);              // setViewPort & clear background
        renderer.DrawTeapot(camera.state.viewProjection.data(), world.data()); // use manipulated world
        im3dImplGL3.Draw(camera.state.viewProjection.data());

        // transfer backbuffer
        wgl.Present();
    }

    return 0;
}
