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

bool Fisch::setPos(ImVec2& pos)
{
	ImGui::Begin("set pos", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	static int prevPos{};
	static bool positioned{};
	if (prevPos != static_cast<int>(pos.x + pos.y))
	{
		prevPos = static_cast<int>(pos.x + pos.y);
		positioned = false;
	}
	if (!positioned)
	{
		positioned = true;
		ImGui::SetWindowPos({ pos.x - ImGui::GetWindowSize().x / 2.0f, pos.y - ImGui::GetWindowSize().y / 2.0f });
	}
	if (ImGui::Button("Ok"))
	{
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddRectFilled(ImGui::GetWindowPos(), { ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y }, IM_COL32(255, 0, 0, 255));
		pos.x = ImGui::GetWindowPos().x + ImGui::GetWindowSize().x / 2.0f;
		pos.y = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y / 2.0f;
		return true;
	}
	ImGui::End();

	return false;
}

bool Fisch::setArea(ImRect& rect)
{
	static int prevPos{};
	static bool positioned{};
	if (prevPos != static_cast<int>(rect.Min.x + rect.Min.y + rect.Max.x + rect.Max.y))
	{
		prevPos = static_cast<int>(rect.Min.x + rect.Min.y + rect.Max.x + rect.Max.y);
		positioned = false;
	}
	if (!positioned)
	{
		positioned = true;
		ImGui::SetNextWindowPos({ rect.Min.x,  rect.Min.y });
		ImGui::SetNextWindowSize({ rect.Max.x - rect.Min.x, rect.Max.y - rect.Min.y });
	}
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

cv::Mat Fisch::screenshot(const ImRect& rect)
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

void Fisch::enableCameraMode(const ImVec2& pos)
{
	POINT robloxClientToScreenPoint{};
	ClientToScreen(robloxHWnd, &robloxClientToScreenPoint);
	SetCursorPos(static_cast<int>(robloxClientToScreenPoint.x + pos.x), static_cast<int>(robloxClientToScreenPoint.y + pos.y));
	SendInput(1, &mouseMove, sizeof(INPUT));
	SendInput(2, leftMouseClick, sizeof(INPUT));
}

void Fisch::blurCamera()
{
	SendInput(2, mClick, sizeof(INPUT));
}

void Fisch::lookDown()
{
	RECT robloxClientRect{};
	POINT robloxClientToScreenPoint{};
	GetClientRect(robloxHWnd, &robloxClientRect);
	ClientToScreen(robloxHWnd, &robloxClientToScreenPoint);
	SetCursorPos(robloxClientToScreenPoint.x + robloxClientRect.right / 2, robloxClientToScreenPoint.y + robloxClientRect.bottom / 2);
	SendInput(1, &mouseMove, sizeof(INPUT));
	SendInput(1, &rightMouseClick[0], sizeof(INPUT));
	MOUSEINPUT mi{};
	INPUT input{};
	mi.dwFlags = MOUSEEVENTF_MOVE;
	input.type = INPUT_MOUSE;
	for (int i = 0; i < 50; i++) {
		mi.dy = 700 / 50;
		input.mi = mi;
		SendInput(1, &input, sizeof(INPUT));
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	SendInput(1, &rightMouseClick[1], sizeof(INPUT));
}

void Fisch::zoomIn()
{
	RECT robloxClientRect{};
	POINT robloxClientToScreenPoint{};
	GetClientRect(robloxHWnd, &robloxClientRect);
	ClientToScreen(robloxHWnd, &robloxClientToScreenPoint);
	SetCursorPos(robloxClientToScreenPoint.x + robloxClientRect.right / 2, robloxClientToScreenPoint.y + robloxClientRect.bottom / 2);
	for (int i = 0; i < 40; i++)
	{
		SendInput(1, &mouseScroll[0], sizeof(INPUT));
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	SendInput(1, &mouseScroll[1], sizeof(INPUT));
}

void Fisch::castRod()
{
	RECT robloxClientRect{};
	POINT robloxClientToScreenPoint{};
	GetClientRect(robloxHWnd, &robloxClientRect);
	ClientToScreen(robloxHWnd, &robloxClientToScreenPoint);
	SetCursorPos(robloxClientToScreenPoint.x + robloxClientRect.right / 2, robloxClientToScreenPoint.y + robloxClientRect.bottom / 2);
	SendInput(1, &mouseMove, sizeof(INPUT));
	SendInput(1, &leftMouseClick[0], sizeof(INPUT));
	std::this_thread::sleep_for(std::chrono::milliseconds(600));
	SendInput(1, &leftMouseClick[1], sizeof(INPUT));
}

void Fisch::clickShakeButton(const cv::Rect& rect)
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
	SendInput(1, &mouseMove, sizeof(INPUT));
	SendInput(2, leftMouseClick, sizeof(INPUT));
}
