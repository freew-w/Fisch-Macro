#pragma once

namespace fisch
{
	template <int dx, int dy>
	inline INPUT mouseClick[3]{ {.type = INPUT_MOUSE, .mi{dx, dy, 0, MOUSEEVENTF_MOVE, 0, 0}}, {.type = INPUT_MOUSE, .mi{0, 0, 0, MOUSEEVENTF_LEFTDOWN, 0, 0}}, {.type = INPUT_MOUSE, .mi{0, 0, 0, MOUSEEVENTF_LEFTUP, 0, 0}} };

	inline HWND robloxHWnd{};
	inline bool isRunning = true;

	void error(const TCHAR* msg);
	bool init();
	cv::Mat screenshot();
	cv::Rect findShakeButton(cv::Mat mat);
	void clickShakeButton(cv::Rect rect);
}
