#pragma once

namespace camera
{
struct CameraState;
}
class Im3dGuiImpl;
class Im3dGui
{
    Im3dGuiImpl *m_impl = nullptr;

public:
    Im3dGui();
    ~Im3dGui();
    void NewFrame(const camera::CameraState *camera, int x, int y, float deltaTime);
    void Manipulate(float world[16]);
    void Draw(const float *viewProjection, int w, int h);
};
