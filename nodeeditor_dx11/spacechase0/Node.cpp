#include "Node.hpp"
#include "Utils.hpp"

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

} // namespace spacechase0