#pragma once

class Fisch
{
public:
	bool isRunning = true;
	bool enabled{};
	HWND robloxHWnd{};

	static Fisch& get();
	void warn(LPCWSTR msg);
	void error(LPCWSTR msg);
	bool setArea(ImRect& rect);
	cv::Mat screenshot(ImRect rect);
	cv::Rect findShakeButton(cv::Mat mat);
	void clickShakeButton(cv::Rect rect);

private:
	INPUT mouseClick[3]{ {.type = INPUT_MOUSE, .mi{1, 0, 0, MOUSEEVENTF_MOVE, 0, 0}}, {.type = INPUT_MOUSE, .mi{0, 0, 0, MOUSEEVENTF_LEFTDOWN, 0, 0}}, {.type = INPUT_MOUSE, .mi{0, 0, 0, MOUSEEVENTF_LEFTUP, 0, 0}} };

	Fisch();
};

inline Fisch& fisch = Fisch::get();
