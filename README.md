# imgui samples

IMGUI の練習

## dependencies

* https://github.com/john-chapman/im3d
  * glew
* https://github.com/nlohmann/json
* https://github.com/SergiusTheBest/plog

## im3d_minimum

Minimum sample without imgui.
Define `IM3D_VERTEX_ALIGNMENT=16` is very important.

Application is separated 3 parts.

* Window and 3D API. Window back buffer, size and mouse state.
* 3D scene and camera.
* Im3d. Combine window size, mouse state, scene and camera state.
