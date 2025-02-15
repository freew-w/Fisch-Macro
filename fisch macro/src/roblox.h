#pragma once

class Roblox
{
public:
    inline static Roblox& getInstance() { static Roblox roblox{}; return roblox; }

    inline const HWND& getRobloxHWnd() const { return robloxHWnd_; }

    inline POINT getRobloxWindowPosition() const { POINT point{}; ClientToScreen(robloxHWnd_, &point); return point; }
    inline POINT getRobloxWindowSize() const { RECT rect; GetClientRect(robloxHWnd_, &rect); return { rect.right, rect.bottom }; }

private:
    HWND robloxHWnd_{};

    Roblox();
};
