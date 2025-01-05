#include "pch.hpp"
#include "fisch.hpp"
#include "gui.hpp"

int main()
{
    Fisch &fisch = Fisch::get();

    SetForegroundWindow(fisch.getRobloxHWnd());

    std::thread renderingThread(
        []()
        {
            Gui &gui = Gui::get();
            gui.startRendering();
        });

    renderingThread.join();

    return 0;
}
