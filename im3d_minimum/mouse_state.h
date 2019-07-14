#pragma once

enum class ButtonFlags : int
{
    Left = 1,
    Right = 2,
    Middle = 4,
};

struct MouseState
{
    int X;
    int Y;
    int Wheel;
    ButtonFlags Buttons;
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
