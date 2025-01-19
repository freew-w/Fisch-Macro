#include "pch.hpp"
#include "fisch.hpp"
#include "gui.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT gui::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	switch (uMsg)
	{
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED || !d3dDevice)
			return 0;
		d3dpp.BackBufferWidth = (UINT)LOWORD(lParam);
		d3dpp.BackBufferHeight = (UINT)HIWORD(lParam);
		ImGui_ImplDX9_InvalidateDeviceObjects();
		if (d3dDevice->Reset(&d3dpp) == D3DERR_INVALIDCALL)
			IM_ASSERT(0);
		ImGui_ImplDX9_CreateDeviceObjects();
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool gui::init()
{
	while (!fisch::robloxHWnd)
	{
		static int retryCount{};
		if (retryCount++ >= 5) return false;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	wc.cbClsExtra = 0;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.cbWndExtra = 0;
	wc.hbrBackground = nullptr;
	wc.hCursor = nullptr;
	wc.hIcon = nullptr;
	wc.hIconSm = nullptr;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = _T("fisch-macro");
	wc.lpszMenuName = nullptr;
	wc.style = CS_CLASSDC;
	RegisterClassEx(&wc);

	RECT robloxClientRect{};
	POINT robloxClientToScreenPoint{};
	GetClientRect(fisch::robloxHWnd, &robloxClientRect);
	ClientToScreen(fisch::robloxHWnd, &robloxClientToScreenPoint);
	hWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE, wc.lpszClassName, _T("fisch-macro"), WS_POPUP, robloxClientToScreenPoint.x, robloxClientToScreenPoint.y, robloxClientRect.right, robloxClientRect.bottom, nullptr, nullptr, wc.hInstance, nullptr);
	if (!hWnd) return false;
	SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 255, LWA_ALPHA);
	MARGINS margins{ -1, -1, -1, -1 };
	DwmExtendFrameIntoClientArea(hWnd, &margins);

	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	d3dpp.Windowed = true;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	if (FAILED(d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3dDevice)))
		return false;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX9_Init(d3dDevice);

	return true;
}

void gui::startRendering()
{
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);

	while (fisch::isRunning)
	{
		MSG msg;
		while (PeekMessage(&msg, gui::hWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				fisch::isRunning = false;
		}

		static bool shouldShowOnNextFocus{};
		if (GetForegroundWindow() == fisch::robloxHWnd)
		{
			if (GetAsyncKeyState(VK_INSERT) & 0x01)
				shouldRender = !shouldRender;
			if (shouldShowOnNextFocus)
			{
				shouldShowOnNextFocus = false;
				shouldRender = true;
			}
		}
		else
		{
			if (shouldRender)
				shouldShowOnNextFocus = true;
			shouldRender = false;
		}
		if (shouldRender)
		{
			RECT robloxClientRect{};
			POINT robloxClientToScreenPoint{};
			GetClientRect(fisch::robloxHWnd, &robloxClientRect);
			ClientToScreen(fisch::robloxHWnd, &robloxClientToScreenPoint);
			SetWindowLongPtr(hWnd, GWL_EXSTYLE, GetWindowLongPtr(hWnd, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);
			SetWindowPos(hWnd, HWND_TOPMOST, robloxClientToScreenPoint.x, robloxClientToScreenPoint.y, robloxClientRect.right, robloxClientRect.bottom, SWP_SHOWWINDOW);
		}
		else
			SetWindowLongPtr(hWnd, GWL_EXSTYLE, GetWindowLongPtr(hWnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT);

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		if (shouldRender)
		{
			ImGui::Begin("test", &fisch::isRunning);
			ImGui::End();
		}
		ImGui::EndFrame();

		d3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
		d3dDevice->BeginScene();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		d3dDevice->EndScene();
		d3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
	}
}

void gui::cleanup()
{
	d3dDevice->Release();
	d3d->Release();
	DestroyWindow(hWnd);
	UnregisterClass(wc.lpszClassName, wc.hInstance);
}
