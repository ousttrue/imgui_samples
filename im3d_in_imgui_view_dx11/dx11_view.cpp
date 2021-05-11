#include <ScreenState.h>

#include "dx11_view.h"
#include "dx11_renderer.h"
#include "orbit_camera.h"

#include "im3d_impl.h"
#include "im3d_impl_dx11.h"
#include <im3d.h>

class DX11ViewImpl
{
    OrbitCamera camera;
    std::array<float, 16> world = amth::IdentityMatrix();
    std::array<float, 4> view_color = {0.60f, 0.45f, 0.55f, 1.00f};
    DX11Renderer renderer;
    Im3dImplDx11 im3dImplDx11;

public:
    void *Draw(void *deviceContext, const screenstate::ScreenState &viewState)
    {
        camera.WindowInput(viewState);

        //
        // gizmo update
        //
        Im3d_Impl_NewFrame(&camera.state, &viewState);
        // process gizmo, not draw, build draw list.
        Im3d::Gizmo("GizmoUnified", world.data());
        Im3d::EndFrame();

        //
        // render to viewport
        //
        // setViewPort & clear background
        auto renderTarget = renderer.NewFrameToRenderTarget(deviceContext,
                                                            viewState.Width, viewState.Height, view_color.data());

        // use manipulated world
        renderer.DrawTeapot(deviceContext, camera.state.viewProjection.data(), world.data());
        // draw gizmo
        im3dImplDx11.Draw(deviceContext, camera.state.viewProjection.data());

        return renderTarget;
    }
};

DX11View::DX11View()
    : m_impl(new DX11ViewImpl)
{
}

DX11View::~DX11View()
{
    delete m_impl;
}

void *DX11View::Draw(void *deviceContext, const struct screenstate::ScreenState &viewState)
{
    return m_impl->Draw(deviceContext, viewState);
}
