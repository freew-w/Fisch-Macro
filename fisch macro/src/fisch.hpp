#pragma once

namespace fisch
{
	inline HWND robloxHWnd{};
	inline bool isRunning = true;

	bool init();
	void error(const TCHAR* msg);

	inline cv::Mat screenshot()
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

	/*inline cv::Mat screenshot()
	{
		RECT robloxClientRect{};
		POINT robloxClientToScreenPoint{};
		GetClientRect(robloxHWnd, &robloxClientRect);
		ClientToScreen(robloxHWnd, &robloxClientToScreenPoint);

		static HDC hDC = GetDC(nullptr);
		static HDC hMemDC = CreateCompatibleDC(hDC);
		static HBITMAP hBitmap = CreateCompatibleBitmap(hDC, robloxClientRect.right, robloxClientRect.bottom);

		HGDIOBJ hOldBitmap = SelectObject(hMemDC, hBitmap);
		BitBlt(hMemDC, 0, 0, robloxClientRect.right, robloxClientRect.bottom, hDC, robloxClientToScreenPoint.x, robloxClientToScreenPoint.y, SRCCOPY);

		cv::Mat mat(robloxClientRect.bottom, robloxClientRect.right, CV_8UC4);
		BITMAPINFOHEADER bih = { sizeof(BITMAPINFOHEADER), robloxClientRect.right, -robloxClientRect.bottom, 1, 32, BI_RGB };
		GetDIBits(hDC, hBitmap, 0, robloxClientRect.bottom, mat.data, (BITMAPINFO*)&bih, DIB_RGB_COLORS);

		SelectObject(hMemDC, hOldBitmap);

		return mat;
	}*/
}
