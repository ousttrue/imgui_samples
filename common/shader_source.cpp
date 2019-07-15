#include "shader_source.h"
#include <vector>
#include <fstream>
#include <sstream>

static std::string trim(const std::string &src)
{
    auto it = src.begin();
    for (; it != src.end(); ++it)
    {
        if (!isspace(*it))
        {
            break;
        }
    }
    return std::string(it, src.end());
}

///
/// ShaderSource
///
ShaderSource::ShaderSource(const std::string &src, const std::string &version)
    : m_src(trim(src)), m_version(version)
{
}

std::vector<std::byte> readall(const std::filesystem::path &path)
{
    std::vector<std::byte> buffer;
    std::ifstream io(path, std::ios::binary);
    if (io)
    {
        io.seekg(0, std::ios::end);
        buffer.resize(io.tellg());
        io.seekg(0, std::ios::beg);
        io.read((char *)buffer.data(), buffer.size());
    }
    return buffer;
}

ShaderSource ShaderSource::FromPath(const std::filesystem::path &path, const std::string &version)
{
    auto buffer = readall(path);
    auto begin = (const char *)buffer.data();
    return ShaderSource(std::string(begin, begin + buffer.size()), version);
}

void ShaderSource::Replace(const std::string &src, const std::string &dst)
{
    std::stringstream ss;

    auto pos = 0;
    while (pos < m_src.size())
    {
        auto found = m_src.find(src, pos);
        if (found == std::string::npos)
        {
            // 残り全部
            ss << m_src.substr(pos);
            break;
        }

        ss << m_src.substr(pos, found - pos);
        ss << dst;
        pos = (int)(found + src.size() + 1);
    }

    m_src = ss.str();
}

std::string ShaderSource::GetSource() const
{
    if(m_version.empty()){
        return m_src;
    }
    return m_version + "\n" + m_src;
}

