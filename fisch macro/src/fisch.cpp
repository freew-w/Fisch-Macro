#include "pch.hpp"
#include "fisch.hpp"

bool fisch::init()
{
    robloxHWnd = FindWindow(nullptr, _T("Roblox"));
    if (!robloxHWnd) return false;

    return true;
}

void fisch::error(const TCHAR* msg)
{
    MessageBox(nullptr, msg, _T("Error"), MB_ICONERROR);
    exit(EXIT_FAILURE);
}