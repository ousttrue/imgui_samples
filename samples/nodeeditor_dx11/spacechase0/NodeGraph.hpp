#ifndef NODEGRAPH_HPP
#define NODEGRAPH_HPP

#include <any>
#include <imgui.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include "Node.hpp"
#include "Context.hpp"

namespace spacechase0
{

class Graph
{
public:
    Graph();
    std::vector<std::unique_ptr<Node>> nodes;
    std::unordered_map<std::string, NodeType> types;

    void update();
    // void deletePressed();

private:
    Context m_context;
    ImVec2 m_scroll = ImVec2(0, 0);
};

float GetSquaredDistanceToBezierCurve(const ImVec2 &point, const ImVec2 &p1, const ImVec2 &p2, const ImVec2 &p3, const ImVec2 &p4);

} // namespace spacechase0

#endif // NODEGRAPH_HPP
