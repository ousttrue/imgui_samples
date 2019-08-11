// Prototype of standalone node graph editor for ImGui
// Thread: https://github.com/ocornut/imgui/issues/306
//
// This is based on code by:
// @emoon https://gist.github.com/emoon/b8ff4b4ce4f1b43e79f2
// @ocornut https://gist.github.com/ocornut/7e9b3ec566a333d725d4
// @flix01 https://github.com/Flix01/imgui/blob/b248df2df98af13d4b7dbb70c92430afc47a038a/addons/imguinodegrapheditor/imguinodegrapheditor.cpp#L432

#include "Nodes.h"
#include "Node.h"
#include "NodesElement.h"
#include "Bezier.h"

namespace ChemiaAion
{
static const std::vector<NodeType> nodes_types_ =
    {
        ////////////////////////////////////////////////////////////////////////////////
        {
            {std::string("Test")},

            {{std::string("FloatTEST"), ConnectionType_Float},
             {std::string("Int"), ConnectionType_Int}},

            {{std::string("Float"), ConnectionType_Float}}},
        ////////////////////////////////////////////////////////////////////////////////
        {
            {std::string("BigInput")},

            {{std::string("Float1"), ConnectionType_Float},
             {std::string("Float2"), ConnectionType_Float},
             {std::string("Int1"), ConnectionType_Int},
             {std::string("Int2"), ConnectionType_Int},
             {{}, ConnectionType_None},
             {std::string("Float3"), ConnectionType_Float},
             {std::string("Float4"), ConnectionType_Float},
             {std::string("Float5"), ConnectionType_Float}},

            {{std::string("Float1"), ConnectionType_Float},
             {std::string("Int1"), ConnectionType_Int},
             {{}, ConnectionType_None},
             {std::string("Vec1"), ConnectionType_Vec3}}},
        ////////////////////////////////////////////////////////////////////////////////
        {
            {std::string("BigOutput")},

            {{std::string("VecTEST"), ConnectionType_Vec3},
             {{}, ConnectionType_None},
             {std::string("Int"), ConnectionType_Int},
             {std::string("Float"), ConnectionType_Float}},

            {{std::string("FloatTEST"), ConnectionType_Float},
             {std::string("Float"), ConnectionType_Float},
             {{}, ConnectionType_None},
             {std::string("Int1"), ConnectionType_Int},
             {std::string("Int2"), ConnectionType_Int},
             {{}, ConnectionType_None},
             {std::string("VecABC"), ConnectionType_Vec3},
             {std::string("VecXYZ"), ConnectionType_Vec3}}}
        ////////////////////////////////////////////////////////////////////////////////
};

////////////////////////////////////////////////////////////////////////////////

class NodesImpl
{
    std::vector<std::unique_ptr<Node>> nodes_;

    int32_t id_ = 0;
    NodesElement element_;

    // Canvas座標系
    //
    // スクロール + 拡大縮小

    ImVec2 canvas_mouse_;
    ImVec2 canvas_position_;
    ImVec2 canvas_size_;
    ImVec2 canvas_scroll_;

    float canvas_scale_ = 1.0f;

    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////

    void UpdateScroll()
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

