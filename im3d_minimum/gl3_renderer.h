#pragma once
#include <string>

class GL3RendererImpl;
class GL3Renderer
{
    GL3RendererImpl *m_impl = nullptr;
public:
    GL3Renderer();
    ~GL3Renderer();
    void NewFrame(int screenWidth, int screenHeight);
    void DrawTeapot(const float *viewProjection, const float *world);
};

unsigned int CreateShader(const std::string &vsSrc, const std::string &fsSrc);
