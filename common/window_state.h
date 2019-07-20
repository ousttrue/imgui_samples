#pragma once
#include "mouse_state.h"

struct WindowState
{
    int Width;
    int Height;
    float ElapsedSeconds;
    float DeltaSeconds;
    MouseState Mouse;
};
