#pragma once

struct WindowState;
namespace camera
{
struct CameraState;
}

void Im3d_Impl_NewFrame(const camera::CameraState *camera, const WindowState *window);
