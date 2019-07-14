#pragma once


class GL3RendererImpl;
class GL3Renderer
{
    GL3RendererImpl *m_impl = nullptr;
public:
    GL3Renderer();
    ~GL3Renderer();
    void BeginFrame(int screenWidth, int screenHeight);
    void EndFrame();
    void DrawTeapot(const float *projection, const float *world);
};