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
#include "Canvas.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

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

class NodesImpl
{
    std::vector<std::unique_ptr<Node>> nodes_;

    int32_t id_ = 0;
    NodesElement element_;

    Canvas m_canvas;

    void DisplayNodes(ImDrawList *drawList, ImVec2 offset)
    {
        ImGui::SetWindowFontScale(m_canvas.canvas_scale_);
        for (auto &node : nodes_)
        {
            node->Display(drawList, offset, m_canvas.canvas_scale_, element_);
        }
        ImGui::SetWindowFontScale(1.0f);
    }

public:
    void ProcessNodes()
    {
        m_canvas.Update();

        ImVec2 offset = m_canvas.GetOffset();
        element_.UpdateState(offset, m_canvas.canvas_size_, m_canvas.canvas_mouse_, m_canvas.canvas_scale_, nodes_);

        ImDrawList *draw_list = ImGui::GetWindowDrawList();

        // connection curve
        for (auto &node : nodes_)
        {
            for (auto &connection : node->inputs_)
            {
                if (connection->connections_ == 0)
                {
                    continue;
                }

                bool selected = false;
                selected |= element_.state_ == NodesState_SelectedConnection;
                selected |= element_.state_ == NodesState_DragingConnection;
                selected &= element_.connection_ == connection.get();

                const float distance_squared = m_canvas.RenderLines(draw_list, offset, node, connection, selected);

                if (element_.state_ == NodesState_Default)
                {

                    if (distance_squared < (10.0f * 10.0f))
                    {
                        element_.Reset(NodesState_HoverConnection);

                        element_.rectMin_ = (connection->target_->position_ + connection->input_->position_);
                        element_.rectMax_ = (node->position_ + connection->position_);

                        element_.node_ = &*node;
                        element_.connection_ = connection.get();
                    }
                }
            }
        }

        DisplayNodes(draw_list, offset);

        if (element_.state_ == NodesState_SelectingEmpty ||
            element_.state_ == NodesState_SelectingValid ||
            element_.state_ == NodesState_SelectingMore)
        {
            // yellow transparent rect
            draw_list->AddRectFilled(element_.rectMin_, element_.rectMax_, ImColor(200.0f, 200.0f, 0.0f, 0.1f));
            draw_list->AddRect(element_.rectMin_, element_.rectMax_, ImColor(200.0f, 200.0f, 0.0f, 0.5f));
        }

        ////////////////////////////////////////////////////////////////////////////////
        // Context Menu
        ////////////////////////////////////////////////////////////////////////////////

        {
            ImGui::SetCursorScreenPos(m_canvas.canvas_position_);

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
                        auto n = Node::Create(m_canvas.NewNodePosition(), node, ++id_);
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