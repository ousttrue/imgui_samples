#pragma once
#include "mouse_state.h"

struct WindowState
{
    int Width;
    int Height;
    float elapsedSeconds;
    float deltaSeconds;
    MouseState Mouse;
};
