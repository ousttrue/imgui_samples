#define IMGUI_DEFINE_MATH_OPERATORS

#include "NodesElement.h"
#include "Node.h"
#include "Bezier.h"

namespace ChemiaAion
{
static Node *GetHoverNode(std::vector<std::unique_ptr<Node>> &nodes_, ImVec2 offset, ImVec2 pos, float canvas_scale_)
{
    for (auto &node : nodes_)
    {
        ImRect rect((node->position_ * canvas_scale_) + offset, ((node->position_ + node->size_) * canvas_scale_) + offset);

        rect.Expand(2.0f);

        if (rect.Contains(pos))
        {
            return &*node;
        }
    }

    return nullptr;
}

void NodesElement::UpdateState(ImVec2 offset,
                               const ImVec2 &canvas_size_, const ImVec2 &canvas_mouse_, float canvas_scale_,
                               std::vector<std::unique_ptr<Node>> &nodes_)
{
    if (state_ == NodesState_HoverNode && ImGui::IsMouseDoubleClicked(0))
    {
        if (node_->state_ < 0)
        { // collapsed node goes to full
            node_->size_.y = node_->full_height;
        }
        else
        { // full node goes to collapsed
            node_->size_.y = node_->collapsed_height;
        }

        node_->state_ = -node_->state_;
    }

    switch (state_)
    {
    case NodesState_Default:
    {
        if (ImGui::IsMouseDown(0) && !ImGui::IsMouseDown(1) && !ImGui::IsMouseDown(2))
        {
            ImRect canvas = ImRect(ImVec2(0.0f, 0.0f), canvas_size_);

            if (!canvas.Contains(canvas_mouse_))
            {
                break;
            }

            Reset(NodesState_SelectingEmpty);

            position_ = ImGui::GetIO().MousePos;
            rect_.Min = ImGui::GetIO().MousePos;
            rect_.Max = ImGui::GetIO().MousePos;
        }
    }
    break;

    // helper: just block all states till next update
    case NodesState_Block:
    {
        Reset();
    }
    break;

    case NodesState_HoverConnection:
    {
        const float distance_squared = GetSquaredDistanceToBezierCurve(
            ImGui::GetIO().MousePos,
            offset + (rect_.Min * canvas_scale_),
            offset + (rect_.Min * canvas_scale_) + (ImVec2(+50.0f, 0.0f) * canvas_scale_),
            offset + (rect_.Max * canvas_scale_) + (ImVec2(-50.0f, 0.0f) * canvas_scale_),
            offset + (rect_.Max * canvas_scale_));

        if (distance_squared > (10.0f * 10.0f))
        {
            Reset();
            break;
        }

        if (ImGui::IsMouseDown(0))
        {
            state_ = NodesState_SelectedConnection;
        }
    }
    break;

    case NodesState_DragingInput:
    {
        if (!ImGui::IsMouseDown(0) || ImGui::IsMouseClicked(1))
        {
            Reset(NodesState_Block);
            break;
        }

        ImVec2 p1 = offset + (position_ * canvas_scale_);
        ImVec2 p2 = p1 + (ImVec2(-50.0f, 0.0f) * canvas_scale_);
        ImVec2 p3 = ImGui::GetIO().MousePos + (ImVec2(+50.0f, 0.0f) * canvas_scale_);
        ImVec2 p4 = ImGui::GetIO().MousePos;

        ImGui::GetWindowDrawList()->AddBezierCurve(p1, p2, p3, p4, ImColor(0.0f, 1.0f, 0.0f, 1.0f), 2.0f * canvas_scale_);
    }
    break;

    case NodesState_DragingInputValid:
    {
        state_ = NodesState_DragingInput;

        if (ImGui::IsMouseClicked(1))
        {
            Reset(NodesState_Block);
            break;
        }

        ImVec2 p1 = offset + (position_ * canvas_scale_);
        ImVec2 p2 = p1 + (ImVec2(-50.0f, 0.0f) * canvas_scale_);
        ImVec2 p3 = ImGui::GetIO().MousePos + (ImVec2(+50.0f, 0.0f) * canvas_scale_);
        ImVec2 p4 = ImGui::GetIO().MousePos;

        ImGui::GetWindowDrawList()->AddBezierCurve(p1, p2, p3, p4, ImColor(0.0f, 1.0f, 0.0f, 1.0f), 2.0f * canvas_scale_);
    }
    break;

    case NodesState_DragingOutput:
    {
        if (!ImGui::IsMouseDown(0) || ImGui::IsMouseClicked(1))
        {
            Reset(NodesState_Block);
            break;
        }

        ImVec2 p1 = offset + (position_ * canvas_scale_);
        ImVec2 p2 = p1 + (ImVec2(+50.0f, 0.0f) * canvas_scale_);
        ImVec2 p3 = ImGui::GetIO().MousePos + (ImVec2(-50.0f, 0.0f) * canvas_scale_);
        ImVec2 p4 = ImGui::GetIO().MousePos;

        ImGui::GetWindowDrawList()->AddBezierCurve(p1, p2, p3, p4, ImColor(0.0f, 1.0f, 0.0f, 1.0f), 2.0f * canvas_scale_);
    }
    break;

    case NodesState_DragingOutputValid:
    {
        state_ = NodesState_DragingOutput;

        if (ImGui::IsMouseClicked(1))
        {
            Reset(NodesState_Block);
            break;
        }

        ImVec2 p1 = offset + (position_ * canvas_scale_);
        ImVec2 p2 = p1 + (ImVec2(+50.0f, 0.0f) * canvas_scale_);
        ImVec2 p3 = ImGui::GetIO().MousePos + (ImVec2(-50.0f, 0.0f) * canvas_scale_);
        ImVec2 p4 = ImGui::GetIO().MousePos;

        ImGui::GetWindowDrawList()->AddBezierCurve(p1, p2, p3, p4, ImColor(0.0f, 1.0f, 0.0f, 1.0f), 2.0f * canvas_scale_);
    }
    break;

    case NodesState_SelectingEmpty:
    {
        if (!ImGui::IsMouseDown(0))
        {
            Reset(NodesState_Block);
            break;
        }

        rect_.Min = ImMin(position_, ImGui::GetIO().MousePos);
        rect_.Max = ImMax(position_, ImGui::GetIO().MousePos);
    }
    break;

    case NodesState_SelectingValid:
    {
        if (!ImGui::IsMouseDown(0))
        {
            Reset(NodesState_Selected);
            break;
        }

        rect_.Min = ImMin(position_, ImGui::GetIO().MousePos);
        rect_.Max = ImMax(position_, ImGui::GetIO().MousePos);

        state_ = NodesState_SelectingEmpty;
    }
    break;

    case NodesState_SelectingMore:
    {
        rect_.Min = ImMin(position_, ImGui::GetIO().MousePos);
        rect_.Max = ImMax(position_, ImGui::GetIO().MousePos);

        if (ImGui::IsMouseDown(0) && ImGui::GetIO().KeyShift)
        {
            break;
        }

        for (auto &node : nodes_)
        {
            ImVec2 node_rect_min = offset + (node->position_ * canvas_scale_);
            ImVec2 node_rect_max = node_rect_min + (node->size_ * canvas_scale_);

            ImRect node_rect(node_rect_min, node_rect_max);

            if (ImGui::GetIO().KeyCtrl && rect_.Overlaps(node_rect))
            {
                node->id_ = -abs(node->id_); // add "selected" flag
                continue;
            }

            if (!ImGui::GetIO().KeyCtrl && rect_.Contains(node_rect))
            {
                node->id_ = -abs(node->id_); // add "selected" flag
                continue;
            }
        }

        Reset(NodesState_Selected);
    }
    break;

    case NodesState_Selected:
    {
        // delete all selected nodes
        if (ImGui::IsKeyPressed(ImGui::GetIO().KeyMap[ImGuiKey_Delete]))
        {
            std::vector<std::unique_ptr<Node>> replacement;
            replacement.reserve(nodes_.size());

            // delete connections
            for (auto &node : nodes_)
            {
                for (auto &connection : node->inputs_)
                {
                    if (connection->connections_ == 0)
                    {
                        continue;
                    }

                    if (node->id_ < 0)
                    {
                        connection->input_->connections_--;
                    }

                    if (connection->target_->id_ < 0)
                    {
                        connection->target_ = nullptr;
                        connection->input_ = nullptr;
                        connection->connections_ = 0;
                    }
                }
            }

            // save not selected nodes
            for (auto &node : nodes_)
            {
                if (node->id_ > 0)
                {
                    replacement.push_back(std::move(node));
                }
            }

            nodes_ = std::move(replacement);

            Reset();
            break;
        }

        // no action taken
        if (!ImGui::IsMouseClicked(0))
        {
            break;
        }

        Reset();
        auto hovered = GetHoverNode(nodes_, offset, ImGui::GetIO().MousePos, canvas_scale_);

        // empty area under the mouse
        if (!hovered)
        {
            position_ = ImGui::GetIO().MousePos;
            rect_.Min = ImGui::GetIO().MousePos;
            rect_.Max = ImGui::GetIO().MousePos;

            if (ImGui::GetIO().KeyShift)
            {
                state_ = NodesState_SelectingMore;
            }
            else
            {
                state_ = NodesState_SelectingEmpty;
            }
            break;
        }

        // lets select node under the mouse
        if (ImGui::GetIO().KeyShift)
        {
            hovered->id_ = -abs(hovered->id_);
            state_ = NodesState_DragingSelected;
            break;
        }

        // lets toggle selection of a node under the mouse
        if (!ImGui::GetIO().KeyShift && ImGui::GetIO().KeyCtrl)
        {
            if (hovered->id_ > 0)
            {
                hovered->id_ = -abs(hovered->id_);
                state_ = NodesState_DragingSelected;
            }
            else
            {
                hovered->id_ = abs(hovered->id_);
                state_ = NodesState_Selected;
            }
            break;
        }

        // lets start dragging
        if (hovered->id_ < 0)
        {
            state_ = NodesState_DragingSelected;
            break;
        }

        // not selected node clicked, lets jump selection to it
        for (auto &node : nodes_)
        {
            if (node.get() != hovered)
            {
                node->id_ = abs(node->id_);
            }
        }
    }
    break;

    case NodesState_DragingSelected:
    {
        if (!ImGui::IsMouseDown(0))
        {
            if (node_)
            {
                if (ImGui::GetIO().KeyShift || ImGui::GetIO().KeyCtrl)
                {
                    Reset(NodesState_Selected);
                    break;
                }

                state_ = NodesState_HoverNode;
            }
            else
            {
                Reset(NodesState_Selected);
            }
        }
        else
        {
            for (auto &node : nodes_)
            {
                if (node->id_ < 0)
                {
                    node->position_ += ImGui::GetIO().MouseDelta / canvas_scale_;
                }
            }
        }
    }
    break;

    case NodesState_SelectedConnection:
    {
        if (ImGui::IsMouseClicked(1))
        {
            Reset(NodesState_Block);
            break;
        }

        if (ImGui::IsMouseDown(0))
        {
            const float distance_squared = GetSquaredDistanceToBezierCurve(
                ImGui::GetIO().MousePos,
                offset + (rect_.Min * canvas_scale_),
                offset + (rect_.Min * canvas_scale_) + (ImVec2(+50.0f, 0.0f) * canvas_scale_),
                offset + (rect_.Max * canvas_scale_) + (ImVec2(-50.0f, 0.0f) * canvas_scale_),
                offset + (rect_.Max * canvas_scale_));

            if (distance_squared > (10.0f * 10.0f))
            {
                Reset();
                break;
            }

            state_ = NodesState_DragingConnection;
        }
    }
    break;

    case NodesState_DragingConnection:
    {
        if (!ImGui::IsMouseDown(0))
        {
            state_ = NodesState_SelectedConnection;
            break;
        }

        if (ImGui::IsMouseClicked(1))
        {
            Reset(NodesState_Block);
            break;
        }

        node_->position_ += ImGui::GetIO().MouseDelta / canvas_scale_;
        connection_->target_->position_ += ImGui::GetIO().MouseDelta / canvas_scale_;
    }
    break;
    }
}

} // namespace ChemiaAion