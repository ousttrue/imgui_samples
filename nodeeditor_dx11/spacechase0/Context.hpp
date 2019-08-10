#pragma once
#include <memory>

namespace spacechase0
{

struct Connection;
struct Context
{
    bool clickedInSomething = false;
    bool dragging = false;
    std::unique_ptr<Connection> connSel;
    bool connSelInput = false;

    void NewFrame()
    {
        clickedInSomething = false;
        dragging = false;
    }

    void clear()
    {
        if (connSel)
        {
            if (connSelInput)
                connSel->other->inputs[connSel->index] = std::any();
            else
                connSel->other->outputs[connSel->index] = std::any();
        }
    }

    void deselectAll(const std::vector<std::unique_ptr<Node>> &nodes)
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
};

} // namespace spacechase0