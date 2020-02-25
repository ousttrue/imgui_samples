#pragma once
#include "Win32Window.h"
#include <windowsx.h>

struct Gwlp
{
    // last param of CreateWindow to GWLP
    static void Set(HWND hWnd, LPARAM lParam)
    {
        auto pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
    }

    template <class T>
    static T *Get(HWND hWnd)
    {
        return reinterpret_cast<T *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }
};

namespace screenstate
{

Win32Window::Win32Window(const wchar_t *className)
    : m_className(className), m_hInstance(GetModuleHandle(NULL))
{
}

Win32Window::~Win32Window()
{
    ::UnregisterClassW(m_className.c_str(), m_hInstance);
}

LRESULT CALLBACK Win32Window::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto window = Gwlp::Get<Win32Window>(hWnd);

    switch (message)
    {
    case WM_CREATE:
        Gwlp::Set(hWnd, lParam);
        return 0;

    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            window->m_state.Width = LOWORD(lParam);
            window->m_state.Height = HIWORD(lParam);
        }
        break;

        // TODO:
        // case WM_KEYDOWN:
        //     if (pSample)
        //     {
        //         pSample->OnKeyDown(static_cast<UINT8>(wParam));
        //     }
        //     return 0;

        // case WM_KEYUP:
        //     if (pSample)
        //     {
        //         pSample->OnKeyUp(static_cast<UINT8>(wParam));
        //     }
        //     return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_MOUSEMOVE:
        window->m_state.MouseX = GET_X_LPARAM(lParam);
        window->m_state.MouseY = GET_Y_LPARAM(lParam);
        return 0;

    case WM_LBUTTONDOWN:
        if (!window->m_state.HasCapture())
        {
            SetCapture(hWnd);
        }
        window->m_state.Set(MouseButtonFlags::LeftDown);
        return 0;

    case WM_LBUTTONUP:
        window->m_state.Unset(MouseButtonFlags::LeftDown);
        if (!window->m_state.HasCapture())
        {
            ReleaseCapture();
        }
        return 0;

    case WM_RBUTTONDOWN:
        if (!window->m_state.HasCapture())
        {
            SetCapture(hWnd);
        }
        window->m_state.Set(MouseButtonFlags::RightDown);
        return 0;

    case WM_RBUTTONUP:
        window->m_state.Unset(MouseButtonFlags::RightDown);
        if (!window->m_state.HasCapture())
        {
            ReleaseCapture();
        }
        return 0;

    case WM_MBUTTONDOWN:
        if (!window->m_state.HasCapture())
        {
            SetCapture(hWnd);
        }
        window->m_state.Set(MouseButtonFlags::MiddleDown);
        return 0;

    case WM_MBUTTONUP:
        window->m_state.Unset(MouseButtonFlags::MiddleDown);
        if (!window->m_state.HasCapture())
        {
            ReleaseCapture();
        }
        return 0;

    case WM_MOUSEWHEEL:
    {
        auto zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        if (zDelta < 0)
        {
            window->m_state.Set(MouseButtonFlags::WheelMinus);
        }
        else if (zDelta > 0)
        {
            window->m_state.Set(MouseButtonFlags::WheelPlus);
        }
        return 0;
    }

    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;

    case WM_SETCURSOR:
        if (!window->m_enableSetCursor)
        {
            if (LOWORD(lParam) == HTCLIENT)
            {
                window->m_state.Set(MouseButtonFlags::CursorUpdate);
                return 1;
            }
        }
        break;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}

HWND Win32Window::Create(const wchar_t *titleName, int width, int height)
{
    // Initialize the window class.
    WNDCLASSEXW windowClass = {
        .cbSize = (UINT)sizeof(WNDCLASSEXW),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = WindowProc,
        .hInstance = m_hInstance,
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .lpszClassName = m_className.c_str(),
    };
    if (!RegisterClassExW(&windowClass))
    {
        return NULL;
    }

    if (width && height)
    {
        RECT windowRect = {0, 0, width, height};
        AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
        width = windowRect.right - windowRect.left;
        height = windowRect.bottom - windowRect.top;
    }
    else
    {
        width = CW_USEDEFAULT;
        height = CW_USEDEFAULT;
    }

    // Create the window and store a handle to it.
    m_hwnd = CreateWindowW(
        m_className.c_str(),
        titleName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width,
        height,
        nullptr, // We have no parent window.
        nullptr, // We aren't using menus.
        m_hInstance,
        this);

    return m_hwnd;
}

void Win32Window::Show(int nCmdShow)
{
    ShowWindow(m_hwnd, nCmdShow);
}

bool Win32Window::Update(ScreenState *pState)
{
    MSG msg = {};
    while (true)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                return false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            break;
        }
    }

    if (msg.message == WM_QUIT)
    {
        return false;
    }

    auto now = timeGetTime();
    if (m_startTime)
    {
        now -= m_startTime;
    }
    else
    {
        now = 0;
    }
    if (now > m_lastTime)
    {
        m_state.ElapsedSeconds = now * 0.001f;
        auto delta = now - m_lastTime;
        m_state.DeltaSeconds = delta * 0.001f;
    }
    else
    {
        // work around
        m_startTime = now;
        m_state.DeltaSeconds = 0.016f;
        m_state.ElapsedSeconds = 0.016f;
    }
    m_lastTime = now;

    *pState = m_state;
    m_state.Clear();
    return true;
}
} // namespace screenstate
