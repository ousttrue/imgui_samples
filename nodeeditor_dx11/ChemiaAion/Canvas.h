#pragma once
#include <imgui.h>
#include <memory>

namespace ChemiaAion
{

struct Node;
struct Connection;
struct Canvas
{
    ImVec2 canvas_mouse_;
    ImVec2 canvas_position_;
    ImVec2 canvas_size_;
    ImVec2 canvas_scroll_;

    float canvas_scale_ = 1.0f;

    ImVec2 GetOffset() const;
    ImVec2 NewNodePosition() const;

    ImVec2 Update();

private:
    void DrawGrid();
    void UpdateScroll();

public:
    float RenderLines(ImDrawList *draw_list, ImVec2 offset,
                      const std::unique_ptr<Node> &node,
                      const std::unique_ptr<Connection> &connection,
                      bool selected);
};

} // namespace ChemiAion
