#define IMGUI_DEFINE_MATH_OPERATORS
#include "Node.h"
#include <algorithm>
#include <imgui_internal.h>

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
} // namespace ChemiaAion
