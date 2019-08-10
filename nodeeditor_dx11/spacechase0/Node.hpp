#pragma once
#include <vector>
#include <unordered_map>
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

struct NodeType
{
    std::vector<std::pair<ConnectionType, std::string>> inputs;
    std::vector<std::pair<ConnectionType, std::string>> outputs;
    bool canUserCreate = true;
};

struct Node
{
public:
    ImVec2 position;

    std::string type;

    std::vector<std::any> inputs;
    std::vector<std::any> outputs;

    bool collapsed = false;
    bool selected = false;

private:
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