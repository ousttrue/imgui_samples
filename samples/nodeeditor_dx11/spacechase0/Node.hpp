#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <any>
#include <imgui.h>

namespace spacechase0
{

enum ConnectionType
{
    Sequence,
    Int,
    Float,
    String,
    Vector2,
};
inline ImColor getConnectorColor(ConnectionType connType)
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

struct NodeType
{
    std::vector<std::pair<ConnectionType, std::string>> inputs;
    std::vector<std::pair<ConnectionType, std::string>> outputs;
    bool canUserCreate = true;
};

struct Context;
struct Node
{
private:
    ImVec2 position;
    std::string type;
    bool collapsed = false;

public:
    bool selected = false;

    std::vector<std::any> inputs;
    std::vector<std::any> outputs;

    void Draw(Context *context, ImDrawList *draw, int n, const NodeType &nodeType, const ImVec2 &offset, const ImVec2 &mouse,
              const std::vector<std::unique_ptr<Node>> &nodes,
              const std::unordered_map<std::string, NodeType> &types);

private:
    void DrawContent(Context *context, ImDrawList *draw, int n, const NodeType &nodeType, const ImVec2 &offset, const ImVec2 &mouse,
                     const std::vector<std::unique_ptr<Node>> &nodes,
                     const std::unordered_map<std::string, NodeType> &types);
    void doPinCircle(ImDrawList *draw, ImVec2 pos, ConnectionType connType, bool filled);
    void doPinValue(const std::string &label, ConnectionType connType, std::any &input);
    friend class Graph;

    ImVec2 getInputConnectorPos(ImVec2 base, int index);

    ImVec2 getOutputConnectorPos(ImVec2 base, int index);
};

struct Connection
{
    Node *other = nullptr;
    int index = 0;
    bool selected = false;
};

} // namespace spacechase0