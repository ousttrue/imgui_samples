#pragma once
#include <stdint.h>

namespace screenstate
{
enum MouseButtonFlags
{
    None,
    LeftDown = 0x01,
    RightDown = 0x02,
    MiddleDown = 0x04,
    WheelPlus = 0x08,
    WheelMinus = 0x10,
    CursorUpdate = 0x20,
};

struct ScreenState
{
    int16_t Width;
    int16_t Height;
    // uint32_t Time;
    float ElapsedSeconds;
    float DeltaSeconds;
    int16_t MouseX;
    int16_t MouseY;
    MouseButtonFlags MouseFlag;

    bool Has(MouseButtonFlags flag) const
    {
        return (MouseFlag & flag) != 0;
    }

    void Set(MouseButtonFlags flag)
    {
        MouseFlag = (MouseButtonFlags)(MouseFlag | flag);
    }

    void Unset(MouseButtonFlags flag)
    {
        MouseFlag = (MouseButtonFlags)(MouseFlag & ~flag);
    }

    void Clear()
    {
        Unset((MouseButtonFlags)(MouseButtonFlags::WheelMinus | MouseButtonFlags::WheelPlus | MouseButtonFlags::CursorUpdate));
    }

    float AspectRatio() const
    {
        if (Height == 0)
        {
            return 1;
        }
        return (float)Width / Height;
    }

    bool HasCapture() const
    {
        return Has(MouseButtonFlags::LeftDown) || Has(MouseButtonFlags::RightDown) || Has(MouseButtonFlags::MiddleDown);
    }

    bool SameSize(const ScreenState &rhs) const
    {
        return Width == rhs.Width && Height == rhs.Height;
    }

    // float DeltaSeconds(const ScreenState &rhs) const
    // {
    //     auto delta = Time - rhs.Time;
    //     if (delta == 0)
    //     {
    //         return 0.001f;
    //     }
    //     return 0.001f * delta;
    // }

    ScreenState Crop(int x, int y, int w, int h) const
    {
        auto state = *this;
        state.Width = w;
        state.Height = h;
        state.MouseX -= x;
        state.MouseY -= y;
        return state;
    }
};
static_assert(sizeof(ScreenState) == 20, "sizeof(WindowMouseState)");
} // namespace screenstate
