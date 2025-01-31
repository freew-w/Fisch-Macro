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
		if (GetAsyncKeyState(VK_F6) & 0x01) fisch.enabled = !fisch.enabled;
		if (cv::waitKey(1) == 27) fisch.isRunning = false;
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		static bool firstExec = true;
		static int findComponentFailSafe = 0;
		if (!fisch.enabled)
		{
			firstExec = true;
			findComponentFailSafe = 0;
			continue;
		}

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

		cv::Rect shakeButtonRect = fisch.findShakeButton(fisch.screenshot(config.config.searchShakeRect));
		if (shakeButtonRect.width)
		{
			findComponentFailSafe = 0;
			fisch.clickShakeButton(shakeButtonRect);
			std::this_thread::sleep_for(std::chrono::milliseconds(config.config.clickShakeDelay));
			continue;
		}

		cv::Mat fishingMinigameMat = fisch.screenshot(config.config.searchBarRect);
		cv::Rect lineRect = fisch.findLine(fishingMinigameMat);
		cv::Rect arrowRect = fisch.findArrow(fishingMinigameMat);
		if (lineRect.x || arrowRect.width)
		{
			findComponentFailSafe = 0;

			static constexpr double kp = 0.1, kd = 2.0;
			static auto lastLoopTime = std::chrono::high_resolution_clock::now();
			static double prevError = lineRect.x - arrowRect.x;
			//static int prevArrowX = arrowRect.x;

			auto currentTime = std::chrono::high_resolution_clock::now();
			auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastLoopTime).count();

			double error = lineRect.x - arrowRect.x;
			double derivative = (error - prevError) / deltaTime;
			//double velocity = deltaTime ? (arrowRect.x - prevArrowX) / static_cast<double>(deltaTime) : 0;

			int output = static_cast<int>(kp * error + kd * derivative);

			if (lineRect.x < config.config.barDeadZoneLeftPos.x)
			{
				SendInput(1, &fisch.leftMouseClick[1], sizeof(INPUT));
				continue;
			}
			else if (lineRect.x > config.config.barDeadZoneRightPos.x)
			{
				SendInput(1, &fisch.leftMouseClick[0], sizeof(INPUT));
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}

			if (output > 0)
			{
				SendInput(1, &fisch.leftMouseClick[0], sizeof(INPUT));
				std::this_thread::sleep_for(std::chrono::milliseconds(output));
			}
			else
			{
				SendInput(1, &fisch.leftMouseClick[1], sizeof(INPUT));
				std::this_thread::sleep_for(std::chrono::milliseconds(-output));
			}

			lastLoopTime = currentTime;
			prevError = error;
			//prevArrowX = arrowRect.x;

			continue;
		}

		static auto lastIncrementTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastIncrementTime).count() >= 1)
		{
			findComponentFailSafe++;
			lastIncrementTime = currentTime;
		}
		if (findComponentFailSafe >= 2)
		{
			findComponentFailSafe = 0;

			if (config.config.autoBlur)
				fisch.blurCamera();
			if (config.config.autoLookDown)
				fisch.lookDown();
			fisch.castRod();
		}
	}

	renderingThread.join();
}
