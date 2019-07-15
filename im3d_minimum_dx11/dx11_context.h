#pragma once

class DX11ContextImpl;
class DX11Context
{
    DX11ContextImpl *m_impl = nullptr;
public:
    DX11Context();
    ~DX11Context();
    void* Create(void *hwnd);
    void Present();
};