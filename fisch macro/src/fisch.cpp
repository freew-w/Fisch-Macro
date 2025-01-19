#include "pch.hpp"
#include "fisch.hpp"

void fisch::error(const TCHAR* msg)
{
	MessageBox(nullptr, msg, _T("Error"), MB_ICONERROR);
	exit(EXIT_FAILURE);
}

bool fisch::init()
{
	robloxHWnd = FindWindow(nullptr, _T("Roblox"));
	if (!robloxHWnd) return false;

	return true;
}

cv::Mat fisch::screenshot()
{
	RECT robloxClientRect{};
	POINT robloxClientToScreenPoint{};
	GetClientRect(robloxHWnd, &robloxClientRect);
	ClientToScreen(robloxHWnd, &robloxClientToScreenPoint);

	HDC hDC = GetDC(nullptr);
	HDC hMemDC = CreateCompatibleDC(hDC);
	HBITMAP hBitmap = CreateCompatibleBitmap(hDC, robloxClientRect.right, robloxClientRect.bottom);

	HGDIOBJ hOldBitmap = SelectObject(hMemDC, hBitmap);
	StretchBlt(hMemDC, 0, 0, robloxClientRect.right, robloxClientRect.bottom, hDC, robloxClientToScreenPoint.x, robloxClientToScreenPoint.y, robloxClientRect.right, robloxClientRect.bottom, SRCCOPY);

	cv::Mat mat(robloxClientRect.bottom, robloxClientRect.right, CV_8UC4);
	BITMAPINFOHEADER bih = { sizeof(BITMAPINFOHEADER), robloxClientRect.right, -robloxClientRect.bottom, 1, 32, BI_RGB };
	GetDIBits(hDC, hBitmap, 0, robloxClientRect.bottom, mat.data, (BITMAPINFO*)&bih, DIB_RGB_COLORS);

	SelectObject(hMemDC, hOldBitmap);
	DeleteDC(hMemDC);
	DeleteObject(hBitmap);
	ReleaseDC(nullptr, hDC);

	return mat;
}

cv::Rect fisch::findShakeButton(cv::Mat mat)
{
	cv::Mat contourMask(mat.rows, mat.cols, CV_8UC1, cv::Scalar());
	std::vector<std::vector<cv::Point>> contours;

	cv::cvtColor(mat, mat, cv::COLOR_RGBA2GRAY);
	cv::Canny(mat, mat, 200, 300);
	cv::findContours(mat, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	for (size_t i{}; i < contours.size(); i++)
	{
		if (cv::contourArea(contours[i]) < 7000 || cv::contourArea(contours[i]) > 9500)
			if (i != contours.size() - 1)
				continue;
			else
				break;
		cv::approxPolyDP(contours[i], contours[i], 0.05 * cv::arcLength(contours[i], true), true);
		return cv::boundingRect(contours[i]);
	}

	return cv::Rect();
}

void fisch::clickShakeButton(cv::Rect rect)
{
	/*static int lastPos{};
	int pos = rect.x + rect.width / 2 + rect.y + rect.height / 2;
	if (abs(lastPos - pos) < 3)
	{
		lastPos = pos;
		return;
	}
	lastPos = pos;*/

	POINT robloxClientToScreenRect{};
	ClientToScreen(robloxHWnd, &robloxClientToScreenRect);
	SetCursorPos(robloxClientToScreenRect.x + rect.x + rect.width / 2, robloxClientToScreenRect.y + rect.y + rect.height / 2);
	SendInput(3, mouseClick<1, 0>, sizeof(INPUT));
}
