#include "imgui_node_graph_test.h"
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

const float NODE_SLOT_RADIUS = 4.0f;
const ImVec2 NODE_WINDOW_PADDING(8.0f, 8.0f);

struct Context
{
    bool open_context_menu = false;
    int node_hovered_in_list = -1;
    int node_hovered_in_scene = -1;

    bool IsHovered(int ID) const
    {
        return node_hovered_in_list == ID || node_hovered_in_scene == ID;
    }
};

// Dummy
struct Node
{
    int ID;
    char Name[32];
    ImVec2 Pos, Size;
    float Value;
    ImVec4 Color;
    int InputsCount, OutputsCount;

    Node(int id, const char *name, const ImVec2 &pos, float value, const ImVec4 &color, int inputs_count, int outputs_count)
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

    ImColor GetBGColor(const Context &context, int node_selected) const
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

    void DrawLeftPanel(int *node_selected, Context *context)
    {
        // Node *node = &nodes[node_idx];
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

    ImVec2 GetInputSlotPos(int slot_no) const
    {
        return ImVec2(Pos.x, Pos.y + Size.y * ((float)slot_no + 1) / ((float)InputsCount + 1));
    }
    ImVec2 GetOutputSlotPos(int slot_no) const
    {
        return ImVec2(Pos.x + Size.x, Pos.y + Size.y * ((float)slot_no + 1) / ((float)OutputsCount + 1));
    }

    void Draw(ImDrawList *draw_list, const ImVec2 &offset, Context *context, int *node_selected)
    {
        // Node *node = &nodes[node_idx];
        ImGui::PushID(ID);
        ImVec2 node_rect_min = offset + Pos;

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
            Pos = Pos + ImGui::GetIO().MouseDelta;

        ImU32 node_bg_color = GetBGColor(*context, *node_selected);
        draw_list->AddRectFilled(node_rect_min, node_rect_max, node_bg_color, 4.0f);
        draw_list->AddRect(node_rect_min, node_rect_max, IM_COL32(100, 100, 100, 255), 4.0f);
        for (int slot_idx = 0; slot_idx < InputsCount; slot_idx++)
            draw_list->AddCircleFilled(offset + GetInputSlotPos(slot_idx), NODE_SLOT_RADIUS, IM_COL32(150, 150, 150, 150));
        for (int slot_idx = 0; slot_idx < OutputsCount; slot_idx++)
            draw_list->AddCircleFilled(offset + GetOutputSlotPos(slot_idx), NODE_SLOT_RADIUS, IM_COL32(150, 150, 150, 150));

        ImGui::PopID();
    }
};

struct NodeLink
{
    int InputIdx, InputSlot, OutputIdx, OutputSlot;

    NodeLink(int input_idx, int input_slot, int output_idx, int output_slot)
    {
        InputIdx = input_idx;
        InputSlot = input_slot;
        OutputIdx = output_idx;
        OutputSlot = output_slot;
    }
};

// Creating a node graph editor for ImGui
// Quick demo, not production code! This is more of a demo of how to use ImGui to create custom stuff.
// Better version by @daniel_collin here https://gist.github.com/emoon/b8ff4b4ce4f1b43e79f2
// See https://github.com/ocornut/imgui/issues/306
// v0.03: fixed grid offset issue, inverted sign of 'scrolling'
// Animated gif: https://cloud.githubusercontent.com/assets/8225057/9472357/c0263c04-4b4c-11e5-9fdf-2cd4f33f6582.gif

#include <math.h> // fmodf

class Nodes
{
    ImVector<Node> nodes;
    ImVector<NodeLink> links;
    ImVec2 scrolling = ImVec2(0.0f, 0.0f);
    bool m_show_grid = true;
    int node_selected = -1;

public:
    Nodes()
    {
        nodes.push_back(Node(0, "MainTex", ImVec2(40, 50), 0.5f, ImColor(255, 100, 100), 1, 1));
        nodes.push_back(Node(1, "BumpMap", ImVec2(40, 150), 0.42f, ImColor(200, 100, 200), 1, 1));
        nodes.push_back(Node(2, "Combine", ImVec2(270, 80), 1.0f, ImColor(0, 200, 100), 2, 2));
        links.push_back(NodeLink(0, 0, 2, 0));
        links.push_back(NodeLink(1, 0, 2, 1));
    }

    void ShowLeftPanel(Context *context)
    {
        // Draw a list of nodes on the left side
        ImGui::BeginChild("node_list", ImVec2(100, 0));
        ImGui::Text("Nodes");
        ImGui::Separator();

        for (int node_idx = 0; node_idx < nodes.Size; node_idx++)
        {
            nodes[node_idx].DrawLeftPanel(&node_selected, context);
        }
        ImGui::EndChild();
    }

