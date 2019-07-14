#pragma once

class WGLContextImpl;
class WGLContext
{
    WGLContextImpl *m_impl = nullptr;
    void *m_hwnd = nullptr;

public:
    WGLContext();
    ~WGLContext();
    bool Create(void *hwnd, int major, int minor);
    void Present();
};
