#include "imgui_node_graph_test.h"
#include "context.h"
#include "node.h"
#include <imgui.h>
#include <plog/Log.h>
#include <vector>

const float MIN_SCALING = 0.3f;
const float MAX_SCALING = 2.0f;

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

namespace edon
{
class Nodes
{
    std::vector<Node> m_nodes;
    std::vector<NodeLink> m_links;
    ImVec2 m_scrolling = ImVec2(0.0f, 0.0f);
    float m_scaling = 1.0f;
    bool m_show_grid = true;
    int m_node_selected = -1;

public:
    Nodes()
    {
        m_nodes.push_back(Node(0, "MainTex", ImVec2(40, 50), 0.5f, ImColor(255, 100, 100), 1, 1));
        m_nodes.push_back(Node(1, "BumpMap", ImVec2(40, 150), 0.42f, ImColor(200, 100, 200), 1, 1));
        m_nodes.push_back(Node(2, "Combine", ImVec2(270, 80), 1.0f, ImColor(0, 200, 100), 2, 2));
        m_links.push_back(NodeLink(0, 0, 2, 0));
        m_links.push_back(NodeLink(1, 0, 2, 1));
    }

    void ShowLeftPanel(Context *context)
    {
        // Draw a list of nodes on the left side
        ImGui::BeginChild("node_list", ImVec2(100, 0));
        ImGui::Text("Nodes");
        ImGui::Separator();

        for (auto &node : m_nodes)
        {
            node.DrawLeftPanel(&m_node_selected, context);
        }
        ImGui::EndChild();
    }

    void DrawGrid(ImDrawList *draw_list, const ImVec2 &_scrolling)
    {
        ImU32 GRID_COLOR = IM_COL32(200, 200, 200, 40);
        float GRID_SZ = 64.0f * m_scaling;
        ImVec2 win_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_sz = ImGui::GetWindowSize();
        auto scrolling = _scrolling;
        for (float x = fmodf(scrolling.x, GRID_SZ); x < canvas_sz.x; x += GRID_SZ)
        {
            draw_list->AddLine(ImVec2(x, 0.0f) + win_pos, ImVec2(x, canvas_sz.y) + win_pos, GRID_COLOR);
        }
        for (float y = fmodf(scrolling.y, GRID_SZ); y < canvas_sz.y; y += GRID_SZ)
        {
            draw_list->AddLine(ImVec2(0.0f, y) + win_pos, ImVec2(canvas_sz.x, y) + win_pos, GRID_COLOR);
        }
    }

    void ContextMenu(Context *context, const ImVec2 &offset)
    {
        if (!ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1))
        {
            m_node_selected = context->node_hovered_in_list = context->node_hovered_in_scene = -1;
            context->open_context_menu = true;
        }
        if (context->open_context_menu)
        {
            ImGui::OpenPopup("context_menu");
            if (context->node_hovered_in_list != -1)
                m_node_selected = context->node_hovered_in_list;
            if (context->node_hovered_in_scene != -1)
                m_node_selected = context->node_hovered_in_scene;
        }

        // Draw context menu
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        if (ImGui::BeginPopup("context_menu"))
        {
            Node *node = m_node_selected != -1 ? &m_nodes[m_node_selected] : NULL;
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
                    m_nodes.push_back(Node((int)m_nodes.size(), "New node", scene_pos, 0.5f, ImColor(100, 100, 200), 2, 2));
                }
                if (ImGui::MenuItem("Paste", NULL, false, false))
                {
                }
            }
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
    }

    void ShowRightPanelHeader()
    {
        // right header
        ImGui::Text("Hold middle mouse button to scroll (%.2f,%.2f)", m_scrolling.x, m_scrolling.y);

        ImGui::SameLine();
        ImGui::Checkbox("Show grid", &m_show_grid);

        ImGui::SameLine();
        ImGui::PushItemWidth(-100);
        ImGui::SliderFloat("zoom", &m_scaling, MIN_SCALING, MAX_SCALING, "%0.2f");
    }

    ///
    /// Canvas
    ///
    void ShowRightPanelCanvas(Context *context)
    {
        // スクロールを加味したcanvasの原点
        ImVec2 offset = ImGui::GetCursorScreenPos() + m_scrolling;

        ImDrawList *draw_list = ImGui::GetWindowDrawList();

        // Display grid
        if (m_show_grid)
        {
            DrawGrid(draw_list, m_scrolling);
        }

        {
            draw_list->ChannelsSplit(2);
            draw_list->ChannelsSetCurrent(0); // Background
            // Display links
            for (auto &link : m_links)
            {
                Node *node_inp = &m_nodes[link.InputIdx];
                Node *node_out = &m_nodes[link.OutputIdx];
                ImVec2 p1 = offset + node_inp->GetOutputSlotPos(link.InputSlot, m_scaling);
                ImVec2 p2 = offset + node_out->GetInputSlotPos(link.OutputSlot, m_scaling);
                draw_list->AddBezierCurve(p1, p1 + ImVec2(+50, 0), p2 + ImVec2(-50, 0), p2, IM_COL32(200, 200, 100, 255), 3.0f * m_scaling);
            }

            // Display nodes
            for (auto &node : m_nodes)
            {
                // move, draw
                node.Process(draw_list, offset, context, &m_node_selected, m_scaling);
            }
            draw_list->ChannelsMerge();
        }

        // Open context menu
        ContextMenu(context, offset);
        // Scrolling
        if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive())
        {
            if (ImGui::IsMouseDragging(2, 0.0f))
            {
                m_scrolling = m_scrolling + ImGui::GetIO().MouseDelta;
            }
            auto io = ImGui::GetIO();
            if (io.MouseWheel > 0)
            {
                //m_scaling *= 1.25f;
                m_scaling += 0.1f;
            }
            else if (io.MouseWheel < 0)
            {
                //m_scaling *= 0.8f;
                m_scaling -= 0.1f;
            }
            m_scaling = std::clamp(m_scaling, MIN_SCALING, MAX_SCALING);
        }
    }

    void ShowRightPanel(Context *context)
    {
        ShowRightPanelHeader();

        // Create our child canvas
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, IM_COL32(60, 60, 70, 200));
        ImGui::BeginChild("scrolling_region", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
        ImGui::PushItemWidth(120.0f * m_scaling);

        auto backup = ImGui::GetStyle();
        ImGui::GetStyle().ScaleAllSizes(m_scaling);
        ImGui::SetWindowFontScale(m_scaling);
        ShowRightPanelCanvas(context);
        ImGui::SetWindowFontScale(1.0f);
        ImGui::GetStyle() = backup;

        ImGui::PopItemWidth();
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }

    void Show()
    {
        Context context;

        ShowLeftPanel(&context);

        ImGui::SameLine();
        ImGui::BeginGroup();
        ShowRightPanel(&context);
        ImGui::EndGroup();
    }
};
} // namespace edon

// Really dumb data structure provided for the example.
// Note that we storing links are INDICES (not ID) to make example code shorter, obviously a bad idea for any general purpose code.
void ShowExampleAppCustomNodeGraph(bool *opened)
{
    static edon::Nodes s_nodes;

    // ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiSetCond_FirstUseEver);
    if (ImGui::Begin("Example: Custom Node Graph", opened))
    {
        s_nodes.Show();
    }
    ImGui::End();
}
