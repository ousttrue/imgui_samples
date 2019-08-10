
#include <any>
#include <cmath>
#include <iostream>
#include <imgui.h>
#include <imgui_internal.h>

#include "NodeGraph.hpp"
#include "Utils.hpp"

static void DrawGrid(ImDrawList *draw, const ImVec2 &pos, const ImVec2 &size,
                     const ImVec2 &scroll)
{
    ImU32 gridColor = ImColor(128, 128, 128, 32);
    float gridSize = 64;

    for (float x = fmodf(scroll.x, gridSize); x < size.x; x += gridSize)
        draw->AddLine(ImVec2(x, 0) + pos, ImVec2(x, size.y) + pos, gridColor);
    for (float y = fmodf(scroll.y, gridSize); y < size.y; y += gridSize)
        draw->AddLine(ImVec2(0, y) + pos, ImVec2(size.x, y) + pos, gridColor);
}

namespace spacechase0
{

void Graph::update()
{
    ImGui::BeginChild("Playground", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove);

    ImDrawList *draw = ImGui::GetWindowDrawList();

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 size = ImGui::GetWindowSize();
    ImVec2 mouse = ImGui::GetIO().MousePos;

    m_context.NewFrame();

    DrawGrid(draw, pos, size, m_scroll);

    // Draw nodes
    int n = 0;
    auto offset = pos + m_scroll;
    for (auto &node : nodes)
    {
        node->Draw(&m_context, draw, n++, types[node->type], offset, mouse, nodes, types);
    }

    if ((ImGui::IsMouseClicked(0) && !m_context.clickedInSomething) || ImGui::IsMouseClicked(1))
    {
        // 何もないところを左クリック
        // 右クリック
        m_context.deselectAll(nodes);
        m_context.clear();
    }

    // Scrolling
    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDown(2))
    {
        m_scroll += ImGui::GetIO().MouseDelta;
    }

    // Right click menu
    if (!ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseReleased(1))
        ImGui::OpenPopup("PlaygroundContextMenu");

    if (ImGui::BeginPopup("PlaygroundContextMenu"))
    {
        for (const auto &type_ : types)
        {
            const auto &type = type_.second;
            if (type.canUserCreate && ImGui::MenuItem(type_.first.c_str()))
            {
                auto node = std::make_unique<Node>();
                node->position = mouse - m_scroll;
                node->type = type_.first;
                node->inputs.resize(type.inputs.size());
                node->outputs.resize(type.outputs.size());
                nodes.push_back(std::move(node));
            }
        }
        ImGui::EndPopup();
    }

    ImGui::EndChild();
}

// void Graph::deletePressed()
// {
//     if (nodes.size() == 0)
//         return;

//     for (auto &node : nodes)
//     {
//         for (auto &input : node->inputs)
//         {
//             if (input.type() == typeid(Connection))
//             {
//                 Connection conn = std::any_cast<Connection>(input);
//                 if (!conn.selected)
//                     continue;

//                 conn.other->outputs[conn.index] = std::any();
//                 input = std::any();
//             }
//         }
//         for (auto &output : node->outputs)
//         {
//             if (output.type() == typeid(Connection))
//             {
//                 Connection conn = std::any_cast<Connection>(output);
//                 if (!conn.selected)
//                     continue;

//                 conn.other->inputs[conn.index] = std::any();
//                 output = std::any();
//             }
//         }
//     }

//     for (int i = (int)nodes.size() - 1; i >= 0; --i)
//     {
//         if (nodes[i]->selected)
//         {
//             for (auto &input : nodes[i]->inputs)
//             {
//                 if (input.type() == typeid(Connection))
//                 {
//                     Connection conn = std::any_cast<Connection>(input);
//                     conn.other->outputs[conn.index] = std::any();
//                     input = std::any();
//                 }
//             }
//             for (auto &output : nodes[i]->outputs)
//             {
//                 if (output.type() == typeid(Connection))
//                 {
//                     Connection conn = std::any_cast<Connection>(output);
//                     conn.other->inputs[conn.index] = std::any();
//                     output = std::any();
//                 }
//             }
//             nodes.erase(nodes.begin() + i);
//         }
//     }
// }

Graph::Graph()
{
    types.insert(std::make_pair<std::string, NodeType>("Start", {{}, {{ConnectionType::Sequence, ""}}, false}));
    types.insert(std::make_pair<std::string, NodeType>("Nop", {{{ConnectionType::Sequence, ""}}, {{ConnectionType::Sequence, ""}}}));
    types.insert(std::make_pair<std::string, NodeType>("Print", {{{ConnectionType::Sequence, ""}, {ConnectionType::String, "String"}}, {{ConnectionType::Sequence, ""}}}));
    types.insert(std::make_pair<std::string, NodeType>("Concat", {{{ConnectionType::String, "String 1"}, {ConnectionType::String, "String 2"}}, {{ConnectionType::String, "Output"}}}));
    types.insert(std::make_pair<std::string, NodeType>("Int ToString", {{{ConnectionType::Int, "Input"}}, {{ConnectionType::String, "Output"}}}));
    types.insert(std::make_pair<std::string, NodeType>("Float ToString", {{{ConnectionType::Float, "Input"}}, {{ConnectionType::String, "Output"}}}));
    types.insert(std::make_pair<std::string, NodeType>("Split Vec2", {{{ConnectionType::Vector2, "Input"}}, {{ConnectionType::Int, "X"}, {ConnectionType::Int, "Y"}}}));
    auto startNode = std::make_unique<Node>();
    startNode->type = "Start";
    startNode->outputs.resize(1);

    auto floatNode = std::make_unique<Node>();
    floatNode->type = "Float ToString";
    floatNode->inputs.resize(1);
    floatNode->inputs[0] = 3.14f;
    floatNode->outputs.resize(1);
    floatNode->position = ImVec2(100, 150);

    auto printNode = std::make_unique<Node>();
    printNode->type = "Print";
    printNode->inputs.resize(2);
    printNode->outputs.resize(1);
    printNode->position = ImVec2(400, 0);

    Connection conn1o;
    conn1o.other = startNode.get();
    conn1o.index = 0;
    printNode->inputs[0] = conn1o;
    Connection conn1i;
    conn1i.other = printNode.get();
    conn1i.index = 0;
    startNode->outputs[0] = conn1i;

    Connection conn2o;
    conn2o.other = floatNode.get();
    conn2o.index = 0;
    printNode->inputs[1] = conn2o;
    Connection conn2i;
    conn2i.other = printNode.get();
    conn2i.index = 1;
    floatNode->outputs[0] = conn2i;
    floatNode->selected = true;

    nodes.push_back(std::move(startNode));
    nodes.push_back(std::move(floatNode));
    nodes.push_back(std::move(printNode));
}

} // namespace spacechase0
