#pragma once
#include <nlohmann/json.hpp>
#include <fstream>

namespace windowplacement
{
struct Configuration
{
    WINDOWPLACEMENT window;
};
} // namespace windowplacement

namespace nlohmann
{
template <>
struct adl_serializer<RECT>
{
    static void to_json(json &j, const RECT &data)
    {
        j = json{
            {"left", data.left},
            {"right", data.right},
            {"top", data.top},
            {"bottom", data.bottom},
        };
    }

    static void from_json(const json &j, RECT &data)
    {
        j.at("left").get_to(data.left);
        j.at("right").get_to(data.right);
        j.at("top").get_to(data.top);
        j.at("bottom").get_to(data.bottom);
    }
};
template <>
struct adl_serializer<POINT>
{
    static void to_json(json &j, const POINT &data)
    {
        j = json{
            {"x", data.x},
            {"y", data.y},
        };
    }

    static void from_json(const json &j, POINT &data)
    {
        j.at("x").get_to(data.x);
        j.at("y").get_to(data.y);
    }
};
template <>
struct adl_serializer<WINDOWPLACEMENT>
{
    static void to_json(json &j, const WINDOWPLACEMENT &data)
    {
        j = json{
            {"length", data.length},
            {"flags", data.flags},
            {"showCmd", data.showCmd},
            {"ptMinPosition", data.ptMinPosition},
            {"ptMaxPosition", data.ptMaxPosition},
            {"rcNormalPosition", data.rcNormalPosition},
        };
    }
    static void from_json(const json &j, WINDOWPLACEMENT &data)
    {
        j.at("length").get_to(data.length);
        j.at("flags").get_to(data.flags);
        j.at("showCmd").get_to(data.showCmd);
        j.at("ptMinPosition").get_to(data.ptMinPosition);
        j.at("ptMaxPosition").get_to(data.ptMaxPosition);
        j.at("rcNormalPosition").get_to(data.rcNormalPosition);
    }
};
template <>
struct adl_serializer<windowplacement::Configuration>
{
    static void to_json(json &j, const windowplacement::Configuration &data)
    {
        j = json{
            {"window", data.window},
        };
    }

    static void from_json(const json &j, windowplacement::Configuration &data)
    {
        j.at("window").get_to(data.window);
    }
};
} // namespace nlohmann

namespace windowplacement
{

///
/// Window state save & restore
///
inline std::string read_allstring(const std::wstring &path)
{
    auto buffer = std::string();

    // open the file for binary reading
    std::ifstream file;
    file.open(path, std::ios_base::binary);
    if (file.is_open())
    {
        // get the length of the file
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        // read the file
        buffer.resize(fileSize);
        file.read(buffer.data(), fileSize);

        file.close();
    }

    return buffer;
}

inline void Restore(HWND hWnd, UINT showCmd, const wchar_t *config_file)
{
    auto config_str = read_allstring(config_file);
    if (config_str.empty())
    {
        ShowWindow(hWnd, showCmd);
    }
    else
    {
        auto parsed = nlohmann::json::parse(config_str);
        Configuration config = parsed;
        // config.window.showCmd = showCmd;
        SetWindowPlacement(hWnd, &config.window);
    }
}

inline void Save(HWND hWnd, const wchar_t *config_file)
{
    Configuration config;
    if (GetWindowPlacement(hWnd, &config.window))
    {
        nlohmann::json config_json = config;
        std::ofstream config_file(config_file);
        config_file << config_json.dump(4);
    }
}
} // namespace windowplacement
