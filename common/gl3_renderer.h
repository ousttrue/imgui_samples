#pragma once
#include <string>
#include <filesystem>

class GL3ShaderSource
{
    std::string m_src;
    std::string m_version;

public:
    GL3ShaderSource(const std::string &src, const std::string &version);
    void Define(const std::string &name);
    void Replace(const std::string &src, const std::string &dst);
    void Insert(const std::string &src);
    std::string GetSource() const;
    static GL3ShaderSource FromPath(const std::filesystem::path &path, const std::string &version);
};

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
