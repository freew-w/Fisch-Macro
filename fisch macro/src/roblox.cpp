#include "pch.h"
#include "roblox.h"

Roblox::Roblox()
{
    robloxHWnd_ = FindWindowW(nullptr, L"Roblox");
    if (!robloxHWnd_)
        throw std::runtime_error("Roblox window not found\nMake sure Roblox is running");
}
