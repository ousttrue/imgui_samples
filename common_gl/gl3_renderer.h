#pragma once
#include <string>
#include <filesystem>

unsigned int CreateShader(const std::string &debugName, const std::string &vsSrc, const std::string &fsSrc);

class GL3RendererImpl;
class GL3Renderer
{
    GL3RendererImpl *m_impl = nullptr;
    std::string m_version;

public:
    GL3Renderer(const std::string &shaderVerrsion = "#version 140");
    ~GL3Renderer();
    void NewFrame(int screenWidth, int screenHeight);
    void DrawTeapot(const float *viewProjection, const float *world);
};
