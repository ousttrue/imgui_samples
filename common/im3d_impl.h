#pragma once

namespace screenstate
{
struct ScreenState;
}
namespace camera
{
struct CameraState;
}

void Im3d_Impl_NewFrame(const camera::CameraState *camera, const screenstate::ScreenState *window);
