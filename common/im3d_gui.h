#pragma once

struct MouseState;
namespace camera
{
struct CameraState;
}

void Im3d_Impl_NewFrame(const camera::CameraState *camera, const MouseState *mouse, float deltaTime);
