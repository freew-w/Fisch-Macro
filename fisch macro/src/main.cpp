#include "pch.h"
#include "fisch.h"
#include "config.h"
#include "gui.h"

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
//int main()
{
    std::thread renderingThread([]()
        {
            gui.startRendering();
        });

    SetForegroundWindow(fisch.robloxHWnd);

    fisch.startMacro();

    renderingThread.join();
}
