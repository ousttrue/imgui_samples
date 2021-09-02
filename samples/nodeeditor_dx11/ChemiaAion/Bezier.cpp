#include "Bezier.h"

static float ImVec2Dot(const ImVec2 &S1, const ImVec2 &S2)
{
    return (S1.x * S2.x + S1.y * S2.y);
}

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

static constexpr auto bezier_weights_ = BezierWeights<16>();

namespace ChemiaAion
{

float GetSquaredDistancePointSegment(const ImVec2 &P, const ImVec2 &S1, const ImVec2 &S2)
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

float GetSquaredDistanceToBezierCurve(const ImVec2 &point, const ImVec2 &p1, const ImVec2 &p2, const ImVec2 &p3, const ImVec2 &p4)
{
    float minSquaredDistance = FLT_MAX;
    float tmp;

    ImVec2 L = p1;
    ImVec2 temp;

    for (int i = 1; i < 16 - 1; ++i)
    {
        const ImVec4 &W = ImVec4(bezier_weights_.x_[i], bezier_weights_.y_[i], bezier_weights_.z_[i], bezier_weights_.w_[i]);

        temp.x = W.x * p1.x + W.y * p2.x + W.z * p3.x + W.w * p4.x;
        temp.y = W.x * p1.y + W.y * p2.y + W.z * p3.y + W.w * p4.y;

        tmp = GetSquaredDistancePointSegment(point, L, temp);

        if (minSquaredDistance > tmp)
        {
            minSquaredDistance = tmp;
        }

        L = temp;
    }

    tmp = GetSquaredDistancePointSegment(point, L, p4);

    if (minSquaredDistance > tmp)
    {
        minSquaredDistance = tmp;
    }

    return minSquaredDistance;
}

} // namespace ChemiaAion