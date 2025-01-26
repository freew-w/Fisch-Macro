#include "pch.h"
#include "fisch.h"
#include "config.h"

Fisch& Fisch::get()
{
	static Fisch fisch{};
	return fisch;
}

void Fisch::warn(LPCWSTR msg)
{
	MessageBoxW(nullptr, msg, L"Error", MB_ICONWARNING);
}

void Fisch::error(LPCWSTR msg)
{
	MessageBoxW(nullptr, msg, L"Error", MB_ICONERROR);
	exit(EXIT_FAILURE);
}

bool Fisch::setArea(ImRect& rect)
{
	ImGui::Begin("set area", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	if (ImGui::Button("Ok"))
	{
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddRectFilled(ImGui::GetWindowPos(), { ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y }, IM_COL32(255, 0, 0, 255));
		rect.Min.x = ImGui::GetWindowPos().x;
		rect.Min.y = ImGui::GetWindowPos().y;
		rect.Max.x = ImGui::GetWindowPos().x + ImGui::GetWindowSize().x;
		rect.Max.y = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y;
		return true;
	}
	ImGui::End();

	return false;
}

Fisch::Fisch()
{
	robloxHWnd = FindWindowW(nullptr, L"Roblox");
	if (!robloxHWnd)
		error(L"Roblox not found");
}

cv::Mat Fisch::screenshot(ImRect rect)
{
	if (rect.Min.x == 0 && rect.Min.y == 0 && rect.Max.x == 0 && rect.Max.y == 0)
		return cv::Mat();

	POINT robloxClientToScreenPoint{};
	ClientToScreen(robloxHWnd, &robloxClientToScreenPoint);
	int w = static_cast<int>(rect.Max.x - rect.Min.x);
	int h = static_cast<int>(rect.Max.y - rect.Min.y);

	HDC hDC = GetDC(nullptr);
	HDC hMemDC = CreateCompatibleDC(hDC);
	HBITMAP hBitmap = CreateCompatibleBitmap(hDC, w, h);

	HGDIOBJ hOldBitmap = SelectObject(hMemDC, hBitmap);
	StretchBlt(hMemDC, 0, 0, w, h, hDC, static_cast<int>(robloxClientToScreenPoint.x + rect.Min.x), static_cast<int>(robloxClientToScreenPoint.y + rect.Min.y), w, h, SRCCOPY);

	cv::Mat mat(h, w, CV_8UC4);
	BITMAPINFOHEADER bih = { sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB };
	GetDIBits(hDC, hBitmap, 0, h, mat.data, reinterpret_cast<BITMAPINFO*>(&bih), DIB_RGB_COLORS);

	SelectObject(hMemDC, hOldBitmap);
	DeleteDC(hMemDC);
	DeleteObject(hBitmap);
	ReleaseDC(nullptr, hDC);

	return mat;
}

cv::Rect Fisch::findShakeButton(cv::Mat mat)
{
	cv::Mat contourMask(mat.rows, mat.cols, CV_8UC1, cv::Scalar());
	std::vector<std::vector<cv::Point>> contours;

	cv::cvtColor(mat, mat, cv::COLOR_RGBA2GRAY);
	cv::dilate(mat, mat, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));
	cv::Canny(mat, mat, 200, 300);
	cv::findContours(mat, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	for (auto& contour : contours)
	{
		if (cv::contourArea(contour) < 7000 || cv::contourArea(contour) > 12500 || contour.size() < 100)
			continue;
		cv::approxPolyDP(contour, contour, 0.05 * cv::arcLength(contour, true), true);

		RECT robloxClientRect{};
		GetClientRect(robloxHWnd, &robloxClientRect);
		cv::Rect resultRect = cv::boundingRect(contour);
		resultRect.x += static_cast<int>(config.config.searchShakeRect.Min.x - robloxClientRect.left);
		resultRect.y += static_cast<int>(config.config.searchShakeRect.Min.y - robloxClientRect.top);
		return resultRect;
	}

	return cv::Rect();
}

void Fisch::clickShakeButton(cv::Rect rect)
{
	/*static int lastPos{};
	int pos = rect.x + rect.width / 2 + rect.y + rect.height / 2;
	if (abs(lastPos - pos) < 4)
	{
		lastPos = pos;
		return;
	}
	lastPos = pos;*/

	POINT robloxClientToScreenPoint{};
	ClientToScreen(robloxHWnd, &robloxClientToScreenPoint);
	SetCursorPos(robloxClientToScreenPoint.x + rect.x + rect.width / 2, robloxClientToScreenPoint.y + rect.y + rect.height / 2);
	SendInput(3, mouseClick, sizeof(INPUT));
}
