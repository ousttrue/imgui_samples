
#include <any>
#include <cmath>
#include <imgui.h>
#include <imgui_internal.h>

#include "NodeGraph.hpp"

namespace
{
ImVec2 operator-(ImVec2 a, ImVec2 b) { return ImVec2(a.x - b.x, a.y - b.y); }
ImVec2 operator+(ImVec2 a, ImVec2 b) { return ImVec2(a.x + b.x, a.y + b.y); }
void operator+=(ImVec2 &a, ImVec2 b) { a = a + b; }
void operator-=(ImVec2 &a, ImVec2 b) { a = a - b; }

template <int n>
struct BezierWeights
{
    constexpr BezierWeights() : x_(), y_(), z_(), w_()
    {
        for (int i = 1; i <= n; ++i)
        {
            float t = (float)i / (float)(n + 1);
            float u = 1.0f - t;

            x_[i - 1] = u * u * u;
            y_[i - 1] = 3 * u * u * t;
            z_[i - 1] = 3 * u * t * t;
            w_[i - 1] = t * t * t;
        }
    }

    float x_[n];
    float y_[n];
    float z_[n];
    float w_[n];
};

static constexpr auto bezier_weights_ = BezierWeights<16>();

float ImVec2Dot(const ImVec2 &S1, const ImVec2 &S2)
{
    return (S1.x * S2.x + S1.y * S2.y);
}

float GetSquaredDistancePointSegment(const ImVec2 &P, const ImVec2 &S1, const ImVec2 &S2)
{
    const float l2 = (S1.x - S2.x) * (S1.x - S2.x) + (S1.y - S2.y) * (S1.y - S2.y);

    if (l2 < 1.0f)
    {
        return (P.x - S2.x) * (P.x - S2.x) + (P.y - S2.y) * (P.y - S2.y);
    }

    ImVec2 PS1(P.x - S1.x, P.y - S1.y);
    ImVec2 T(S2.x - S1.x, S2.y - S2.y);

    const float tf = ImVec2Dot(PS1, T) / l2;
    const float minTf = 1.0f < tf ? 1.0f : tf;
    const float t = 0.0f > minTf ? 0.0f : minTf;

    T.x = S1.x + T.x * t;
    T.y = S1.y + T.y * t;

    return (P.x - T.x) * (P.x - T.x) + (P.y - T.y) * (P.y - T.y);
}

float GetSquaredDistanceToBezierCurve(const ImVec2 &point, const ImVec2 &p1, const ImVec2 &p2, const ImVec2 &p3, const ImVec2 &p4)
{
    float minSquaredDistance = FLT_MAX;
    float tmp;

    ImVec2 L = p1;
    ImVec2 temp;

    for (int i = 1; i < 16 - 1; ++i)
    {
        const ImVec4 &W = ImVec4(bezier_weights_.x_[i], bezier_weights_.y_[i], bezier_weights_.z_[i], bezier_weights_.w_[i]);

        temp.x = W.x * p1.x + W.y * p2.x + W.z * p3.x + W.w * p4.x;
        temp.y = W.x * p1.y + W.y * p2.y + W.z * p3.y + W.w * p4.y;

        tmp = GetSquaredDistancePointSegment(point, L, temp);

        if (minSquaredDistance > tmp)
        {
            minSquaredDistance = tmp;
        }

        L = temp;
    }

    tmp = GetSquaredDistancePointSegment(point, L, p4);

    if (minSquaredDistance > tmp)
    {
        minSquaredDistance = tmp;
    }

    return minSquaredDistance;
}
} // namespace

