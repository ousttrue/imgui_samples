// Prototype of standalone node graph editor for ImGui
// Thread: https://github.com/ocornut/imgui/issues/306
//
// This is based on code by:
// @emoon https://gist.github.com/emoon/b8ff4b4ce4f1b43e79f2
// @ocornut https://gist.github.com/ocornut/7e9b3ec566a333d725d4
// @flix01 https://github.com/Flix01/imgui/blob/b248df2df98af13d4b7dbb70c92430afc47a038a/addons/imguinodegrapheditor/imguinodegrapheditor.cpp#L432

#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include "imgui_internal.h"

#include <memory>
#include <string>
#include <vector>
#include <algorithm>

namespace ChemiaAion
{
class NodesImpl;
class Nodes final
{
    NodesImpl *m_impl = nullptr;

public:
    Nodes();
    ~Nodes();

    void ProcessNodes();
};
} // namespace ImGui