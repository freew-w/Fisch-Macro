#include "pch.h"
#include "fisch.h"
#include "config.h"
#include "gui.h"

int main()
{
	std::thread renderingThread([]()
		{
			gui.startRendering();
		});

	SetForegroundWindow(fisch.robloxHWnd);

	while (fisch.isRunning)
	{
		if (fisch.enabled)
		{
			cv::Rect shakeButtonRect = fisch.findShakeButton(fisch.screenshot(config.config.searchShakeRect));
			if (shakeButtonRect.width > 0)
			{
				fisch.clickShakeButton(shakeButtonRect);
				std::this_thread::sleep_for(std::chrono::milliseconds(config.config.clickShakeDelay));
			}
		}

		if (cv::waitKey(1) == 27) fisch.isRunning = false;

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	renderingThread.join();
}
