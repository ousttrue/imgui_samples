#pragma once
#include <imgui.h>

template <int n>
struct BezierWeights
{
    constexpr BezierWeights() : x_(), y_(), z_(), w_()
    {
        for (int i = 1; i <= n; ++i)
        {
            float t = (float)i / (float)(n + 1);
            float u = 1.0f - t;

            x_[i - 1] = u * u * u;
            y_[i - 1] = 3 * u * u * t;
            z_[i - 1] = 3 * u * t * t;
            w_[i - 1] = t * t * t;
        }
    }

    float x_[n];
    float y_[n];
    float z_[n];
    float w_[n];
};

inline ImVec2 operator-(ImVec2 a, ImVec2 b) { return ImVec2(a.x - b.x, a.y - b.y); }
inline ImVec2 operator+(ImVec2 a, ImVec2 b) { return ImVec2(a.x + b.x, a.y + b.y); }
inline void operator+=(ImVec2 &a, ImVec2 b) { a = a + b; }
inline void operator-=(ImVec2 &a, ImVec2 b) { a = a - b; }

inline float ImVec2Dot(const ImVec2 &S1, const ImVec2 &S2)
{
    return (S1.x * S2.x + S1.y * S2.y);
}

inline float GetSquaredDistancePointSegment(const ImVec2 &P, const ImVec2 &S1, const ImVec2 &S2)
{
    const float l2 = (S1.x - S2.x) * (S1.x - S2.x) + (S1.y - S2.y) * (S1.y - S2.y);

    if (l2 < 1.0f)
    {
        return (P.x - S2.x) * (P.x - S2.x) + (P.y - S2.y) * (P.y - S2.y);
    }

    ImVec2 PS1(P.x - S1.x, P.y - S1.y);
    ImVec2 T(S2.x - S1.x, S2.y - S2.y);

    const float tf = ImVec2Dot(PS1, T) / l2;
    const float minTf = 1.0f < tf ? 1.0f : tf;
    const float t = 0.0f > minTf ? 0.0f : minTf;

    T.x = S1.x + T.x * t;
    T.y = S1.y + T.y * t;

    return (P.x - T.x) * (P.x - T.x) + (P.y - T.y) * (P.y - T.y);
}


