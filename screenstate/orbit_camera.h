#pragma once
#include "ScreenState.h"
#include "camera_state.h"
#include <array>


struct OrbitCamera
{
    camera::CameraState state;

    float zNear = 0.1f;
    float zFar = 100.0f;
    float aspectRatio = 1.0f;

    int prevMouseX = -1;
    int prevMouseY = -1;

    float shiftX = 0;
    float shiftY = 0;
    float shiftZ = 2.0f;
    float yawRadians = 0;
    float pitchRadians = 0;

    OrbitCamera()
    {
        CalcView();
        CalcPerspective();
        state.CalcViewProjection();
    }
    void CalcView();
    void CalcPerspective();
    void SetViewport(int x, int y, int w, int h);
    void WindowInput(const screenstate::ScreenState &window);
};
