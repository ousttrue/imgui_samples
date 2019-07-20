#pragma once

class DX11RendererImpl;
class DX11Renderer
{
    DX11RendererImpl *m_impl = nullptr;

public:
    DX11Renderer();
    ~DX11Renderer();
    void* NewFrameToRenderTarget(void *deviceContext, int width, int height, const float *clear);
    void DrawTeapot(void *deviceContext, const float *viewProjection, const float *world);
};
