#pragma once

enum class ButtonFlags : int
{
    None = 0,
    Left = 1,
    Right = 2,
    Middle = 4,
};

struct MouseState
{
    int X = 0;
    int Y = 0;
    int Wheel = 0;
    ButtonFlags Buttons = ButtonFlags::None;

    void Down(ButtonFlags button)
    {
        Buttons = static_cast<ButtonFlags>((int)Buttons | (int)button);
    }
    void Up(ButtonFlags button)
    {
        Buttons = static_cast<ButtonFlags>((int)Buttons & ~(int)button);
    }
    bool IsDown(ButtonFlags button) const
    {
        return (int)Buttons & (int)button;
    }
};
