#include "node.h"
#include "context.h"

const ImVec2 NODE_WINDOW_PADDING(8.0f, 8.0f);
const float NODE_SLOT_RADIUS = 4.0f;

namespace edon
{
Node::Node(int id, const char *name, const std::array<float, 2> &pos, float value, const ImVec4 &color, int inputs_count, int outputs_count)
    : m_id(id), m_name(name), m_pos(pos)
{
    Value = value;
    Color = color;
    InputsCount = inputs_count;
    OutputsCount = outputs_count;
}

ImColor Node::GetBGColor(const Context &context, int node_selected) const
{
    if (context.IsHovered(m_id) || (context.node_hovered_in_list == -1 && node_selected == m_id))
    {
        return IM_COL32(75, 75, 75, 255);
    }
    else
    {
        return IM_COL32(60, 60, 60, 255);
    }
}

void Node::DrawLeftPanel(int *node_selected, Context *context)
{
    ImGui::PushID(m_id);
    if (ImGui::Selectable(m_name.c_str(), m_id == *node_selected))
    {
        *node_selected = m_id;
    }
    if (ImGui::IsItemHovered())
    {
        context->node_hovered_in_list = m_id;
        (context->open_context_menu) |= ImGui::IsMouseClicked(1);
    }
    ImGui::PopID();
}

ImVec2 Node::GetInputSlotPos(int slot_no, float scaling) const
{
    return ImVec2(m_pos[0] * scaling, m_pos[1] * scaling + m_size[1] * ((float)slot_no + 1) / ((float)InputsCount + 1));
}

ImVec2 Node::GetOutputSlotPos(int slot_no, float scaling) const
{
    return ImVec2(m_pos[0] * scaling + m_size[0], m_pos[1] * scaling + m_size[1] * ((float)slot_no + 1) / ((float)OutputsCount + 1));
}

void Node::Process(ImDrawList *draw_list, const ImVec2 &offset, Context *context, int *node_selected, float scaling)
{
    // Node *node = &nodes[node_idx];
    ImGui::PushID(m_id);
    ImVec2 node_rect_min = offset + *(ImVec2 *)&m_pos * scaling;

    // Display node contents first
    draw_list->ChannelsSetCurrent(1); // Foreground
    bool old_any_active = ImGui::IsAnyItemActive();
    ImGui::SetCursorScreenPos(node_rect_min + NODE_WINDOW_PADDING);
    ImGui::BeginGroup(); // Lock horizontal position
    ImGui::Text("%s", m_name.c_str());
    ImGui::SliderFloat("##value", &Value, 0.0f, 1.0f, "Alpha %.2f");
    ImGui::ColorEdit3("##color", &Color.x);
    ImGui::EndGroup();

    // Save the size of what we have emitted and whether any of the widgets are being used
    bool node_widgets_active = (!old_any_active && ImGui::IsAnyItemActive());
    auto size = ImGui::GetItemRectSize() + NODE_WINDOW_PADDING + NODE_WINDOW_PADDING;
    m_size[0] = size.x;
    m_size[1] = size.y;
    ImVec2 node_rect_max = node_rect_min + size;

    // Display node box
    draw_list->ChannelsSetCurrent(0); // Background
    ImGui::SetCursorScreenPos(node_rect_min);
    ImGui::InvisibleButton("node", size);
    if (ImGui::IsItemHovered())
    {
        context->node_hovered_in_scene = m_id;
        context->open_context_menu |= ImGui::IsMouseClicked(1);
    }
    bool node_moving_active = ImGui::IsItemActive();
    if (node_widgets_active || node_moving_active)
        *node_selected = m_id;
    if (node_moving_active && ImGui::IsMouseDragging(0))
    {
        m_pos[0] += ImGui::GetIO().MouseDelta[0] / scaling;
        m_pos[1] += ImGui::GetIO().MouseDelta[1] / scaling;
    }

    ImU32 node_bg_color = GetBGColor(*context, *node_selected);
    draw_list->AddRectFilled(node_rect_min, node_rect_max, node_bg_color, 4.0f);
    draw_list->AddRect(node_rect_min, node_rect_max, IM_COL32(100, 100, 100, 255), 4.0f);
    for (int slot_idx = 0; slot_idx < InputsCount; slot_idx++)
    {
        draw_list->AddCircleFilled(offset + GetInputSlotPos(slot_idx, scaling), NODE_SLOT_RADIUS, IM_COL32(150, 150, 150, 150));
    }
    for (int slot_idx = 0; slot_idx < OutputsCount; slot_idx++)
    {
        draw_list->AddCircleFilled(offset + GetOutputSlotPos(slot_idx, scaling), NODE_SLOT_RADIUS, IM_COL32(150, 150, 150, 150));
    }

    ImGui::PopID();
}

} // namespace edon