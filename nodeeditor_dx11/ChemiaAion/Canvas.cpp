#include "Canvas.h"
#include "Node.h"
#include "Connection.h"
#include "Bezier.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

namespace ChemiaAion
{

ImVec2 Canvas::GetOffset() const
{
    return canvas_position_ + canvas_scroll_;
}

ImVec2 Canvas::NewNodePosition() const
{
    return (canvas_mouse_ - canvas_scroll_) / canvas_scale_;
}

ImVec2 Canvas::Update()
{
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
    {
        canvas_mouse_ = ImGui::GetIO().MousePos - ImGui::GetCursorScreenPos();
        canvas_position_ = ImGui::GetCursorScreenPos();
        canvas_size_ = ImGui::GetWindowSize();

        UpdateScroll();
    }

    DrawGrid();

    return GetOffset();
}

void Canvas::DrawGrid()
{
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    ImU32 color = ImColor(0.5f, 0.5f, 0.5f, 0.8f);
    const float size = 64.0f * canvas_scale_;

    for (float x = fmodf(canvas_scroll_.x, size); x < canvas_size_.x; x += size)
    {
        draw_list->AddLine(ImVec2(x, 0.0f) + canvas_position_, ImVec2(x, canvas_size_.y) + canvas_position_, color);
    }

    for (float y = fmodf(canvas_scroll_.y, size); y < canvas_size_.y; y += size)
    {
        draw_list->AddLine(ImVec2(0.0f, y) + canvas_position_, ImVec2(canvas_size_.x, y) + canvas_position_, color);
    }
}

void Canvas::UpdateScroll()
{
    {
        ImVec2 scroll;
        if (ImGui::IsMouseDown(2))
        {
            scroll += ImGui::GetIO().MouseDelta;
        }
        canvas_scroll_ += scroll;
    }

    {
        ImVec2 mouse = canvas_mouse_;
        float zoom = 0.0f;
        {
            zoom += ImGui::GetIO().MouseWheel;
        }

        ImVec2 focus = (mouse - canvas_scroll_) / canvas_scale_;

        if (zoom < 0.0f)
        {
            canvas_scale_ /= 1.05f;
        }

        if (zoom > 0.0f)
        {
            canvas_scale_ *= 1.05f;
        }

        if (canvas_scale_ < 0.3f)
        {
            canvas_scale_ = 0.3f;
        }

        if (canvas_scale_ > 3.0f)
        {
            canvas_scale_ = 3.0f;
        }

        focus = canvas_scroll_ + (focus * canvas_scale_);
        canvas_scroll_ += (mouse - focus);
    }
}

float Canvas::RenderLines(ImDrawList *draw_list, ImVec2 offset,
                          const std::unique_ptr<Node> &node,
                          const std::unique_ptr<Connection> &connection,
                          bool selected)
{
    ImVec2 p1 = offset;
    if (connection->target_->state_ > 0)
    { // we are connected to output of not a collapsed node
        p1 += ((connection->target_->position_ + connection->input_->position_) * canvas_scale_);
    }
    else
    { // we are connected to output of a collapsed node
        p1 += ((connection->target_->position_ + ImVec2(connection->target_->size_.x, connection->target_->size_.y / 2.0f)) * canvas_scale_);
    }

    ImVec2 p4 = offset;
    if (node->state_ > 0)
    { // we are not a collapsed node
        p4 += ((node->position_ + connection->position_) * canvas_scale_);
    }
    else
    { // we are a collapsed node
        p4 += ((node->position_ + ImVec2(0.0f, node->size_.y / 2.0f)) * canvas_scale_);
    }

    // default bezier control points
    ImVec2 p2 = p1 + (ImVec2(+50.0f, 0.0f) * canvas_scale_);
    ImVec2 p3 = p4 + (ImVec2(-50.0f, 0.0f) * canvas_scale_);

    draw_list->AddBezierCurve(p1, p2, p3, p4, ImColor(0.5f, 0.5f, 0.5f, 1.0f), 2.0f * canvas_scale_);

    if (selected)
    {
        draw_list->AddBezierCurve(p1, p2, p3, p4, ImColor(1.0f, 1.0f, 1.0f, 0.25f), 4.0f * canvas_scale_);
    }

    return GetSquaredDistanceToBezierCurve(ImGui::GetIO().MousePos, p1, p2, p3, p4);
}

} // namespace ChemiaAion