namespace spacechase0
{
ImVec2 Node::getInputConnectorPos(ImVec2 base, int index)
{
    return base + position + ImVec2(5, 34) + ImVec2(0, index * 25);
}

ImVec2 Node::getOutputConnectorPos(ImVec2 base, int index)
{
    return base + position + ImVec2(300 - 20, 34) + ImVec2(0, index * 25);
}

void Graph::update()
{
    ImGui::BeginChild("Playground", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove);

    ImDrawList *draw = ImGui::GetWindowDrawList();

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 size = ImGui::GetWindowSize();
    ImVec2 mouse = ImGui::GetIO().MousePos;

    // Draw BG
    for (float x = fmodf(scroll.x, gridSize); x < size.x; x += gridSize)
        draw->AddLine(ImVec2(x, 0) + pos, ImVec2(x, size.y) + pos, gridColor);
    for (float y = fmodf(scroll.y, gridSize); y < size.y; y += gridSize)
        draw->AddLine(ImVec2(0, y) + pos, ImVec2(size.x, y) + pos, gridColor);

    bool clickedInSomething = false;
    bool dragging = false;

    // Draw nodes
    int n = 0;
    for (auto &node : nodes)
    {
        ImGui::PushID(n);
        const auto &type = types[node->type];

        ImVec2 nodePos = pos + scroll + node->position;
        ImVec2 nodeSize(300, 25 + (node->collapsed ? 0 : (std::max(node->inputs.size(), node->outputs.size()) * 25 + 10)));

        bool small = (node->collapsed || (std::max(node->inputs.size(), node->outputs.size()) == 0));

        // Handle selection, dragging, collapsing
        if (ImGui::IsMouseClicked(0) && !ImGui::IsMouseDown(1) && !ImGui::IsMouseDown(2))
        {
            if (ImRect(nodePos, nodePos + nodeSize).Contains(mouse))
                clickedInSomething = true;

            if (ImRect(nodePos, nodePos + ImVec2(nodeSize.x, 25)).Contains(mouse))
            {
                if (!ImGui::GetIO().KeyShift)
                {
                    deselectAll();
                }
                node->selected = true;

                if (ImGui::IsMouseDoubleClicked(0))
                    node->collapsed = !node->collapsed;
            }
        }
        if (ImGui::IsMouseDown(0) && node->selected)
        {
            dragging = true;
            node->position += ImGui::GetIO().MouseDelta;
        }

        // Draw node BG
        ImGui::BeginGroup();
        ImGui::SetCursorScreenPos(pos + scroll + node->position);
        if (node->selected)
        {
            draw->AddRect(nodePos, nodePos + nodeSize, ImColor(255, 255, 255, 255), 16, ImDrawCornerFlags_All, 4);
        }
        draw->AddRectFilled(nodePos, nodePos + nodeSize, ImColor(64, 64, 64, 200), 16, ImDrawCornerFlags_All);
        draw->AddRectFilled(nodePos, nodePos + ImVec2(nodeSize.x, 25), ImColor(0, 32, 64, 200), 16, small ? ImDrawCornerFlags_All : ImDrawCornerFlags_Top);
        ImGui::EndGroup();

        ImGui::SetCursorScreenPos(nodePos + ImVec2(7, 7));
        ImGui::Text(node->type.c_str());

        if (node->collapsed)
        {
            ImGui::PopID();
            ++n;
            continue;
        }

        int i = 0;
        for (auto &input : node->inputs)
        {
            ImVec2 connPos = node->getInputConnectorPos(pos + scroll, i);

            doPinCircle(draw, connPos, type.inputs[i].first, input.type() == typeid(Connection));
            if (ImRect(connPos, connPos + ImVec2(16, 16)).Contains(mouse))
            {
                if (!connSel && ImGui::IsMouseClicked(0))
                {
                    clickedInSomething = true;
                    deselectAll();
                    connSel.reset(new Connection{node.get(), i});
                    connSelInput = true;

                    if (input.type() == typeid(Connection))
                    {
                        auto other = std::any_cast<Connection>(input);
                        other.other->outputs[other.index] = std::any();
                    }
                    input = Connection();
                }
                else if (connSel && ImGui::IsMouseReleased(0) && !connSelInput && connSel->other != node.get() && type.inputs[i].first == types[connSel->other->type].outputs[connSel->index].first)
                {
                    input = (*connSel);
                    connSel->other->outputs[connSel->index] = Connection{node.get(), i};

                    connSel.reset();
                }
            }

            ImGui::SetCursorScreenPos(connPos + ImVec2(20, 0));

            ImGui::PushItemWidth(75);
            doPinValue((type.inputs[i].second + "##i" + std::to_string(i)).c_str(), type.inputs[i].first, input);
            ImGui::PopItemWidth();

            if (input.type() == typeid(Connection))
            {
                Connection conn = std::any_cast<Connection>(input);
                if (conn.other == nullptr)
                {
                    connPos += ImVec2(8, 8);
                    ImVec2 otherConnPos = mouse;
                    draw->AddBezierCurve(connPos, connPos + ImVec2(50, 0), otherConnPos + ImVec2(-50, 0), otherConnPos, getConnectorColor(type.inputs[i].first), 2);
                }
            }

            ++i;
        }

        i = 0;
        for (auto &output : node->outputs)
        {
            ImVec2 connPos = node->getOutputConnectorPos(pos + scroll, i);

            doPinCircle(draw, connPos, type.outputs[i].first, output.type() == typeid(Connection));
            if (ImRect(connPos, connPos + ImVec2(16, 16)).Contains(mouse))
            {
                if (!connSel && ImGui::IsMouseClicked(0))
                {
                    clickedInSomething = true;
                    deselectAll();
                    connSel.reset(new Connection{node.get(), i});
                    connSelInput = false;
                    if (output.type() == typeid(Connection))
                    {
                        auto other = std::any_cast<Connection>(output);
                        other.other->inputs[other.index] = std::any();
                    }
                    output = Connection();
                }
                else if (connSel && ImGui::IsMouseReleased(0) && connSelInput && connSel->other != node.get() && type.outputs[i].first == types[connSel->other->type].inputs[connSel->index].first)
                {
                    output = (*connSel);
                    connSel->other->inputs[connSel->index] = Connection{node.get(), i};

                    connSel.reset();
                }
            }

            ImGui::SetCursorScreenPos(connPos - ImVec2(90, 0) - ImVec2(ImGui::CalcTextSize(type.outputs[i].second.c_str()).x, 0));

            ImGui::PushItemWidth(75);
            doPinValue((type.outputs[i].second + "##o" + std::to_string(i)).c_str(), type.outputs[i].first, output);
            ImGui::PopItemWidth();

            // Draw connections
            if (output.type() == typeid(Connection))
            {
                Connection &conn = std::any_cast<Connection &>(output);

                connPos += ImVec2(8, 8);
                ImVec2 otherConnPos = conn.other == nullptr ? mouse : conn.other->getInputConnectorPos(pos + scroll, conn.index) + ImVec2(8, 8);

                if (GetSquaredDistanceToBezierCurve(mouse, connPos, connPos + ImVec2(50, 0), otherConnPos + ImVec2(-50, 0), otherConnPos) < 25 && ImGui::IsMouseClicked(0))
                {
                    if (!ImGui::GetIO().KeyShift)
                    {
                        deselectAll();
                    }
                    conn.selected = true;
                    clickedInSomething = true;
                }

                if (conn.selected)
                {
                    ImU32 invColor = getConnectorColor(type.outputs[i].first) ^ 0x00FFFFFF;
                    draw->AddBezierCurve(connPos, connPos + ImVec2(50, 0), otherConnPos + ImVec2(-50, 0), otherConnPos, invColor, 4);
                }
                draw->AddBezierCurve(connPos, connPos + ImVec2(50, 0), otherConnPos + ImVec2(-50, 0), otherConnPos, getConnectorColor(type.outputs[i].first), 2);
            }

            ++i;
        }

        ImGui::PopID();
        ++n;
    }

    if (ImGui::IsMouseClicked(0) && !clickedInSomething || ImGui::IsMouseClicked(1))
    {
        deselectAll();
        if (connSel)
        {
            if (connSelInput)
                connSel->other->inputs[connSel->index] = std::any();
            else
                connSel->other->outputs[connSel->index] = std::any();
        }
    }

    // Scrolling
    if (!dragging && !clickedInSomething && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
    {
        if (ImGui::IsMouseDragging(0, 6) && !ImGui::IsMouseDown(1) && !ImGui::IsMouseDown(2))
            scroll += ImGui::GetIO().MouseDelta;
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
                node->position = mouse - scroll;
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

void Graph::deletePressed()
{
    if (nodes.size() == 0)
        return;

    for (auto &node : nodes)
    {
        for (auto &input : node->inputs)
        {
            if (input.type() == typeid(Connection))
            {
                Connection conn = std::any_cast<Connection>(input);
                if (!conn.selected)
                    continue;

                conn.other->outputs[conn.index] = std::any();
                input = std::any();
            }
        }
        for (auto &output : node->outputs)
        {
            if (output.type() == typeid(Connection))
            {
                Connection conn = std::any_cast<Connection>(output);
                if (!conn.selected)
                    continue;

                conn.other->inputs[conn.index] = std::any();
                output = std::any();
            }
        }
    }

    for (int i = nodes.size() - 1; i >= 0; --i)
    {
        if (nodes[i]->selected)
        {
            for (auto &input : nodes[i]->inputs)
            {
                if (input.type() == typeid(Connection))
                {
                    Connection conn = std::any_cast<Connection>(input);
                    conn.other->outputs[conn.index] = std::any();
                    input = std::any();
                }
            }
            for (auto &output : nodes[i]->outputs)
            {
                if (output.type() == typeid(Connection))
                {
                    Connection conn = std::any_cast<Connection>(output);
                    conn.other->inputs[conn.index] = std::any();
                    output = std::any();
                }
            }
            nodes.erase(nodes.begin() + i);
        }
    }
}

void Graph::deselectAll()
{
    for (auto &other : nodes)
    {
        other->selected = false;
        for (auto &oin : other->inputs)
        {
            if (oin.type() == typeid(Connection))
            {
                std::any_cast<Connection &>(oin).selected = false;
            }
        }
        for (auto &oout : other->outputs)
        {
            if (oout.type() == typeid(Connection))
            {
                std::any_cast<Connection &>(oout).selected = false;
            }
        }
    }
}

ImU32 Graph::getConnectorColor(ConnectionType connType)
{
    switch (connType)
    {
    case ConnectionType::Sequence:
        return ImColor(200, 200, 200, 255);
    case ConnectionType::Int:
        return ImColor(255, 0, 0, 255);
    case ConnectionType::Float:
        return ImColor(0, 255, 0, 255);
    case ConnectionType::String:
        return ImColor(0, 0, 255, 255);
    case ConnectionType::Vector2:
        return ImColor(255, 255, 0, 255);
    }
}

void Graph::doPinCircle(ImDrawList *draw, ImVec2 pos, ConnectionType connType, bool filled)
{
    switch (connType)
    {
    case ConnectionType::Sequence:
        draw->AddTriangle(pos, pos + ImVec2(8, 8), pos + ImVec2(0, 16), getConnectorColor(connType), 1);
        if (filled)
            draw->AddTriangleFilled(pos + ImVec2(1, 4), pos + ImVec2(5, 8), pos + ImVec2(1, 12), getConnectorColor(connType));
        break;
    case ConnectionType::Int:
    case ConnectionType::Float:
    case ConnectionType::String:
    case ConnectionType::Vector2:
        draw->AddCircle(pos + ImVec2(8, 8), 8, getConnectorColor(connType));
        if (filled)
            draw->AddCircleFilled(pos + ImVec2(8, 8), 5, getConnectorColor(connType));
        break;
    }
}

void Graph::doPinValue(const std::string &label, ConnectionType connType, std::any &input)
{
    switch (connType)
    {
    case ConnectionType::Sequence:
    {
        ImGui::Text((label.substr(0, label.find("##"))).c_str());
    }
    break;
    case ConnectionType::Int:
    {
        if (!input.has_value())
            input = 0;
        if (input.type() != typeid(int))
        {
            ImGui::Text((label.substr(0, label.find("##"))).c_str());
            return;
        }
        int val = std::any_cast<int>(input);
        ImGui::InputInt(label.c_str(), &val, 0, 0);
        input = val;
    }
    break;
    case ConnectionType::Float:
    {
        if (!input.has_value())
            input = 0.f;
        if (input.type() != typeid(float))
        {
            ImGui::Text((label.substr(0, label.find("##"))).c_str());
            return;
        }
        float val = std::any_cast<float>(input);
        ImGui::InputFloat(label.c_str(), &val, 0, 0);
        input = val;
    }
    break;
    case ConnectionType::String:
    {
        if (!input.has_value())
            input = std::string();
        if (input.type() != typeid(std::string))
        {
            ImGui::Text((label.substr(0, label.find("##"))).c_str());
            return;
        }
        std::string val = std::any_cast<std::string>(input);
        val.resize(1024, '\0');
        ImGui::InputText(label.c_str(), &val[0], 1024);
        input = std::string(val.c_str());
    }
    break;
    case ConnectionType::Vector2:
    {
        if (!input.has_value())
            input = ImVec2(0, 0);
        if (input.type() != typeid(ImVec2))
        {
            ImGui::Text((label.substr(0, label.find("##"))).c_str());
            return;
        }
        ImVec2 val = std::any_cast<ImVec2>(input);
        ImGui::InputFloat2(label.c_str(), &val.x);
        input = val;
    }
    break;
    }
}

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
} // namespace NodeGraph
