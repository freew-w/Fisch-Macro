#include "pch.hpp"
#include "fisch.hpp"

Fisch::Fisch()
{
    executablePath_ = std::filesystem::current_path();
    robloxHWnd_ = FindWindowW(nullptr, L"Roblox");
    GetClientRect(robloxHWnd_, &robloxClientRect_);
    ClientToScreen(robloxHWnd_, &robloxClientToWindowPoint_);

    if (!robloxHWnd_)
    {
        MessageBoxW(nullptr, L"Roblox not found\nExiting now", L"Error", MB_ICONERROR);
        exit(EXIT_FAILURE);
    }
}
