#include "win32_window.h"
#include <Windows.h>
#include <windowsx.h> // GET_X_LPARAM macros
#include <assert.h>

static LRESULT CALLBACK WindowProc(HWND _hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
class Win32WindowImpl *g_window = nullptr;

static ATOM GetOrRegisterWindowClass(HINSTANCE hInstance, const TCHAR *className)
{
    static ATOM wndclassex = 0;
    if (wndclassex == 0)
    {
        WNDCLASSEX wc = {0};
        wc.cbSize = sizeof(wc);
        wc.style = CS_OWNDC; // | CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = className;
        wc.hCursor = LoadCursor(0, IDC_ARROW);
        wndclassex = RegisterClassEx(&wc);
    }
    return wndclassex;
}

class Win32WindowImpl
{
    HWND m_hwnd = NULL;
    int m_width = 0;
    int m_height = 0;
    MouseState m_mouseState = {0};

public:
    Win32WindowImpl(HWND hWnd)
        : m_hwnd(hWnd)
    {
        assert(g_window == nullptr);
        g_window = this;

        RECT rect;
        GetClientRect(hWnd, &rect);
        m_width = rect.right - rect.left;
        m_height = rect.bottom - rect.top;
    }
    ~Win32WindowImpl()
    {
        if (m_hwnd)
        {
            DestroyWindow(m_hwnd);
        }
    }
    HWND GetHandle() const { return m_hwnd; }
    void Resize(int w, int h)
    {
        m_width = w;
        m_height = h;
    }
    bool IsRunning()
    {
        MSG msg;
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) && msg.message != WM_QUIT)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (msg.message == WM_QUIT)
        {
            return false;
        }
        return true;
    }
    std::tuple<int, int> GetSize() const
    {
        return std::make_tuple(m_width, m_height);
    }
    const MouseState &GetMouseState() const
    {
        return m_mouseState;
    }
    MouseState &GetMouseState()
    {
        return m_mouseState;
    }
};

static LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SIZE:
    {
        auto w = (int)LOWORD(lParam);
        auto h = (int)HIWORD(lParam);
        g_window->Resize(w, h);
        break;
    }

    case WM_PAINT:
        ValidateRect(hWnd, 0);
        break;

    case WM_CLOSE:
        PostQuitMessage(0);
        break;

    case WM_MOUSEMOVE:
    {
        auto &mouse = g_window->GetMouseState();
        mouse.X = GET_X_LPARAM(lParam);
        mouse.Y = GET_Y_LPARAM(lParam);
        return 0;
    }

    case WM_LBUTTONDOWN:
    {
        auto &mouse = g_window->GetMouseState();
        mouse.Down(ButtonFlags::Left);
        return 0;
    }

    case WM_LBUTTONUP:
    {
        auto &mouse = g_window->GetMouseState();
        mouse.Up(ButtonFlags::Left);
        return 0;
    }

    case WM_MBUTTONDOWN:
    {
        auto &mouse = g_window->GetMouseState();
        mouse.Down(ButtonFlags::Middle);
        return 0;
    }

    case WM_MBUTTONUP:
    {
        auto &mouse = g_window->GetMouseState();
        mouse.Up(ButtonFlags::Middle);
        return 0;
    }

    case WM_RBUTTONDOWN:
    {
        auto &mouse = g_window->GetMouseState();
        mouse.Down(ButtonFlags::Right);
        return 0;
    }

    case WM_RBUTTONUP:
    {
        auto &mouse = g_window->GetMouseState();
        mouse.Up(ButtonFlags::Right);
        return 0;
    }

    case WM_MOUSEWHEEL:
    {
        auto &mouse = g_window->GetMouseState();
        mouse.Wheel = GET_WHEEL_DELTA_WPARAM(wParam);
        return 0;
    }

    default:
        break;
    };
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

Win32Window::Win32Window()
{
}

Win32Window::~Win32Window()
{
    if (m_impl)
    {
        delete m_impl;
        m_impl = nullptr;
    }
}

bool Win32Window::Create(int w, int h, const wchar_t *title)
{
    auto hInstance = GetModuleHandle(0);
    auto wndClass = GetOrRegisterWindowClass(hInstance, L"Win32WindowImpl");
    auto hWnd = CreateWindowEx(
        WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
        MAKEINTATOM(wndClass),
        title,
        WS_OVERLAPPEDWINDOW | WS_MINIMIZEBOX | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        CW_USEDEFAULT, CW_USEDEFAULT,
        w, h,
        nullptr,
        nullptr,
        hInstance,
        this);
    if (hWnd == NULL)
    {
        return false;
    }
    m_impl = new Win32WindowImpl(hWnd);
    ShowWindow(hWnd, SW_SHOW);
    return true;
}

bool Win32Window::IsRunning()
{
    return m_impl->IsRunning();
}

void *Win32Window::GetHandle() const
{
    return m_impl->GetHandle();
}

std::tuple<int, int> Win32Window::GetSize() const
{
    return m_impl->GetSize();
}

bool Win32Window::HasFocus() const
{
    return m_impl->GetHandle() == GetFocus();
}

MouseState Win32Window::GetMouseState() const
{
    auto state = m_impl->GetMouseState();
    m_impl->GetMouseState().Wheel = 0; // clear
    return state;
}

float Win32Window::GetTimeSeconds() const
{
    return timeGetTime() * 0.001f;
}
