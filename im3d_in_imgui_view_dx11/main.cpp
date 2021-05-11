#include <Win32Window.h>
#include "dx11_context.h"
#include "dx11_view.h"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h> // d3d11.h

#include <plog/Log.h>
#include <plog/Appenders/DebugOutputAppender.h>

int main(int argc, char **argv)
{
    static plog::DebugOutputAppender<plog::TxtFormatter> debugOutputAppender;
    plog::init(plog::verbose, &debugOutputAppender);

    screenstate::Win32Window window(L"CLASS_NAME");
    auto hwnd = window.Create(L"im3d_in_imgui_view", 640, 480);
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
    auto deviceContext = dx11.GetDeviceContext();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    // Setup Platform/Renderer bindings
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init((ID3D11Device *)device, (ID3D11DeviceContext *)deviceContext);
    bool show_demo_window = true;

    DX11View view;

    screenstate::ScreenState windowState;
    while (window.Update(&windowState))
    {
        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        io.MouseDown[0] = windowState.Has(screenstate::MouseButtonFlags::LeftDown);
        io.MouseDown[1] = windowState.Has(screenstate::MouseButtonFlags::RightDown);
        io.MouseDown[2] = windowState.Has(screenstate::MouseButtonFlags::MiddleDown);
        if (windowState.Has(screenstate::MouseButtonFlags::WheelMinus))
        {
            io.MouseWheel = -100;
        }
        else if (windowState.Has(screenstate::MouseButtonFlags::WheelPlus))
        {
            io.MouseWheel = 100;
        }
        ImGui::NewFrame();

        ////////////////////////////////////////////////////////////

        {
            // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
            if (show_demo_window)
            {
                ImGui::ShowDemoWindow(&show_demo_window);
            }

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            if (ImGui::Begin("render target", nullptr,
                             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
            {
                auto size = ImGui::GetContentRegionAvail();
                auto pos = ImGui::GetWindowPos();
                auto frameHeight = ImGui::GetFrameHeight();

                // update view camera
                auto viewState = windowState.Crop(pos.x, pos.y + frameHeight, size.x, size.y);
                auto renderTarget = view.Draw(deviceContext, viewState);
                ImGui::ImageButton((ImTextureID)renderTarget, size, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), 0);
            }
            ImGui::End();
            ImGui::PopStyleVar();
        }

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

    ImGui_ImplDX11_Shutdown();

    return 0;
}
