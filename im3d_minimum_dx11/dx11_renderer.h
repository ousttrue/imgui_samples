#pragma once

class DX11RendererImpl;
class DX11Renderer
{
    DX11RendererImpl *m_impl = nullptr;

public:
    DX11Renderer();
    ~DX11Renderer();
    bool Create(void *device);
    void* NewFrame(int screenWidth, int screenHeight);
    void DrawTeapot(void *deviceContext, const float *viewProjection, const float *world);
};
