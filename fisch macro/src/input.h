#pragma once
#include "pch.h"

namespace input
{
    enum class MouseButton : int
    {
        left = MOUSEEVENTF_LEFTDOWN,
        right = MOUSEEVENTF_RIGHTDOWN
    };

    inline void pressKey(int key)
    {
        INPUT input{ .type = INPUT_KEYBOARD, .ki{0, static_cast<WORD>(MapVirtualKey(key, MAPVK_VK_TO_VSC)), KEYEVENTF_SCANCODE, 0, 0} };
        SendInput(1, &input, sizeof(INPUT));
    }

    inline void releaseKey(int key)
    {
        INPUT input{ .type = INPUT_KEYBOARD, .ki{0, static_cast<WORD>(MapVirtualKey(key, MAPVK_VK_TO_VSC)), KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP, 0, 0} };
        SendInput(1, &input, sizeof(INPUT));
    }

    inline void clickKey(int key)
    {
        INPUT input[2]{ {.type = INPUT_KEYBOARD, .ki{0, static_cast<WORD>(MapVirtualKey(key, MAPVK_VK_TO_VSC)), KEYEVENTF_SCANCODE, 0, 0} }, {.type = INPUT_KEYBOARD, .ki{0, static_cast<WORD>(MapVirtualKey(key, MAPVK_VK_TO_VSC)), KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP, 0, 0} } };
        SendInput(2, input, sizeof(INPUT));
    }

    inline void pressMouse(MouseButton button)
    {
        INPUT input{ .type = INPUT_MOUSE, .mi{0, 0, 0, static_cast<DWORD>(button), 0, 0} };
        SendInput(1, &input, sizeof(INPUT));
    }

    inline void releaseMouse(MouseButton button)
    {
        INPUT input{ .type = INPUT_MOUSE, .mi{0, 0, 0, static_cast<DWORD>(button == MouseButton::left ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP), 0, 0} };
        SendInput(1, &input, sizeof(INPUT));
    }

    inline void clickMouse(MouseButton button)
    {
        INPUT input[2]{ {.type = INPUT_MOUSE, .mi{0, 0, 0, static_cast<DWORD>(button), 0, 0} }, {.type = INPUT_MOUSE, .mi{0, 0, 0, static_cast<DWORD>(button == MouseButton::left ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP), 0, 0} } };
        SendInput(2, input, sizeof(INPUT));
    }

    inline void moveMouse(int x, int y, bool absolute = true)
    {
        if (absolute)
            SetCursorPos(x, y);
        INPUT input = { .type = INPUT_MOUSE, .mi{ absolute ? 1 : x, absolute ? 0 : y, 0, static_cast<DWORD>(MOUSEEVENTF_MOVE), 0, 0} };
        SendInput(1, &input, sizeof(INPUT));
    }

    inline void rotateMouseWheel(int delta)
    {
        INPUT input{ .type = INPUT_MOUSE, .mi{0, 0, static_cast<DWORD>(delta), MOUSEEVENTF_WHEEL, 0, 0} };
        SendInput(1, &input, sizeof(INPUT));
    }
}
