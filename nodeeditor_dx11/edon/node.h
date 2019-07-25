#pragma once
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

namespace edon
{

struct Context;
struct Node
{
    int ID;
    char Name[32];
    ImVec2 Pos, Size;
    float Value;
    ImVec4 Color;
    int InputsCount, OutputsCount;

    Node(int id, const char *name, const ImVec2 &pos, float value, const ImVec4 &color, int inputs_count, int outputs_count);

    ImColor GetBGColor(const Context &context, int node_selected) const;

    void DrawLeftPanel(int *node_selected, Context *context);

    ImVec2 GetInputSlotPos(int slot_no, float scaling) const;
    ImVec2 GetOutputSlotPos(int slot_no, float scaling) const;

    void Process(ImDrawList *draw_list, const ImVec2 &offset, Context *context, int *node_selected, float scaling);
};

} // namespace edon