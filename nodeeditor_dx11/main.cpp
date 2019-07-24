#include "win32_window.h"
#include "dx11_context.h"
#include "dx11_renderer.h"
#include "orbit_camera.h"
#include <plog/Log.h>
#include <plog/Appenders/DebugOutputAppender.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h> // d3d11.h

#include "spacechase0/sample.h"
#include "ChemiaAion/sample.h"
#include "imgui_node_graph_test.h"

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
    auto deviceContext = dx11.GetDeviceContext();

    DX11Renderer renderer;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    // Setup Platform/Renderer bindings
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init((ID3D11Device *)device, (ID3D11DeviceContext *)deviceContext);

    auto world = amth::IdentityMatrix();
    OrbitCamera camera;

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

        // camera update
        camera.WindowInput(windowState);


        NodeEditor();
        ChemiaAion::NodeEditor();
        static bool showNodeGraph = true;
        ShowExampleAppCustomNodeGraph(&showNodeGraph);

        //
        // render
        //
        // setViewPort & clear background
        auto deviceContext = dx11.NewFrame(windowState.Width, windowState.Height);

        // use manipulated world
        renderer.DrawTeapot(deviceContext, camera.state.viewProjection.data(), world.data());

        // imgui Rendering
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // transfer backbuffer
        dx11.Present();
    }

    return 0;
}
