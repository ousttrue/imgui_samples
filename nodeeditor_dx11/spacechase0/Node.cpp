#include "Node.hpp"
#include "Utils.hpp"
#include "Context.hpp"
#include <algorithm>
#include <imgui_internal.h>

#include "Utils.hpp"
static constexpr auto bezier_weights_ = BezierWeights<16>();
float GetSquaredDistanceToBezierCurve(const ImVec2 &point, const ImVec2 &p1, const ImVec2 &p2, const ImVec2 &p3, const ImVec2 &p4)
{
    float minSquaredDistance = FLT_MAX;
    ImVec2 L = p1;

    for (int i = 1; i < 16 - 1; ++i)
    {
        const ImVec4 &W = ImVec4(bezier_weights_.x_[i], bezier_weights_.y_[i], bezier_weights_.z_[i], bezier_weights_.w_[i]);

        ImVec2 temp;
        temp.x = W.x * p1.x + W.y * p2.x + W.z * p3.x + W.w * p4.x;
        temp.y = W.x * p1.y + W.y * p2.y + W.z * p3.y + W.w * p4.y;

        float tmp = GetSquaredDistancePointSegment(point, L, temp);

        if (minSquaredDistance > tmp)
        {
            minSquaredDistance = tmp;
        }

        L = temp;
    }

    {
        auto tmp = GetSquaredDistancePointSegment(point, L, p4);
        if (minSquaredDistance > tmp)
        {
            minSquaredDistance = tmp;
        }
    }

    return minSquaredDistance;
}
namespace spacechase0
{

ImVec2 Node::getInputConnectorPos(ImVec2 base, int index)
{
    return base + position + ImVec2(5, 34) + ImVec2(0, (float)(index * 25));
}

ImVec2 Node::getOutputConnectorPos(ImVec2 base, int index)
{
    return base + position + ImVec2(300 - 20, 34) + ImVec2(0, (float)(index * 25));
}

void Node::Draw(Context *context, ImDrawList *draw, int n, const NodeType &nodeType, const ImVec2 &offset, const ImVec2 &mouse,
                const std::vector<std::unique_ptr<Node>> &nodes,
                const std::unordered_map<std::string, NodeType> &types)
{
    ImGui::PushID(n);

    ImVec2 nodePos = offset + position;
    ImVec2 nodeSize(300, 25 + (collapsed
                                   ? 0
                                   : (float)(std::max(inputs.size(), outputs.size()) * 25 + 10)));

    // Handle selection, dragging, collapsing
    if (ImGui::IsMouseClicked(0) && !ImGui::IsMouseDown(1) && !ImGui::IsMouseDown(2))
    {
        // left click
        if (ImRect(nodePos, nodePos + nodeSize).Contains(mouse))
        {
            // click node
            context->clickedInSomething = true;
        }

        if (ImRect(nodePos, nodePos + ImVec2(nodeSize.x, 25)).Contains(mouse))
        {
            // click node header
            if (!ImGui::GetIO().KeyShift)
            {
                context->deselectAll(nodes);
            }
            selected = true;

            if (ImGui::IsMouseDoubleClicked(0))
            {
                collapsed = !collapsed;
            }
        }
    }

    if (ImGui::IsMouseDown(0) && selected)
    {
        // drag move
        context->dragging = true;
        position += ImGui::GetIO().MouseDelta;
    }

    // Draw node BG
    // ImGui::BeginGroup();
    // ImGui::SetCursorScreenPos(offset + position);
    if (selected)
    {
        draw->AddRect(nodePos, nodePos + nodeSize, ImColor(255, 255, 255, 255), 16, ImDrawCornerFlags_All, 4);
    }
    draw->AddRectFilled(nodePos, nodePos + nodeSize, ImColor(64, 64, 64, 200), 16, ImDrawCornerFlags_All);

    bool small = (collapsed || (std::max(inputs.size(), outputs.size()) == 0));
    draw->AddRectFilled(nodePos, nodePos + ImVec2(nodeSize.x, 25), ImColor(0, 32, 64, 200), 16,
                        small
                            ? ImDrawCornerFlags_All
                            : ImDrawCornerFlags_Top);
    // ImGui::EndGroup();

    // header
    ImGui::SetCursorScreenPos(nodePos + ImVec2(7, 7));
    ImGui::Text(type.c_str());

    if (!collapsed)
    {
        DrawContent(context, draw, n, nodeType, offset, mouse, nodes, types);
    }

    ImGui::PopID();
}

void Node::DrawContent(Context *context, ImDrawList *draw, int n, const NodeType &nodeType, const ImVec2 &offset, const ImVec2 &mouse,
                       const std::vector<std::unique_ptr<Node>> &nodes,
                       const std::unordered_map<std::string, NodeType> &types)
{
    int i = 0;
    for (auto &input : inputs)
    {
        ImVec2 connPos = getInputConnectorPos(offset, i);

        doPinCircle(draw, connPos, nodeType.inputs[i].first, input.type() == typeid(Connection));
        if (ImRect(connPos, connPos + ImVec2(16, 16)).Contains(mouse))
        {
            if (!context->connSel && ImGui::IsMouseClicked(0))
            {
                context->clickedInSomething = true;
                context->deselectAll(nodes);
                context->connSel.reset(new Connection{this, i});
                context->connSelInput = true;

                if (input.type() == typeid(Connection))
                {
                    auto other = std::any_cast<Connection>(input);
                    other.other->outputs[other.index] = std::any();
                }
                input = Connection();
            }
            else if (context->connSel &&
                     ImGui::IsMouseReleased(0) &&
                     !context->connSelInput &&
                     context->connSel->other != this)
            {
                auto found = types.find(context->connSel->other->type);
                if (found != types.end())
                {
                    if (nodeType.inputs[i].first == found->second.outputs[context->connSel->index].first)
                    {
                        input = (*context->connSel);
                        context->connSel->other->outputs[context->connSel->index] = Connection{this, i};
                        context->connSel.reset();
                    }
                }
            }
        }

        ImGui::SetCursorScreenPos(connPos + ImVec2(20, 0));

        ImGui::PushItemWidth(75);
        doPinValue((nodeType.inputs[i].second + "##i" + std::to_string(i)).c_str(), nodeType.inputs[i].first, input);
        ImGui::PopItemWidth();

        if (input.type() == typeid(Connection))
        {
            Connection conn = std::any_cast<Connection>(input);
            if (conn.other == nullptr)
            {
                // マウスでドラッグ
                connPos += ImVec2(8, 8);
                ImVec2 otherConnPos = mouse;
                draw->AddBezierCurve(connPos, connPos + ImVec2(50, 0),
                                     otherConnPos + ImVec2(-50, 0), otherConnPos,
                                     getConnectorColor(nodeType.inputs[i].first), 2);
            }
        }

        ++i;
    }

    i = 0;
    for (auto &output : outputs)
    {
        ImVec2 connPos = getOutputConnectorPos(offset, i);

        doPinCircle(draw, connPos, nodeType.outputs[i].first, output.type() == typeid(Connection));
        if (ImRect(connPos, connPos + ImVec2(16, 16)).Contains(mouse))
        {
            if (!context->connSel && ImGui::IsMouseClicked(0))
            {
                context->clickedInSomething = true;
                context->deselectAll(nodes);
                context->connSel.reset(new Connection{this, i});
                context->connSelInput = false;
                if (output.type() == typeid(Connection))
                {
                    auto other = std::any_cast<Connection>(output);
                    other.other->inputs[other.index] = std::any();
                }
                output = Connection();
            }
            else if (context->connSel && ImGui::IsMouseReleased(0) && context->connSelInput && context->connSel->other != this)
            {
                auto found = types.find(context->connSel->other->type);
                if (found != types.end())
                {
                    if (nodeType.outputs[i].first == found->second.inputs[context->connSel->index].first)
                    {
                        output = (*context->connSel);
                        context->connSel->other->inputs[context->connSel->index] = Connection{this, i};

                        context->connSel.reset();
                    }
                }
            }
        }

        ImGui::SetCursorScreenPos(connPos - ImVec2(90, 0) - ImVec2(ImGui::CalcTextSize(nodeType.outputs[i].second.c_str()).x, 0));

        ImGui::PushItemWidth(75);
        doPinValue((nodeType.outputs[i].second + "##o" + std::to_string(i)).c_str(), nodeType.outputs[i].first, output);
        ImGui::PopItemWidth();

        // Draw connections
        if (output.type() == typeid(Connection))
        {
            Connection &conn = std::any_cast<Connection &>(output);

            connPos += ImVec2(8, 8);
            ImVec2 otherConnPos = conn.other == nullptr ? mouse : conn.other->getInputConnectorPos(offset, conn.index) + ImVec2(8, 8);

            if (GetSquaredDistanceToBezierCurve(mouse,
                                                connPos,
                                                connPos + ImVec2(50, 0),
                                                otherConnPos + ImVec2(-50, 0),
                                                otherConnPos) < 25 &&
                ImGui::IsMouseClicked(0))
            {
                if (!ImGui::GetIO().KeyShift)
                {
                    context->deselectAll(nodes);
                }
                conn.selected = true;
                context->clickedInSomething = true;
            }

            if (conn.selected)
            {
                ImU32 invColor = getConnectorColor(nodeType.outputs[i].first) ^ 0x00FFFFFF;
                draw->AddBezierCurve(connPos, connPos + ImVec2(50, 0), otherConnPos + ImVec2(-50, 0), otherConnPos, invColor, 4);
            }
            draw->AddBezierCurve(
                connPos,
                connPos + ImVec2(50, 0),
                otherConnPos + ImVec2(-50, 0),
                otherConnPos,
                getConnectorColor(nodeType.outputs[i].first), 2);
        }

        ++i;
    }
}

void Node::doPinCircle(ImDrawList *draw, ImVec2 pos, ConnectionType connType, bool filled)
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

void Node::doPinValue(const std::string &label, ConnectionType connType, std::any &input)
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
} // namespace spacechase0