#pragma once
#include "mouse_state.h"
#include <array>

struct OrbitCamera
{
    float zNear = 0.1f;
    float zFar = 10.0f;
    float fovYDegrees = 30.0f;
    float aspectRatio = 1.0f;
    std::array<float, 16> projection;

    int screenWidth = 1;
    int screenHeight = 1;

    int prevMouseX = -1;
    int prevMouseY = -1;

    float shiftX = 0;
    float shiftY = 0;
    float shiftZ = 2.0f;
    float yawRadians = 0;
    float pitchRadians = 0;
    std::array<float, 16> view;

    std::array<float, 16> viewProjection;

    OrbitCamera()
    {
        CalcView();
        CalcPerspective();
        CalcViewProjection();
    }
    void CalcViewProjection();
    void CalcView();
    void CalcPerspective();
    void SetScreenSize(int w, int h);
    void MouseInput(const MouseState &mouse);
};
