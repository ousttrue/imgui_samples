#pragma once
#include <imgui.h>
#include <string>

namespace ChemiaAion
{

enum NodeStateFlag : int32_t
{
    NodeStateFlag_Default = 1,
};

enum ConnectionType : uint32_t
{
    ConnectionType_None = 0,
    ConnectionType_Vec3,
    ConnectionType_Float,
    ConnectionType_Int,
};

struct Node;
struct Connection
{
    ImVec2 position_;
    std::string name_;
    ConnectionType type_;

    Node *target_;
    Connection *input_;
    uint32_t connections_;

    Connection()
    {
        position_ = ImVec2(0.0f, 0.0f);

        type_ = ConnectionType_None;

        target_ = nullptr;
        input_ = nullptr;
        connections_ = 0;
    }

    Connection *Get()
    {
        return this;
    }
};

} // namespace ChemiaAion