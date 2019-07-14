#include "win32_window.h"
#include "wgl_context.h"

int main(int argc, char **argv)
{
    Win32Window window;
    if (!window.Create(640, 480, L"im3d_minimum"))
    {
        return 1;
    }

    WGLContext wgl;
    if (!wgl.Create(window.GetHandle(), 3, 0))
    {
        return 2;
    }

    while (window.IsRunning())
    {
    }

    return 0;
}
