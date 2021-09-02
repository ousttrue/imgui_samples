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

struct NodesElement;
struct Connection;
struct Node
{
    int32_t id_ = 0; // 0 = empty, positive/negative = not selected/selected
    int32_t state_ = NodeStateFlag_Default;

    ImVec2 position_ = ImVec2(0.0f, 0.0f);
    ImVec2 size_ = ImVec2(0.0f, 0.0f);

    float collapsed_height = 0;
    float full_height = 0;

    std::string name_;
    std::vector<std::unique_ptr<Connection>> inputs_;
    std::vector<std::unique_ptr<Connection>> outputs_;

    void Display(ImDrawList *drawList, ImVec2 offset, float canvas_scale_, NodesElement &element_);

    static std::unique_ptr<Node> Create(ImVec2 pos, const NodeType &type, int32_t id);
};

} // namespace ChemiaAion