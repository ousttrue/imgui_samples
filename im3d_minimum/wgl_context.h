#pragma once

class WGLContextImpl;
class WGLContext
{
    WGLContextImpl *m_impl = nullptr;

public:
    WGLContext();
    ~WGLContext();
    bool Create(void *hwnd, int major, int minor);
    void Present();
};
