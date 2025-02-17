#include "pch.h"
#include "config.h"
#include "fisch.h"
#include "gui.h"
#include "input.h"

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
//int main()
{
    SetForegroundWindow(Roblox::getInstance().getRobloxHWnd());

    std::thread renderingThread([]()
        {
            try
            {
                while (fisch::isRunning)
                    Gui::getInstance().renderFrame();
            }
            catch (const std::exception& e)
            {
                Gui::getInstance().hideWindow();
                MessageBoxA(nullptr, e.what(), "Error", MB_ICONERROR | MB_TOPMOST);
                exit(EXIT_FAILURE);
            }
        });

    try
    {
        bool firstRun = true;
        bool newRod = true;
        auto lastLoopTime = std::chrono::steady_clock::now();

        while (fisch::isRunning)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (GetAsyncKeyState(VK_F6) & 0x01) fisch::enabled = !fisch::enabled;

            if (!fisch::enabled)
            {
                firstRun = true;
                newRod = true;
                fisch::failSafe(true);
                continue;
            }

            auto currentTime = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastLoopTime).count() >= 1)
            {
                lastLoopTime = currentTime;
                fisch::runTimeSeconds++;
                if (fisch::runTimeSeconds >= 59)
                {
                    fisch::runTimeSeconds = 0;
                    fisch::runTimeMinutes++;
                }
                if (fisch::runTimeMinutes >= 59)
                {
                    fisch::runTimeMinutes = 0;
                    fisch::runTimeHours++;
                }
            }

            if (firstRun)
            {
                firstRun = false;
                if (Config::getInstance().getConfig().autoEnableCameraMode) fisch::toggleCameraMode();
                if (Config::getInstance().getConfig().autoLookDown) fisch::lookDown();
                if (Config::getInstance().getConfig().autoZoomIn) fisch::zoomIn();
            }

            cv::Rect shakeButtonRect = fisch::findShakeButton(fisch::screenshot(Config::getInstance().getPositions().searchShakeRegion));
            if (shakeButtonRect.width)
            {
                fisch::failSafe(true);

                if (!Config::getInstance().getConfig().autoShake)
                    continue;

                fisch::clickShakeButton(shakeButtonRect);
                continue;
            }

            cv::Mat minigameMat = fisch::screenshot(Config::getInstance().getPositions().searchBarRegion);
            auto [lineRect, arrowRect] = fisch::findLineAndArrow(minigameMat);
            if (lineRect.x && arrowRect.width)
            {
                fisch::failSafe(true);

                if (!Config::getInstance().getConfig().autoBarMinigame)
                    continue;

                if (newRod)
                {
                    newRod = false;
                    if (Config::getInstance().getConfig().autoCalculateBarWidth)
                        Config::getInstance().getConfig().barWidth = fisch::getBarWidth(minigameMat);
                }

                fisch::doBarMinigame(lineRect, arrowRect);
                continue;
            }

            if (fisch::failSafe())
            {
                if (Config::getInstance().getConfig().autoSell) fisch::sell();
                if (Config::getInstance().getConfig().autoBlur) fisch::toggleCameraBlur();
                if (Config::getInstance().getConfig().autoLookDown) fisch::lookDown();
                fisch::castRod();
            }
        }
    }
    catch (const std::exception& e)
    {
        Gui::getInstance().hideWindow();
        MessageBoxA(nullptr, e.what(), "Error", MB_ICONERROR | MB_TOPMOST);
        exit(EXIT_FAILURE);
    }

    if (renderingThread.joinable())
        renderingThread.join();
}
