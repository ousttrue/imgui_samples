#pragma once
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <string>
#include <array>

namespace edon
{

struct Context;
struct Node
{
    int m_id;
    std::string m_name;
    std::array<float, 2> m_pos;
    std::array<float, 2> m_size;

    float Value;
    ImVec4 Color;
    int InputsCount, OutputsCount;

    Node(int id, const char *name, const std::array<float, 2> &pos, float value, const ImVec4 &color, int inputs_count, int outputs_count);

    ImColor GetBGColor(const Context &context, int node_selected) const;

    void DrawLeftPanel(int *node_selected, Context *context);

    ImVec2 GetInputSlotPos(int slot_no, float scaling) const;
    ImVec2 GetOutputSlotPos(int slot_no, float scaling) const;

    void Process(ImDrawList *draw_list, const ImVec2 &offset, Context *context, int *node_selected, float scaling);
};

} // namespace edon