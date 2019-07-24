#pragma once
#include <imgui.h>

namespace ChemiaAion
{

float GetSquaredDistancePointSegment(const ImVec2 &P, const ImVec2 &S1, const ImVec2 &S2);
float GetSquaredDistanceToBezierCurve(const ImVec2 &point, const ImVec2 &p1, const ImVec2 &p2, const ImVec2 &p3, const ImVec2 &p4);

} // namespace ChemiaAion