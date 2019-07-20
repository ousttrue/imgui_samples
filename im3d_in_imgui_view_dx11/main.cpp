#include "win32_window.h"
#include "dx11_context.h"
#include "dx11_renderer.h"
#include "orbit_camera.h"

#include "im3d_impl.h"
#include "im3d_impl_dx11.h"
#include <im3d.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h> // d3d11.h

#include <plog/Log.h>
#include <plog/Appenders/DebugOutputAppender.h>

int main(int argc, char **argv)
{
    static plog::DebugOutputAppender<plog::TxtFormatter> debugOutputAppender;
    plog::init(plog::verbose, &debugOutputAppender);

    Win32Window window;
    auto hwnd = window.Create(640, 480, L"im3d_in_imgui_view");
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
    auto deviceContext = dx11.GetDeviceContext();

    DX11Renderer renderer;
    Im3dImplDx11 im3dImplDx11;

    auto world = amth::IdentityMatrix();
    OrbitCamera camera;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    // Setup Platform/Renderer bindings
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init((ID3D11Device *)device, (ID3D11DeviceContext *)deviceContext);
    bool show_demo_window = true;
    std::array<float, 4> view_color = {0.60f, 0.45f, 0.55f, 1.00f};

    while (window.IsRunning())
    {
        // get window state and mouse input
        auto &windowState = window.GetState();

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        io.MouseDown[0] = (int)windowState.Mouse.IsDown(ButtonFlags::Left);
        io.MouseDown[1] = (int)windowState.Mouse.IsDown(ButtonFlags::Right);
        io.MouseDown[2] = (int)windowState.Mouse.IsDown(ButtonFlags::Middle);
        io.MouseWheel = (float)windowState.Mouse.Wheel;
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
        {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        if (ImGui::Begin("render target", nullptr,
                         ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {
            // auto size = ImGui::GetWindowSize();
            auto size = ImGui::GetContentRegionAvail();
            {
                auto pos = ImGui::GetMousePos();
                WindowState viewState{
                    .Width = (int)size.x,
                    .Height = (int)size.y,
                    .ElapsedSeconds = windowState.ElapsedSeconds,
                    .DeltaSeconds = windowState.DeltaSeconds,
                    .Mouse = {
                        .X = (int)pos.x,
                        .Y = (int)pos.y,
                        .Wheel = windowState.Mouse.Wheel,
                        .Buttons = windowState.Mouse.Buttons}};
                // update view camera
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
                auto renderTarget = renderer.NewFrameToRenderTarget(deviceContext, viewState.Width, viewState.Height, view_color.data());

                // use manipulated world
                renderer.DrawTeapot(deviceContext, camera.state.viewProjection.data(), world.data());
                // draw gizmo
                im3dImplDx11.Draw(deviceContext, camera.state.viewProjection.data());

                ImGui::ImageButton((ImTextureID)renderTarget, size, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), 0);
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();

        {
            // render to backbuffer

            dx11.NewFrame(windowState.Width, windowState.Height);

            // imgui Rendering
            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            // transfer backbuffer
            dx11.Present();
        }
    }

    return 0;
}