    void DrawGrid(ImDrawList *draw_list)
    {
        ImU32 GRID_COLOR = IM_COL32(200, 200, 200, 40);
        float GRID_SZ = 64.0f;
        ImVec2 win_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_sz = ImGui::GetWindowSize();
        for (float x = fmodf(scrolling.x, GRID_SZ); x < canvas_sz.x; x += GRID_SZ)
            draw_list->AddLine(ImVec2(x, 0.0f) + win_pos, ImVec2(x, canvas_sz.y) + win_pos, GRID_COLOR);
        for (float y = fmodf(scrolling.y, GRID_SZ); y < canvas_sz.y; y += GRID_SZ)
            draw_list->AddLine(ImVec2(0.0f, y) + win_pos, ImVec2(canvas_sz.x, y) + win_pos, GRID_COLOR);
    }

    void Show()
    {
        Context context;

        ShowLeftPanel(&context);

        ImGui::SameLine();
        ImGui::BeginGroup();

        // right header
        ImGui::Text("Hold middle mouse button to scroll (%.2f,%.2f)", scrolling.x, scrolling.y);
        ImGui::SameLine(ImGui::GetWindowWidth() - 100);
        ImGui::Checkbox("Show grid", &m_show_grid);

        // Create our child canvas
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, IM_COL32(60, 60, 70, 200));
        ImGui::BeginChild("scrolling_region", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
        ImGui::PushItemWidth(120.0f);

        ImVec2 offset = ImGui::GetCursorScreenPos() + scrolling;
        ImDrawList *draw_list = ImGui::GetWindowDrawList();

        // Display grid
        if (m_show_grid)
        {
            DrawGrid(draw_list);
        }

        // Display links
        draw_list->ChannelsSplit(2);
        draw_list->ChannelsSetCurrent(0); // Background
        for (int link_idx = 0; link_idx < links.Size; link_idx++)
        {
            NodeLink *link = &links[link_idx];
            Node *node_inp = &nodes[link->InputIdx];
            Node *node_out = &nodes[link->OutputIdx];
            ImVec2 p1 = offset + node_inp->GetOutputSlotPos(link->InputSlot);
            ImVec2 p2 = offset + node_out->GetInputSlotPos(link->OutputSlot);
            draw_list->AddBezierCurve(p1, p1 + ImVec2(+50, 0), p2 + ImVec2(-50, 0), p2, IM_COL32(200, 200, 100, 255), 3.0f);
        }

        // Display nodes
        for (int node_idx = 0; node_idx < nodes.Size; node_idx++)
        {
            nodes[node_idx].Draw(draw_list, offset, &context, &node_selected);
        }
        draw_list->ChannelsMerge();

        // Open context menu
        if (!ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1))
        {
            node_selected = context.node_hovered_in_list = context.node_hovered_in_scene = -1;
            context.open_context_menu = true;
        }
        if (context.open_context_menu)
        {
            ImGui::OpenPopup("context_menu");
            if (context.node_hovered_in_list != -1)
                node_selected = context.node_hovered_in_list;
            if (context.node_hovered_in_scene != -1)
                node_selected = context.node_hovered_in_scene;
        }

        // Draw context menu
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        if (ImGui::BeginPopup("context_menu"))
        {
            Node *node = node_selected != -1 ? &nodes[node_selected] : NULL;
            ImVec2 scene_pos = ImGui::GetMousePosOnOpeningCurrentPopup() - offset;
            if (node)
            {
                ImGui::Text("Node '%s'", node->Name);
                ImGui::Separator();
                if (ImGui::MenuItem("Rename..", NULL, false, false))
                {
                }
                if (ImGui::MenuItem("Delete", NULL, false, false))
                {
                }
                if (ImGui::MenuItem("Copy", NULL, false, false))
                {
                }
            }
            else
            {
                if (ImGui::MenuItem("Add"))
                {
                    nodes.push_back(Node(nodes.Size, "New node", scene_pos, 0.5f, ImColor(100, 100, 200), 2, 2));
                }
                if (ImGui::MenuItem("Paste", NULL, false, false))
                {
                }
            }
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();

        // Scrolling
        if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(2, 0.0f))
            scrolling = scrolling + ImGui::GetIO().MouseDelta;

        ImGui::PopItemWidth();
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
        ImGui::EndGroup();
    }
};

// Really dumb data structure provided for the example.
// Note that we storing links are INDICES (not ID) to make example code shorter, obviously a bad idea for any general purpose code.
void ShowExampleAppCustomNodeGraph(bool *opened)
{
    static Nodes s_nodes;

    // ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiSetCond_FirstUseEver);
    if (ImGui::Begin("Example: Custom Node Graph", opened))
    {
        s_nodes.Show();
    }
    ImGui::End();
}
