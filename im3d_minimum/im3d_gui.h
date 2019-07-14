#pragma once

struct MouseState;
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
    void NewFrame(const camera::CameraState *camera, const MouseState *mouse, float deltaTime);
    void Manipulate(float world[16]);
    void Draw(const float *viewProjection);
};
