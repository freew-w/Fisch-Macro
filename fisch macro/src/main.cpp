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
		static bool firstExec = true;
		static int findComponentFailSafe = 0;

		if (fisch.enabled)
		{

			cv::Rect shakeButtonRect = fisch.findShakeButton(fisch.screenshot(config.config.searchShakeRect));

			if (!shakeButtonRect.width) // shake button not found
			{
				if (firstExec)
				{
					firstExec = false;
					if (config.config.autoEnableCameraMode)
						fisch.enableCameraMode(config.config.cameraModePos);
					if (config.config.autoLookDown)
						fisch.lookDown();
					if (config.config.autoZoomIn)
						fisch.zoomIn();
				}

				cv::Mat findBarMat = fisch.screenshot(config.config.searchBarRect);
				cv::Rect lineRect = fisch.findLine(findBarMat);
				cv::Rect barRect = fisch.findBar(findBarMat);

				if (barRect.width || lineRect.x) // bar found
				{
					findComponentFailSafe = 0;

					continue;
				}

				static auto lastIncrementTime = std::chrono::high_resolution_clock::now();
				auto currentTime = std::chrono::high_resolution_clock::now();
				if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastIncrementTime).count() >= 1)
				{
					findComponentFailSafe++;
					lastIncrementTime = currentTime;
				}
				if (findComponentFailSafe >= 2) // restart
				{
					findComponentFailSafe = 0;

					if (config.config.autoBlur)
						fisch.blurCamera();
					if (config.config.autoLookDown)
						fisch.lookDown();
					fisch.castRod();
				}
			}
			else // shake button found
			{
				findComponentFailSafe = 0;

				fisch.clickShakeButton(shakeButtonRect);
				std::this_thread::sleep_for(std::chrono::milliseconds(config.config.clickShakeDelay));
			}
		}
		else
		{
			firstExec = true;
			findComponentFailSafe = 0;
		}

		if (GetAsyncKeyState(VK_F6) & 0x01) fisch.enabled = !fisch.enabled;
		if (cv::waitKey(1) == 27) fisch.isRunning = false;

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	renderingThread.join();
}
