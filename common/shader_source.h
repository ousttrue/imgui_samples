#pragma once
#include <string>
#include <filesystem>

class ShaderSource
{
    std::string m_src;
    std::string m_version;

public:
    ShaderSource(const std::string &src, const std::string &version = "");
    void Replace(const std::string &src, const std::string &dst);
    void Insert(const std::string &src)
    {
        m_src = src + m_src;
    }
    void Define(const std::string &name)
    {
        Insert(std::string("#define ") + name + "\n");
    }
    std::string GetSource() const;
    static ShaderSource FromPath(const std::filesystem::path &path, const std::string &version);
};
