#pragma once
#include "Connection.h"
#include <imgui.h>
#include <memory>
#include <string>
#include <vector>

namespace ChemiaAion
{

struct NodeType
{
    std::string name_;
    std::vector<std::pair<std::string, ConnectionType>> inputs_;
    std::vector<std::pair<std::string, ConnectionType>> outputs_;
};

struct Connection;
struct Node
{
    int32_t id_; // 0 = empty, positive/negative = not selected/selected
    int32_t state_;

    ImVec2 position_;
    ImVec2 size_;

    float collapsed_height;
    float full_height;

    std::string name_;
    std::vector<std::unique_ptr<Connection>> inputs_;
    std::vector<std::unique_ptr<Connection>> outputs_;

    Node()
    {
        id_ = 0;
        state_ = NodeStateFlag_Default;

        position_ = ImVec2(0.0f, 0.0f);
        size_ = ImVec2(0.0f, 0.0f);

        collapsed_height = 0.0f;
        full_height = 0.0f;
    }

    Node *Get()
    {
        return this;
    }

    static std::unique_ptr<Node> Create(ImVec2 pos, const NodeType &type, int32_t id);
};

} // namespace ChemiaAnion