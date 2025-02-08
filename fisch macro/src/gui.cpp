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
        if (GetForegroundWindow() == fisch.robloxHWnd || GetForegroundWindow() == hWnd)
        {
            if (GetAsyncKeyState(VK_INSERT) & 0x01)
            {
                shouldRender = !shouldRender;
                if (shouldRender)
                    SetForegroundWindow(hWnd);
                else
                    SetForegroundWindow(fisch.robloxHWnd);
            }
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
            ImGui::SetNextWindowSize({ 580, 380 }, ImGuiCond_Once);
            ImGui::Begin("Fisch Macro v0.2.0 (insert to show/hide)", &fisch.isRunning);
            {
                static int page{};
                ImGui::BeginChild("side bar", ImVec2(100, 0), true);
                {
                    if (ImGui::Button("Main", { 85, 30 }))
                        page = 0;
                    ImGui::NewLine();
                    if (ImGui::Button("Misc", { 85, 30 }))
                        page = 1;
                    ImGui::NewLine();
                    if (ImGui::Button("Config", { 85, 30 }))
                        page = 2;
                }
                ImGui::EndChild();

                ImGui::SameLine();

                ImGui::BeginChild("content", ImVec2(0, 0), true);
                {
                    static bool setCameraModePosition{},
                        setShakeButtonSearchingRegion{},
                        setBarSearchingRegion{},
                        setBarDeadZoneLeftPosition{},
                        setBarDeadZoneRightPosition{},
                        setSellButtonPosition{};

                    fisch.setPosition(config.coordinates.cameraModePosition, setCameraModePosition);
                    fisch.setRegion(config.coordinates.searchShakeRect, setShakeButtonSearchingRegion);
                    fisch.setRegion(config.coordinates.searchBarRect, setBarSearchingRegion);
                    fisch.setPosition(config.coordinates.barDeadZoneLeftPosition, setBarDeadZoneLeftPosition);
                    fisch.setPosition(config.coordinates.barDeadZoneRightPosition, setBarDeadZoneRightPosition);
                    fisch.setPosition(config.coordinates.sellButtonPosition, setSellButtonPosition);

                    switch (page)
                    {
                    case 0:
                        ImGui::Text("Macro");
                        ImGui::Separator();
                        {
                            if (ImGui::Checkbox("Enabled(F6)", &fisch.enabled))
                                shouldRender = false;
                        }

                        ImGui::Text("Auto Shake");
                        ImGui::Separator();
                        {
                            ImGui::PushID("Auto Shake Enabled");
                            ImGui::Checkbox("Enabled", &config.config.autoShake);
                            ImGui::PopID();

                            ImGui::NewLine();

                            if (!config.config.autoShake) ImGui::BeginDisabled();
                            {

                                ImGui::SetNextItemWidth(100);
                                ImGui::InputInt("Fail Safe Count", &config.config.failSafeCount);
                                ImGui::SameLine(); helpMarker("Threshold for fail safe count.\nMacro will restart if the threshold is reached.\n\nFail safe count increases if no shake button/bar/line is found, with a 1 second cooldown.");

                                ImGui::NewLine();

                                ImGui::SetNextItemWidth(100);
                                ImGui::InputInt("Cast Time", &config.config.castTime);
                                ImGui::SameLine(); helpMarker("The hold time(ms) for casting the rod.");

                                ImGui::NewLine();

                                ImGui::Checkbox("Auto Enable Camera Mode", &config.config.autoEnableCameraMode);
                                ImGui::Checkbox("Auto Blur", &config.config.autoBlur);
                                ImGui::Checkbox("Auto Look Down", &config.config.autoLookDown);
                                ImGui::Checkbox("Auto Zoom In", &config.config.autoZoomIn);

                                ImGui::NewLine();

                                ImGui::SetNextItemWidth(100);
                                ImGui::InputInt("Minimum Shake Button Area", &config.config.minimumShakeButtonArea);
                                ImGui::SetNextItemWidth(100);
                                ImGui::InputInt("Maximum Shake Button Area", &config.config.maximumShakeButtonArea);

                                ImGui::NewLine();

                                ImGui::Checkbox("Check Click Position", &config.config.checkClickShakePosition);
                                if (config.config.checkClickShakePosition)
                                {
                                    ImGui::SameLine();
                                    helpMarker("Prevent clicking the same position multiple times by comparing the current position with the last position.\nIf enabled, the click shake delay will be ignored.");
                                }
                                else
                                {
                                    ImGui::SameLine();
                                    ImGui::SetNextItemWidth(100);
                                    ImGui::InputInt("Click Shake Delay", &config.config.clickShakeDelay);
                                    ImGui::SameLine();
                                    helpMarker("Prevent clicking the same position multiple times by adding a delay(ms) between each click.");
                                }

                                ImGui::NewLine();

                                if (ImGui::Button("Set Camera Mode Button Position"))		setCameraModePosition = !setCameraModePosition;
                                ImGui::SameLine(); helpMarker("Set the position to click to enable camera mode.\n\nMove the box to the desired position by dragging it or right clicking, then click \"Ok\".");
                                if (ImGui::Button("Set Shake Button Searching Region"))		setShakeButtonSearchingRegion = !setShakeButtonSearchingRegion;
                                ImGui::SameLine(); helpMarker("Set the region to search for the shake button.\n\nMove the box to the desired region by resizing and dragging it or ctrl + left clicking two opposite corners, then click \"Ok\".");
                            }
                            if (!config.config.autoShake) ImGui::EndDisabled();
                        }

                        ImGui::Text("Auto Bar Minigame");
                        ImGui::Separator();
                        {
                            ImGui::PushID("Auto Bar Minigame Enabled");
                            ImGui::Checkbox("Enabled", &config.config.autoBarMinigame);
                            ImGui::PopID();

                            ImGui::NewLine();

                            if (!config.config.autoBarMinigame) ImGui::BeginDisabled();
                            {
                                ImGui::Checkbox("Auto Calculate Bar Width", &config.config.autoCalculateBarWidth);
                                if (!config.config.autoCalculateBarWidth)
                                {
                                    ImGui::SameLine();
                                    ImGui::SetNextItemWidth(100);
                                    ImGui::InputInt("Bar Width", &config.config.barWidth);
                                }

                                ImGui::NewLine();

                                ImGui::Checkbox("Use Bar Dead Zone Left", &config.config.useBarDeadZoneLeft);
                                ImGui::Checkbox("Use Bar Dead Zone Right", &config.config.useBarDeadZoneRight);

                                ImGui::NewLine();

                                ImGui::SetNextItemWidth(150);
                                ImGui::InputDouble("Kp", &config.config.kp, 0.1, 1.0);
                                ImGui::SameLine();
                                ImGui::SetNextItemWidth(150);
                                ImGui::InputDouble("Kd", &config.config.kd, 0.1, 1.0);
                                ImGui::SameLine();helpMarker("Kp: proportional gain\nKd: derivative gain\nError = line x - arrow x\nDerivative = (error - previous error) / delta time");

                                ImGui::NewLine();

                                if (ImGui::Button("Set Bar Searching Region"))				setBarSearchingRegion = !setBarSearchingRegion;
                                ImGui::SameLine(); helpMarker("Set the region to search for the bar.\n\nMove the box to the desired region by resizing and dragging it or ctrl + left clicking two opposite corners, then click \"Ok\".");
                                if (ImGui::Button("Set Bar Dead Zone Left Position"))		setBarDeadZoneLeftPosition = !setBarDeadZoneLeftPosition;
                                ImGui::SameLine(); helpMarker("Set the position of the left bar dead zone.\nWhen the line is on the left of this position, the macro will keep moving the bar to the left.\n\nMove the box to the desired position by dragging it or right clicking, then click \"Ok\".");
                                if (ImGui::Button("Set Bar Dead Zone Right Position"))		setBarDeadZoneRightPosition = !setBarDeadZoneRightPosition;
                                ImGui::SameLine(); helpMarker("Set the position of the right bar dead zone.\nWhen the line is on the right of this position, the macro will keep moving the bar to the right.\n\nMove the box to the desired position by dragging it or right clicking, then click \"Ok\".");
                            }
                            if (!config.config.autoBarMinigame) ImGui::EndDisabled();
                        }

                        break;
                    case 1:
                        ImGui::Checkbox("Auto Sell", &config.config.autoSell);
                        ImGui::SameLine(); helpMarker("Automatically sell all fish. Requires the \"Sell Anywhere\" gamepass.");
                        if (ImGui::Button("Set Sell Button Position")) setSellButtonPosition = !setSellButtonPosition;
                        ImGui::SameLine(); helpMarker("Set the position to click to activate the \"Sell Inventory\" button.\n\nMove the box to the desired position by dragging it or right clicking, then click \"Ok\".");

                        break;
                    case 2:
                        static int index = std::find(config.data.configsString.begin(), config.data.configsString.end(), config.data.configTXTname) != config.data.configsString.end()
                            ? static_cast<int>(std::distance(config.data.configsString.begin(), std::find(config.data.configsString.begin(), config.data.configsString.end(), config.data.configTXTname)))
                            : 0;
                        static int prevIndex = index;
                        ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 15);
                        ImGui::ListBox("", &index, config.data.configsCString.get(), static_cast<int>(config.data.configsString.size()));
                        if (index != prevIndex)
                        {
                            prevIndex = index;
                            config.data.configTXTname = config.data.configsString[index];
                        }

                        if (ImGui::Button("Refresh")) config.loadData();
                        ImGui::SameLine();
                        if (ImGui::Button("Load")) config.loadConfig();
                        ImGui::SameLine();
                        if (ImGui::Button("Save")) config.saveConfig();

                        break;
                    }
                }
                ImGui::EndChild();
            }
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
    hWnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW, wc.lpszClassName, L"fisch macro", WS_POPUP, robloxClientToScreenPoint.x, robloxClientToScreenPoint.y, robloxClientRect.right, robloxClientRect.bottom, nullptr, nullptr, wc.hInstance, this);
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

void Gui::helpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
