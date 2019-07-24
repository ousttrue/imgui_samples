#define IMGUI_DEFINE_MATH_OPERATORS
#include "Node.h"
#include "NodesElement.h"
#include <algorithm>
#include <imgui_internal.h>

static bool IsConnectorHovered(ImVec2 connection, float radius)
{

    ImVec2 delta = ImGui::GetIO().MousePos - connection;

    return ((delta.x * delta.x) + (delta.y * delta.y)) < (radius * radius);
}

namespace ChemiaAion
{

std::unique_ptr<Node> Node::Create(ImVec2 pos, const NodeType &type, int32_t id)
{
    auto node = std::make_unique<Node>();

    node->id_ = id;
    node->name_ = type.name_ + std::to_string(id).c_str();
    node->position_ = pos;

    {
        auto &inputs = node->inputs_;
        std::for_each(
            type.inputs_.begin(),
            type.inputs_.end(),
            [&inputs](auto &element) {
                auto connection = std::make_unique<Connection>();
                connection->name_ = element.first;
                connection->type_ = element.second;

                inputs.push_back(std::move(connection));
            });

        auto &outputs = node->outputs_;
        std::for_each(
            type.outputs_.begin(),
            type.outputs_.end(),
            [&outputs](auto &element) {
                auto connection = std::make_unique<Connection>();
                connection->name_ = element.first;
                connection->type_ = element.second;

                outputs.push_back(std::move(connection));
            });
    }

    ////////////////////////////////////////////////////////////////////////////////

    ImVec2 title_size = ImGui::CalcTextSize(node->name_.c_str());

    const float vertical_padding = 1.5f;

    ////////////////////////////////////////////////////////////////////////////////

    ImVec2 inputs_size;
    for (auto &connection : node->inputs_)
    {
        ImVec2 name_size = ImGui::CalcTextSize(connection->name_.c_str());
        inputs_size.x = ImMax(inputs_size.x, name_size.x);
        inputs_size.y += name_size.y * vertical_padding;
    }

    ImVec2 outputs_size;
    for (auto &connection : node->outputs_)
    {
        ImVec2 name_size = ImGui::CalcTextSize(connection->name_.c_str());
        outputs_size.x = ImMax(outputs_size.x, name_size.x);
        outputs_size.y += name_size.y * vertical_padding;
    }

    ////////////////////////////////////////////////////////////////////////////////

    node->size_.x = ImMax((inputs_size.x + outputs_size.x), title_size.x);
    node->size_.x += title_size.y * 6.0f;

    node->collapsed_height = (title_size.y * 2.0f);
    node->full_height = (title_size.y * 3.0f) + ImMax(inputs_size.y, outputs_size.y);

    node->size_.y = node->full_height;

    node->position_ -= node->size_ / 2.0f;

    ////////////////////////////////////////////////////////////////////////////////

    inputs_size = ImVec2(title_size.y * 0.75f, title_size.y * 2.5f);
    for (auto &connection : node->inputs_)
    {
        const float half = ((ImGui::CalcTextSize(connection->name_.c_str()).y * vertical_padding) / 2.0f);

        inputs_size.y += half;
        connection->position_ = ImVec2(inputs_size.x, inputs_size.y);
        inputs_size.y += half;
    }

    outputs_size = ImVec2(node->size_.x - (title_size.y * 0.75f), title_size.y * 2.5f);
    for (auto &connection : node->outputs_)
    {
        const float half = ((ImGui::CalcTextSize(connection->name_.c_str()).y * vertical_padding) / 2.0f);

        outputs_size.y += half;
        connection->position_ = ImVec2(outputs_size.x, outputs_size.y);
        outputs_size.y += half;
    }

    ////////////////////////////////////////////////////////////////////////////////
    return node;
}

void Node::Display(ImDrawList *drawList, ImVec2 offset, float canvas_scale_, NodesElement &element_)
{
    ImGui::PushID(abs(id_));
    ImGui::BeginGroup();

    ImVec2 node_rect_min = offset + (position_ * canvas_scale_);
    ImVec2 node_rect_max = node_rect_min + (size_ * canvas_scale_);

    ImGui::SetCursorScreenPos(node_rect_min);
    ImGui::InvisibleButton("Node", size_ * canvas_scale_);

    ////////////////////////////////////////////////////////////////////////////////

    // state machine for node hover/drag
    {
        bool node_hovered = ImGui::IsItemHovered();
        bool node_active = ImGui::IsItemActive();

        if (node_hovered && element_.state_ == NodesState_HoverNode)
        {
            element_.node_ = this;

            if (node_active)
            {
                id_ = -abs(id_); // add "selected" flag
                element_.state_ = NodesState_DragingSelected;
            }
        }

        if (node_hovered && element_.state_ == NodesState_Default)
        {
            element_.node_ = this;

            if (node_active)
            {
                id_ = -abs(id_); // add "selected" flag
                element_.state_ = NodesState_DragingSelected;
            }
            else
            {
                element_.state_ = NodesState_HoverNode;
            }
        }

        if (!node_hovered && element_.state_ == NodesState_HoverNode)
        {
            if (element_.node_ == this)
            {
                element_.Reset();
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////

    bool consider_hover = element_.node_ ? element_.node_ == this : false;

    ////////////////////////////////////////////////////////////////////////////////

    if (element_.state_ != NodesState_Selected && element_.state_ != NodesState_DragingSelected && element_.state_ != NodesState_SelectingMore)
    {
        id_ = abs(id_); // remove "selected" flag
    }

    ////////////////////////////////////////////////////////////////////////////////

    bool consider_select = false;
    consider_select |= element_.state_ == NodesState_SelectingEmpty;
    consider_select |= element_.state_ == NodesState_SelectingValid;
    consider_select |= element_.state_ == NodesState_SelectingMore;

    if (consider_select)
    {
        bool select_it = false;

        ImRect node_rect(node_rect_min, node_rect_max);

        if (ImGui::GetIO().KeyCtrl)
        {
            select_it |= element_.rect_.Overlaps(node_rect);
        }
        else
        {
            select_it |= element_.rect_.Contains(node_rect);
        }

        consider_hover |= select_it;

        if (select_it && element_.state_ != NodesState_SelectingMore)
        {
            id_ = -abs(id_); // add "selected" flag
            element_.state_ = NodesState_SelectingValid;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////

    ImVec2 title_name_size = ImGui::CalcTextSize(name_.c_str());
    const float corner = title_name_size.y / 2.0f;

    {
        ImVec2 title_area;
        title_area.x = node_rect_max.x;
        title_area.y = node_rect_min.y + (title_name_size.y * 2.0f);

        ImVec2 title_pos;
        title_pos.x = node_rect_min.x + ((title_area.x - node_rect_min.x) / 2.0f) - (title_name_size.x / 2.0f);

        if (state_ > 0)
        {
            drawList->AddRectFilled(node_rect_min, node_rect_max, ImColor(0.25f, 0.25f, 0.25f, 0.9f), corner, ImDrawCornerFlags_All);
            drawList->AddRectFilled(node_rect_min, title_area, ImColor(0.25f, 0.0f, 0.125f, 0.9f), corner, ImDrawCornerFlags_Top);

            title_pos.y = node_rect_min.y + ((title_name_size.y * 2.0f) / 2.0f) - (title_name_size.y / 2.0f);
        }
        else
        {
            drawList->AddRectFilled(node_rect_min, node_rect_max, ImColor(0.25f, 0.0f, 0.125f, 0.9f), corner, ImDrawCornerFlags_All);

            title_pos.y = node_rect_min.y + ((node_rect_max.y - node_rect_min.y) / 2.0f) - (title_name_size.y / 2.0f);
        }

        ImGui::SetCursorScreenPos(title_pos);
        ImGui::Text("%s", name_.c_str());
    }

    ////////////////////////////////////////////////////////////////////////////////

    if (state_ > 0)
    {
        ////////////////////////////////////////////////////////////////////////////////

        for (auto &connection : inputs_)
        {
            if (connection->type_ == ConnectionType_None)
            {
                continue;
            }

            bool consider_io = false;

            ImVec2 input_name_size = ImGui::CalcTextSize(connection->name_.c_str());
            ImVec2 connection_pos = node_rect_min + (connection->position_ * canvas_scale_);

            {
                ImVec2 pos = connection_pos;
                pos += ImVec2(input_name_size.y * 0.75f, -input_name_size.y / 2.0f);

                ImGui::SetCursorScreenPos(pos);
                ImGui::Text("%s", connection->name_.c_str());
            }

            if (IsConnectorHovered(connection_pos, (input_name_size.y / 2.0f)))
            {
                consider_io |= element_.state_ == NodesState_Default;
                consider_io |= element_.state_ == NodesState_HoverConnection;
                consider_io |= element_.state_ == NodesState_HoverNode;

                // from nothing to hovered input
                if (consider_io)
                {
                    element_.Reset(NodesState_HoverIO);
                    element_.node_ = this;
                    element_.connection_ = connection.get();
                    element_.position_ = position_ + connection->position_;
                }

                // we could start draging input now
                if (ImGui::IsMouseClicked(0) && element_.connection_ == connection.get())
                {
                    element_.state_ = NodesState_DragingInput;

                    // remove connection from this input
                    if (connection->input_)
                    {
                        connection->input_->connections_--;
                    }

                    connection->target_ = nullptr;
                    connection->input_ = nullptr;
                    connection->connections_ = 0;
                }

                consider_io = true;
            }
            else if (element_.state_ == NodesState_HoverIO && element_.connection_ == connection.get())
            {
                element_.Reset(); // we are not hovering this last hovered input anymore
            }

            ////////////////////////////////////////////////////////////////////////////////

            ImColor color = ImColor(0.5f, 0.5f, 0.5f, 1.0f);

            if (connection->connections_ > 0)
            {
                drawList->AddCircleFilled(connection_pos, (input_name_size.y / 3.0f), color);
            }

            // currently we are dragin some output, check if there is a possibilty to connect here (this input)
            if (element_.state_ == NodesState_DragingOutput || element_.state_ == NodesState_DragingOutputValid)
            {
                // check is draging output are not from the same node
                if (element_.node_ != this && element_.connection_->type_ == connection->type_)
                {
                    color = ImColor(0.0f, 1.0f, 0.0f, 1.0f);

                    if (consider_io)
                    {
                        element_.state_ = NodesState_DragingOutputValid;
                        drawList->AddCircleFilled(connection_pos, (input_name_size.y / 3.0f), color);

                        if (!ImGui::IsMouseDown(0))
                        {
                            if (connection->input_)
                            {
                                connection->input_->connections_--;
                            }

                            connection->target_ = element_.node_;
                            connection->input_ = element_.connection_;
                            connection->connections_ = 1;
                            element_.connection_->connections_++;

                            element_.Reset(NodesState_HoverIO);
                            element_.node_ = this;
                            element_.connection_ = connection.get();
                            element_.position_ = node_rect_min + connection->position_;
                        }
                    }
                }
            }

            consider_io |= element_.state_ == NodesState_HoverIO;
            consider_io |= element_.state_ == NodesState_DragingInput;
            consider_io |= element_.state_ == NodesState_DragingInputValid;
            consider_io &= element_.connection_ == connection.get();

            if (consider_io)
            {
                color = ImColor(0.0f, 1.0f, 0.0f, 1.0f);

                if (element_.state_ != NodesState_HoverIO)
                {
                    drawList->AddCircleFilled(connection_pos, (input_name_size.y / 3.0f), color);
                }
            }

            drawList->AddCircle(connection_pos, (input_name_size.y / 3.0f), color, ((int)(6.0f * canvas_scale_) + 10), (1.5f * canvas_scale_));
        }

        ////////////////////////////////////////////////////////////////////////////////

        for (auto &connection : outputs_)
        {
            if (connection->type_ == ConnectionType_None)
            {
                continue;
            }

            bool consider_io = false;

            ImVec2 output_name_size = ImGui::CalcTextSize(connection->name_.c_str());
            ImVec2 connection_pos = node_rect_min + (connection->position_ * canvas_scale_);

            {
                ImVec2 pos = connection_pos;
                pos += ImVec2(-output_name_size.x - (output_name_size.y * 0.75f), -output_name_size.y / 2.0f);

                ImGui::SetCursorScreenPos(pos);
                ImGui::Text("%s", connection->name_.c_str());
            }

            if (IsConnectorHovered(connection_pos, (output_name_size.y / 2.0f)))
            {
                consider_io |= element_.state_ == NodesState_Default;
                consider_io |= element_.state_ == NodesState_HoverConnection;
                consider_io |= element_.state_ == NodesState_HoverNode;

                // from nothing to hovered output
                if (consider_io)
                {
                    element_.Reset(NodesState_HoverIO);
                    element_.node_ = this;
                    element_.connection_ = connection.get();
                    element_.position_ = position_ + connection->position_;
                }

                // we could start draging output now
                if (ImGui::IsMouseClicked(0) && element_.connection_ == connection.get())
                {
                    element_.state_ = NodesState_DragingOutput;
                }

                consider_io = true;
            }
            else if (element_.state_ == NodesState_HoverIO && element_.connection_ == connection.get())
            {
                element_.Reset(); // we are not hovering this last hovered output anymore
            }

            ////////////////////////////////////////////////////////////////////////////////

            ImColor color = ImColor(0.5f, 0.5f, 0.5f, 1.0f);

            if (connection->connections_ > 0)
            {
                drawList->AddCircleFilled(connection_pos, (output_name_size.y / 3.0f), ImColor(0.5f, 0.5f, 0.5f, 1.0f));
            }

            // currently we are dragin some input, check if there is a possibilty to connect here (this output)
            if (element_.state_ == NodesState_DragingInput || element_.state_ == NodesState_DragingInputValid)
            {
                // check is draging input are not from the same node
                if (element_.node_ != this && element_.connection_->type_ == connection->type_)
                {
                    color = ImColor(0.0f, 1.0f, 0.0f, 1.0f);

                    if (consider_io)
                    {
                        element_.state_ = NodesState_DragingInputValid;
                        drawList->AddCircleFilled(connection_pos, (output_name_size.y / 3.0f), color);

                        if (!ImGui::IsMouseDown(0))
                        {
                            element_.connection_->target_ = this;
                            element_.connection_->input_ = connection.get();
                            element_.connection_->connections_ = 1;

                            connection->connections_++;

                            element_.Reset(NodesState_HoverIO);
                            element_.node_ = this;
                            element_.connection_ = connection.get();
                            element_.position_ = node_rect_min + connection->position_;
                        }
                    }
                }
            }

            consider_io |= element_.state_ == NodesState_HoverIO;
            consider_io |= element_.state_ == NodesState_DragingOutput;
            consider_io |= element_.state_ == NodesState_DragingOutputValid;
            consider_io &= element_.connection_ == connection.get();

            if (consider_io)
            {
                color = ImColor(0.0f, 1.0f, 0.0f, 1.0f);

                if (element_.state_ != NodesState_HoverIO)
                {
                    drawList->AddCircleFilled(connection_pos, (output_name_size.y / 3.0f), color);
                }
            }

            drawList->AddCircle(connection_pos, (output_name_size.y / 3.0f), color, ((int)(6.0f * canvas_scale_) + 10), (1.5f * canvas_scale_));
        }

        ////////////////////////////////////////////////////////////////////////////////
    }

    ////////////////////////////////////////////////////////////////////////////////

    if ((consider_select && consider_hover) || (id_ < 0))
    {
        drawList->AddRectFilled(node_rect_min, node_rect_max, ImColor(1.0f, 1.0f, 1.0f, 0.25f), corner, ImDrawCornerFlags_All);
    }

    ImGui::EndGroup();
    ImGui::PopID();
}
} // namespace ChemiaAion
