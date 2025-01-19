#include "pch.hpp"
#include "fisch.hpp"
#include "gui.hpp"

int main()
{
	std::thread renderingThread([]()
		{
			if (!gui::init()) fisch::error(_T("Failed to initialize GUI"));
			gui::startRendering();
			gui::cleanup();
		});

	if (!fisch::init()) fisch::error(_T("Failed to initialize macro"));

	while (fisch::isRunning)
	{
		cv::Mat screenshot = fisch::screenshot();
		cv::Rect shakeButtonRect = fisch::findShakeButton(screenshot);

		if (shakeButtonRect.width > 0)
		{
			cv::rectangle(screenshot, shakeButtonRect, cv::Scalar(0, 0, 255), 2);
			fisch::clickShakeButton(shakeButtonRect);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		cv::imshow("screenshot", screenshot);
		if (cv::waitKey(1) == 27) fisch::isRunning = false;

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	renderingThread.join();
}
