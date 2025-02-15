#include "pch.h"
#include "config.h"
#include "fisch.h"
#include "gui.h"
#include "input.h"

//int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
int main()
{
    std::thread renderingThread([]()
        {
            try
            {
                while (fisch::isRunning)
                {
                    Gui::getInstance().renderFrame();
                }
            }
            catch (const std::exception& e)
            {
                std::cout << e.what() << std::endl;
                exit(EXIT_FAILURE);
            }
        });

    try
    {
        bool firstRun = true;
        bool newRun = true;

        while (fisch::isRunning)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (GetAsyncKeyState(VK_F6) & 0x01) fisch::enabled = !fisch::enabled;

            if (!fisch::enabled)
            {
                firstRun = true;
                newRun = true;
                continue;
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

                if (newRun)
                {
                    newRun = false;
                    if (Config::getInstance().getConfig().autoCalculateBarWidth)
                        Config::getInstance().getConfig().barWidth = fisch::getBarWidth(minigameMat);
                }

                fisch::doBarMinigame(lineRect, arrowRect);
                continue;
            }

            if (fisch::failSafe())
            {
                newRun = true;
                if (Config::getInstance().getConfig().autoSell) fisch::sell();
                if (Config::getInstance().getConfig().autoBlur) fisch::toggleCameraBlur();
                if (Config::getInstance().getConfig().autoLookDown) fisch::lookDown();
                fisch::castRod();
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    renderingThread.join();
}
