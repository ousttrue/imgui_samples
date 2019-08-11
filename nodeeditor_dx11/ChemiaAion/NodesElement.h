#pragma once
#include <imgui.h>
#include <cstdint>
#include <vector>
#include <memory>

namespace ChemiaAion
{

enum NodesState : uint32_t
{
    NodesState_Default = 0,
    NodesState_Block, // helper: just block all states till next update (frame)
    NodesState_HoverIO,
    NodesState_HoverConnection,
    NodesState_HoverNode,
    NodesState_DragingInput,
    NodesState_DragingInputValid,
    NodesState_DragingOutput,
    NodesState_DragingOutputValid,
    NodesState_DragingConnection,
    NodesState_DragingSelected,
    NodesState_SelectingEmpty,
    NodesState_SelectingValid,
    NodesState_SelectingMore,
    NodesState_Selected,
    NodesState_SelectedConnection
};

struct Node;
struct Connection;
struct NodesElement
{
    NodesState state_;

    ImVec2 position_;
    ImVec2 rectMin_;
    ImVec2 rectMax_;

    Node *node_;
    Connection *connection_;

    NodesElement()
    {
        Reset();
    }

    void Reset(NodesState state = NodesState_Default)
    {
        state_ = state;

        position_ = ImVec2(0.0f, 0.0f);
        rectMin_ = ImVec2(0.0f, 0.0f);
        rectMax_ = ImVec2(0.0f, 0.0f);

        node_ = nullptr;
        connection_ = nullptr;
    }

    void UpdateState(ImVec2 offset,
                     const ImVec2 &canvas_size_, const ImVec2 &canvas_mouse_, float canvas_scale_,
                     std::vector<std::unique_ptr<Node>> &nodes_);
};

} // namespace ChemiaAion