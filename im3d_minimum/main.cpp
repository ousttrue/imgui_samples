#include "win32_window.h"


int main(int argc, char **argv)
{
    Win32Window window;

    if(!window.Create(640, 480, L"im3d_minimum"))
    {
        return 1;
    }

    while(true)
    {
        if(!window.IsRunning()){
            break;
        }
    }

    return 0;
}
