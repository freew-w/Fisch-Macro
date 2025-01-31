#include "pch.h"
#include "fisch.h"
#include "config.h"
#include "gui.h"

Gui& Gui::get()
{
	static Gui gui{};
	return gui;
}

void Gui::startRendering()
{
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);

	while (fisch.isRunning)
	{
		MSG msg;
		while (PeekMessageW(&msg, hWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		static bool shouldShowOnNextFocus{};
		if (GetForegroundWindow() == fisch.robloxHWnd)
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
			GetClientRect(fisch.robloxHWnd, &robloxClientRect);
			ClientToScreen(fisch.robloxHWnd, &robloxClientToScreenPoint);
			SetWindowLongPtrW(hWnd, GWL_EXSTYLE, GetWindowLongPtr(hWnd, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);
			SetWindowPos(hWnd, HWND_TOPMOST, robloxClientToScreenPoint.x, robloxClientToScreenPoint.y, robloxClientRect.right, robloxClientRect.bottom, SWP_SHOWWINDOW);
		}
		else
			SetWindowLongPtrW(hWnd, GWL_EXSTYLE, GetWindowLongPtr(hWnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT);

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		if (shouldRender)
		{
			ImGui::Begin("fisch macro", &fisch.isRunning);

			ImGui::Checkbox("Enabled", &fisch.enabled);

			static bool doneSettingCameraModePos = true;
			if (ImGui::Button("Set camera mode button position"))		doneSettingCameraModePos = !doneSettingCameraModePos;

			static bool doneSettingBarDeadZoneLeftPos = true;
			if (ImGui::Button("Set bar dead zone left position"))		doneSettingBarDeadZoneLeftPos = !doneSettingBarDeadZoneLeftPos;

			static bool doneSettingBarDeadZoneRightPos = true;
			if (ImGui::Button("Set bar dead zone right position"))		doneSettingBarDeadZoneRightPos = !doneSettingBarDeadZoneRightPos;

			static bool doneSettingShakeButtonSearchingArea = true;
			if (ImGui::Button("Set shake button searching area"))		doneSettingShakeButtonSearchingArea = !doneSettingShakeButtonSearchingArea;

			static bool doneSettingBarSearchingArea = true;
			if (ImGui::Button("Set bar searching area"))				doneSettingBarSearchingArea = !doneSettingBarSearchingArea;

			ImGui::End();

			if (!doneSettingCameraModePos)				doneSettingCameraModePos = fisch.setPos(config.coordinates.cameraModePos);
			if (!doneSettingBarDeadZoneLeftPos)			doneSettingBarDeadZoneLeftPos = fisch.setPos(config.coordinates.barDeadZoneLeftPos);
			if (!doneSettingBarDeadZoneRightPos)		doneSettingBarDeadZoneRightPos = fisch.setPos(config.coordinates.barDeadZoneRightPos);
			if (!doneSettingShakeButtonSearchingArea)	doneSettingShakeButtonSearchingArea = fisch.setArea(config.coordinates.searchShakeRect);
			if (!doneSettingBarSearchingArea)			doneSettingBarSearchingArea = fisch.setArea(config.coordinates.searchBarRect);
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

Gui::Gui()
{
	wc.cbClsExtra = 0;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.cbWndExtra = 0;
	wc.hbrBackground = nullptr;
	wc.hCursor = nullptr;
	wc.hIcon = nullptr;
	wc.hIconSm = nullptr;
	wc.hInstance = GetModuleHandleW(nullptr);
	wc.lpfnWndProc = sWndProc;
	wc.lpszClassName = L"fisch macro";
	wc.lpszMenuName = nullptr;
	wc.style = CS_CLASSDC;
	RegisterClassExW(&wc);

	RECT robloxClientRect{};
	POINT robloxClientToScreenPoint{};
	GetClientRect(fisch.robloxHWnd, &robloxClientRect);
	ClientToScreen(fisch.robloxHWnd, &robloxClientToScreenPoint);
	hWnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE, wc.lpszClassName, L"fisch macro", WS_POPUP, robloxClientToScreenPoint.x, robloxClientToScreenPoint.y, robloxClientRect.right, robloxClientRect.bottom, nullptr, nullptr, wc.hInstance, this);
	if (!hWnd)
		fisch.error(L"Failed to create window");
	SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 255, LWA_ALPHA);
	MARGINS margins{ -1, -1, -1, -1 };
	DwmExtendFrameIntoClientArea(hWnd, &margins);

	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	d3dpp.Windowed = true;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	if (FAILED(d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3dDevice)))
		fisch.error(L"Failed to create d3d device");

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX9_Init(d3dDevice);
}

Gui::~Gui()
{
	if (d3dDevice)
		d3dDevice->Release();
	if (d3d)
		d3d->Release();
	if (hWnd)
		DestroyWindow(hWnd);
	if (wc.lpszClassName)
		UnregisterClass(wc.lpszClassName, wc.hInstance);
}

LRESULT Gui::sWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Gui* pThis = reinterpret_cast<Gui*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));

	switch (uMsg)
	{
	case WM_NCCREATE:
		pThis = reinterpret_cast<Gui*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
		break;
	}
	if (pThis)
		return pThis->WndProc(hWnd, uMsg, wParam, lParam);
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT Gui::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	switch (uMsg)
	{
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED || !d3dDevice)
			return 0;
		d3dpp.BackBufferWidth = static_cast<UINT>(LOWORD(lParam));
		d3dpp.BackBufferHeight = static_cast<UINT>(HIWORD(lParam));
		ImGui_ImplDX9_InvalidateDeviceObjects();
		if (d3dDevice->Reset(&d3dpp) == D3DERR_INVALIDCALL)
			IM_ASSERT(0);
		ImGui_ImplDX9_CreateDeviceObjects();
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
