#include "node.h"
#include "context.h"

const ImVec2 NODE_WINDOW_PADDING(8.0f, 8.0f);
const float NODE_SLOT_RADIUS = 4.0f;

namespace edon
{
Node::Node(int id, const char *name, const ImVec2 &pos, float value, const ImVec4 &color, int inputs_count, int outputs_count)
{
    ID = id;
    strncpy(Name, name, 31);
    Name[31] = 0;
    Pos = pos;
    Value = value;
    Color = color;
    InputsCount = inputs_count;
    OutputsCount = outputs_count;
}

ImColor Node::GetBGColor(const Context &context, int node_selected) const
{
    if (context.IsHovered(ID) || (context.node_hovered_in_list == -1 && node_selected == ID))
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
    ImGui::PushID(ID);
    if (ImGui::Selectable(Name, ID == *node_selected))
    {
        *node_selected = ID;
    }
    if (ImGui::IsItemHovered())
    {
        context->node_hovered_in_list = ID;
        (context->open_context_menu) |= ImGui::IsMouseClicked(1);
    }
    ImGui::PopID();
}

ImVec2 Node::GetInputSlotPos(int slot_no, float scaling) const
{
    return ImVec2(Pos.x * scaling, Pos.y * scaling + Size.y * ((float)slot_no + 1) / ((float)InputsCount + 1));
}

ImVec2 Node::GetOutputSlotPos(int slot_no, float scaling) const
{
    return ImVec2(Pos.x * scaling + Size.x, Pos.y * scaling + Size.y * ((float)slot_no + 1) / ((float)OutputsCount + 1));
}

void Node::Process(ImDrawList *draw_list, const ImVec2 &offset, Context *context, int *node_selected, float scaling)
{
    // Node *node = &nodes[node_idx];
    ImGui::PushID(ID);
    ImVec2 node_rect_min = offset + Pos * scaling;

    // Display node contents first
    draw_list->ChannelsSetCurrent(1); // Foreground
    bool old_any_active = ImGui::IsAnyItemActive();
    ImGui::SetCursorScreenPos(node_rect_min + NODE_WINDOW_PADDING);
    ImGui::BeginGroup(); // Lock horizontal position
    ImGui::Text("%s", Name);
    ImGui::SliderFloat("##value", &Value, 0.0f, 1.0f, "Alpha %.2f");
    ImGui::ColorEdit3("##color", &Color.x);
    ImGui::EndGroup();

    // Save the size of what we have emitted and whether any of the widgets are being used
    bool node_widgets_active = (!old_any_active && ImGui::IsAnyItemActive());
    Size = ImGui::GetItemRectSize() + NODE_WINDOW_PADDING + NODE_WINDOW_PADDING;
    ImVec2 node_rect_max = node_rect_min + Size;

    // Display node box
    draw_list->ChannelsSetCurrent(0); // Background
    ImGui::SetCursorScreenPos(node_rect_min);
    ImGui::InvisibleButton("node", Size);
    if (ImGui::IsItemHovered())
    {
        context->node_hovered_in_scene = ID;
        context->open_context_menu |= ImGui::IsMouseClicked(1);
    }
    bool node_moving_active = ImGui::IsItemActive();
    if (node_widgets_active || node_moving_active)
        *node_selected = ID;
    if (node_moving_active && ImGui::IsMouseDragging(0))
    {
        Pos = Pos + ImGui::GetIO().MouseDelta / scaling;
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