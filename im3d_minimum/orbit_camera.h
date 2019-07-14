#pragma once

struct OrbitCamera
{
    float zNear = 0.1f;
    float zFar = 10.0f;
    float fovYDegrees = 30.0f;
    float aspectRatio = 1.0f;
    float projection[16];

    int screenWidth = 1;
    int screenHeight = 1;

    OrbitCamera()
    {
        CalcPerspective();
    }
    void CalcPerspective();
    void SetScreenSize(int w, int h);
};
