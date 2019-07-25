#pragma once

namespace edon
{

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

} // namespace edon