    void RenderLines(ImDrawList *draw_list, ImVec2 offset)
    {
        for (auto &node : nodes_)
        {
            for (auto &connection : node->inputs_)
            {
                if (connection->connections_ == 0)
                {
                    continue;
                }

                ImVec2 p1 = offset;
                ImVec2 p4 = offset;

                if (connection->target_->state_ > 0)
                { // we are connected to output of not a collapsed node
                    p1 += ((connection->target_->position_ + connection->input_->position_) * canvas_scale_);
                }
                else
                { // we are connected to output of a collapsed node
                    p1 += ((connection->target_->position_ + ImVec2(connection->target_->size_.x, connection->target_->size_.y / 2.0f)) * canvas_scale_);
                }

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

                if (element_.state_ == NodesState_Default)
                {
                    const float distance_squared = GetSquaredDistanceToBezierCurve(ImGui::GetIO().MousePos, p1, p2, p3, p4);

                    if (distance_squared < (10.0f * 10.0f))
                    {
                        element_.Reset(NodesState_HoverConnection);

                        element_.rect_ = ImRect(
                            (connection->target_->position_ + connection->input_->position_),
                            (node->position_ + connection->position_));

                        element_.node_ = &*node;
                        element_.connection_ = connection.get();
                    }
                }

                bool selected = false;
                selected |= element_.state_ == NodesState_SelectedConnection;
                selected |= element_.state_ == NodesState_DragingConnection;
                selected &= element_.connection_ == connection.get();

                draw_list->AddBezierCurve(p1, p2, p3, p4, ImColor(0.5f, 0.5f, 0.5f, 1.0f), 2.0f * canvas_scale_);

                if (selected)
                {
                    draw_list->AddBezierCurve(p1, p2, p3, p4, ImColor(1.0f, 1.0f, 1.0f, 0.25f), 4.0f * canvas_scale_);
                }
            }
        }
    }
    void DisplayNodes(ImDrawList *drawList, ImVec2 offset)
    {
        ImGui::SetWindowFontScale(canvas_scale_);

        for (auto &node : nodes_)
        {
            node->Display(drawList, offset, canvas_scale_, element_);
        }

        ImGui::SetWindowFontScale(1.0f);
    }

public:
    void ProcessNodes()
    {
        ////////////////////////////////////////////////////////////////////////////////
        // UpdateCanvas
        ////////////////////////////////////////////////////////////////////////////////

        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
        {
            canvas_mouse_ = ImGui::GetIO().MousePos - ImGui::GetCursorScreenPos();
            canvas_position_ = ImGui::GetCursorScreenPos();
            canvas_size_ = ImGui::GetWindowSize();

            UpdateScroll();
        }

        ////////////////////////////////////////////////////////////////////////////////
        // DrawGrid
        ////////////////////////////////////////////////////////////////////////////////

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

        ////////////////////////////////////////////////////////////////////////////////
        // Update
        ////////////////////////////////////////////////////////////////////////////////
        ImVec2 offset = canvas_position_ + canvas_scroll_;
        element_.UpdateState(offset, canvas_size_, canvas_mouse_, canvas_scale_, nodes_);

        ////////////////////////////////////////////////////////////////////////////////
        // Draw
        ////////////////////////////////////////////////////////////////////////////////
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        RenderLines(draw_list, offset);
        DisplayNodes(draw_list, offset);
        if (element_.state_ == NodesState_SelectingEmpty || element_.state_ == NodesState_SelectingValid || element_.state_ == NodesState_SelectingMore)
        {
            // yellow transparent rect
            draw_list->AddRectFilled(element_.rect_.Min, element_.rect_.Max, ImColor(200.0f, 200.0f, 0.0f, 0.1f));
            draw_list->AddRect(element_.rect_.Min, element_.rect_.Max, ImColor(200.0f, 200.0f, 0.0f, 0.5f));
        }

        ////////////////////////////////////////////////////////////////////////////////
        // Context Menu
        ////////////////////////////////////////////////////////////////////////////////

        {
            ImGui::SetCursorScreenPos(canvas_position_);

            bool consider_menu = !ImGui::IsAnyItemHovered();
            consider_menu &= ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
            consider_menu &= element_.state_ == NodesState_Default || element_.state_ == NodesState_Selected;
            consider_menu &= ImGui::IsMouseReleased(1);

            if (consider_menu)
            {
                ImGuiContext *context = ImGui::GetCurrentContext();

                if (context->IO.MouseDragMaxDistanceSqr[1] < 36.0f)
                {
                    ImGui::OpenPopup("NodesContextMenu");
                }
            }

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
            if (ImGui::BeginPopup("NodesContextMenu"))
            {
                element_.Reset(NodesState_Block);

                for (auto &node : nodes_types_)
                {
                    if (ImGui::MenuItem(node.name_.c_str()))
                    {
                        element_.Reset();
                        auto n = Node::Create((canvas_mouse_ - canvas_scroll_) / canvas_scale_, node, ++id_);
                        nodes_.push_back(std::move(n));
                        element_.node_ = nodes_.back().get();
                    }
                }
                ImGui::EndPopup();
            }
            ImGui::PopStyleVar();
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
Nodes::Nodes()
    : m_impl(new NodesImpl)
{
}

Nodes::~Nodes()
{
    delete m_impl;
}

void Nodes::ProcessNodes()
{
    ImGui::Begin("Nodes");

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

    ImGui::BeginChild("NodesScrollingRegion", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove);

    m_impl->ProcessNodes();

    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
    ImGui::End();
}

} // namespace ChemiaAion