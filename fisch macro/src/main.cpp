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
			else
			{
				static int clickShakeFailSafe{};
				static auto lastIncrementTime = std::chrono::high_resolution_clock::now();
				auto currentTime = std::chrono::high_resolution_clock::now();
				if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastIncrementTime).count() >= 1)
				{
					clickShakeFailSafe++;
					lastIncrementTime = currentTime;
				}

				if (clickShakeFailSafe > 3)
				{
					clickShakeFailSafe = 0;
					if (config.config.autoEnableCameraMode)
						fisch.enableCameraMode(config.config.cameraModePos);
					if (config.config.autoBlur)
						fisch.blurCamera();
					if (config.config.autoLookDown)
						fisch.lookDown();
					if (config.config.autoZoomIn)
						fisch.zoomIn();
					fisch.castRod();
				}
			}
		}

		if (GetAsyncKeyState(VK_F6) & 0x01) fisch.enabled = !fisch.enabled;
		if (cv::waitKey(1) == 27) fisch.isRunning = false;

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	renderingThread.join();
}
