#include "win32_window.h"
#include <Windows.h>
#include <windowsx.h> // GET_X_LPARAM macros
#include "save_windowplacement.h"
#include <assert.h>
#include <plog/Log.h>

static LRESULT CALLBACK WindowProc(HWND _hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
class Win32WindowImpl *g_window = nullptr;

static ATOM GetOrRegisterWindowClass(HINSTANCE hInstance, const TCHAR *className)
{
    static ATOM wndclassex = 0;
    if (wndclassex == 0)
    {
        WNDCLASSEX wc = {0};
        wc.cbSize = sizeof(wc);
        // wc.style = CS_OWNDC; // | CS_HREDRAW | CS_VREDRAW;
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = className;
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
        // wc.hCursor = LoadCursor(0, IDC_ARROW);
        wndclassex = RegisterClassEx(&wc);
    }
    return wndclassex;
}

class Win32WindowImpl
{
    mutable bool m_clearWheel = true;
    mutable WindowState m_state = {0};

public:
    std::wstring m_configName;
    Win32WindowImpl(HWND hWnd, const std::wstring &name)
    {
        m_configName = name + L".json";
        assert(g_window == nullptr);
        g_window = this;

        RECT rect;
        GetClientRect(hWnd, &rect);
        m_state.Width = rect.right - rect.left;
        m_state.Height = rect.bottom - rect.top;
    }
    ~Win32WindowImpl()
    {
    }
    void Resize(int w, int h)
    {
        m_state.Width = w;
        m_state.Height = h;
    }
    void SetWheel(int d)
    {
        m_state.Mouse.Wheel = d;
        m_clearWheel = false;
    }
    bool IsRunning()
    {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
        {
            if (!GetMessage(&msg, NULL, 0, 0))
            {
                // return (int)msg.wParam;
                return false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return true;
    }
    mutable DWORD m_startTime = 0;
    mutable DWORD m_lastTime = 0;
    const WindowState &GetState() const
    {
        if (m_clearWheel)
        {
            m_state.Mouse.Wheel = 0;
        }

        auto now = timeGetTime();
        if (m_startTime == 0)
        {
            m_startTime = now;
            m_lastTime = now;
        }
        auto elapsed = now - m_startTime;
        auto delta = now - m_lastTime;
        m_lastTime = now;
        m_state.ElapsedSeconds = elapsed * 0.001f;
        m_state.DeltaSeconds = delta * 0.001f;

        m_clearWheel = true;
        return m_state;
    }
    WindowState &GetStateInternal()
    {
        return m_state;
    }
};

static LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
    {
        windowplacement::Save(hWnd, g_window->m_configName.c_str());
        // PostQuitMessage(0);
        return 0;
    }

    case WM_ERASEBKGND:
    {
        static int s_counter = 0;
        if (s_counter++)
        {
            return 0;
        }
        break;
    }

    case WM_PAINT:
    {
        // PAINTSTRUCT ps;
        // HDC hdc = BeginPaint(hWnd, &ps);
        // EndPaint(hWnd, &ps);
        ValidateRect(hWnd, 0);
        return 0;
    }

    case WM_SIZE:
    {
        auto w = (int)LOWORD(lParam);
        auto h = (int)HIWORD(lParam);
        g_window->Resize(w, h);
        break;
    }

    case WM_CLOSE:
        PostQuitMessage(0);
        break;

    case WM_MOUSEMOVE:
    {
        auto &state = g_window->GetStateInternal();
        state.Mouse.X = GET_X_LPARAM(lParam);
        state.Mouse.Y = GET_Y_LPARAM(lParam);
        return 0;
    }

    case WM_LBUTTONDOWN:
    {
        auto &state = g_window->GetStateInternal();
        SetCapture(hWnd);
        state.Mouse.Down(ButtonFlags::Left);
        return 0;
    }

    case WM_LBUTTONUP:
    {
        auto &state = g_window->GetStateInternal();
        state.Mouse.Up(ButtonFlags::Left);
        if (state.Mouse.Buttons == ButtonFlags::None)
        {
            ReleaseCapture();
        }
        return 0;
    }

    case WM_MBUTTONDOWN:
    {
        auto &state = g_window->GetStateInternal();
        SetCapture(hWnd);
        state.Mouse.Down(ButtonFlags::Middle);
        return 0;
    }

    case WM_MBUTTONUP:
    {
        auto &state = g_window->GetStateInternal();
        state.Mouse.Up(ButtonFlags::Middle);
        if (state.Mouse.Buttons == ButtonFlags::None)
        {
            ReleaseCapture();
        }
        return 0;
    }

    case WM_RBUTTONDOWN:
    {
        auto &state = g_window->GetStateInternal();
        SetCapture(hWnd);
        state.Mouse.Down(ButtonFlags::Right);
        return 0;
    }

    case WM_RBUTTONUP:
    {
        auto &state = g_window->GetStateInternal();
        state.Mouse.Up(ButtonFlags::Right);
        if (state.Mouse.Buttons == ButtonFlags::None)
        {
            ReleaseCapture();
        }
        return 0;
    }

    case WM_MOUSEWHEEL:
    {
        auto &state = g_window->GetStateInternal();
        g_window->SetWheel(GET_WHEEL_DELTA_WPARAM(wParam));
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

void *Win32Window::Create(int w, int h, const wchar_t *title)
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
        return nullptr;
    }
    m_impl = new Win32WindowImpl(hWnd, title);
    // ShowWindow(hWnd, SW_SHOW);

    windowplacement::Restore(hWnd, SW_SHOWNORMAL, m_impl->m_configName.c_str());

    return hWnd;
}

bool Win32Window::IsRunning()
{
    return m_impl->IsRunning();
}

const WindowState &Win32Window::GetState() const
{
    return m_impl->GetState();
}
