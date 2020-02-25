#pragma once

class DX11ContextImpl;
class DX11Context
{
    DX11ContextImpl *m_impl = nullptr;
public:
    DX11Context();
    ~DX11Context();
    void* Create(void *hwnd);
    // Get I3D11DeviceContext that has backbuffer
    void* NewFrame(int windowWidth, int windowHeight);
    void Present();
    void *GetDeviceContext();
};
