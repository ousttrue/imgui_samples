#pragma once
#include "ScreenState.h"
#include <Windows.h>
#include <string>

namespace screenstate
{
class Win32Window
{
    HWND m_hwnd = NULL;
    ScreenState m_state{};
    std::wstring m_className;
    HINSTANCE m_hInstance;
    UINT m_lastTime = 0;
    UINT m_startTime = 0;
    bool m_enableSetCursor = true;

public:
    Win32Window(const wchar_t *className);
    ~Win32Window();
    HWND Create(const wchar_t *titleName, int width = 0, int height = 0);
    void Show(int nCmdShow = SW_SHOW);
    bool Update(ScreenState *pState);
    void SetEnableSetCursor(bool enable) { m_enableSetCursor = enable; }

private:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};
} // namespace screenstate