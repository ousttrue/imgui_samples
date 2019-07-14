#pragma once
#include "mouse_state.h"
#include <tuple>

///
/// Windows API Window
///
/// * window size
/// * mouse inputs
///
class Win32Window
{
    class Win32WindowImpl *m_impl = nullptr;

public:
    Win32Window();
    ~Win32Window();
    bool Create(int w, int h, const wchar_t *title);
    bool IsRunning();
    void *GetHandle() const;
    std::tuple<int, int> GetSize() const;
    bool HasFocus() const;
    MouseState GetMouseState() const;
    float GetTimeSeconds() const;
};
