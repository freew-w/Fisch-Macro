#include "pch.h"
#include "config.h"
#include "fisch.h"
#include "gui.h"
#include "roblox.h"

void Gui::renderFrame()
{
    auto [shouldRenderMainUI, shouldRenderInfoUI] = shouldRender();

    beginRendering();
    if (shouldRenderMainUI)
        renderMainUI();
    if (shouldRenderInfoUI && Config::getInstance().getConfig().showInfoUI)
        renderInfoUI();
    endRendering();
}

Gui::Gui()
{
    wc_.cbClsExtra = 0;
    wc_.cbSize = sizeof(WNDCLASSEX);
    wc_.cbWndExtra = 0;
    wc_.hbrBackground = nullptr;
    wc_.hCursor = nullptr;
    wc_.hIcon = nullptr;
    wc_.hIconSm = nullptr;
    wc_.hInstance = GetModuleHandleW(nullptr);
    wc_.lpfnWndProc = sWndProc;
    wc_.lpszClassName = L"Fisch Macro";
    wc_.lpszMenuName = nullptr;
    wc_.style = CS_CLASSDC;
    RegisterClassExW(&wc_);

    POINT robloxWindowPosition = Roblox::getInstance().getRobloxWindowPosition(),
        robloxWindowSize = Roblox::getInstance().getRobloxWindowSize();
    hWnd_ = CreateWindowExW(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW, wc_.lpszClassName, L"Fisch Macro", WS_POPUP, robloxWindowPosition.x, robloxWindowPosition.y, robloxWindowSize.x, robloxWindowSize.y, nullptr, nullptr, wc_.hInstance, this);
    if (!hWnd_)
        throw std::runtime_error("Failed to create window");
    SetLayeredWindowAttributes(hWnd_, RGB(0, 0, 0), 255, LWA_ALPHA);
    MARGINS margins{ -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(hWnd_, &margins);

    d3d_ = Direct3DCreate9(D3D_SDK_VERSION);
    d3dpp_.Windowed = TRUE;
    d3dpp_.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp_.BackBufferFormat = D3DFMT_A8R8G8B8;
    d3dpp_.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    if (FAILED(d3d_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd_, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp_, &d3dDevice_)))
        throw std::runtime_error("Failed to create device");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hWnd_);
    ImGui_ImplDX9_Init(d3dDevice_);

    ShowWindow(hWnd_, SW_SHOWDEFAULT);
    UpdateWindow(hWnd_);
}

Gui::~Gui()
{
    if (d3dDevice_)
        d3dDevice_->Release();
    if (d3d_)
        d3d_->Release();
    if (hWnd_)
        DestroyWindow(hWnd_);
    if (wc_.lpszClassName)
        UnregisterClassW(wc_.lpszClassName, wc_.hInstance);
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
        if (wParam == SIZE_MINIMIZED || !d3dDevice_)
            return 0;
        d3dpp_.BackBufferWidth = static_cast<UINT>(LOWORD(lParam));
        d3dpp_.BackBufferHeight = static_cast<UINT>(HIWORD(lParam));
        ImGui_ImplDX9_InvalidateDeviceObjects();
        if (d3dDevice_->Reset(&d3dpp_) == D3DERR_INVALIDCALL)
            IM_ASSERT(0);
        ImGui_ImplDX9_CreateDeviceObjects();
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

void Gui::positionSetter(Position& position, bool& shouldRender) const
{
    if (!shouldRender)
        return;

    int id = sizeof(position) + position.x + position.y;
    static int prevId{};
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    ImGui::PushID(id);
    ImGui::Begin("set position", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
    {
        if (id != prevId)
        {
            prevId = id;
            ImGui::SetWindowPos({ position.x - ImGui::GetWindowSize().x / 2.0f, position.y - ImGui::GetWindowSize().y / 2.0f });
        }

        if (GetAsyncKeyState(VK_RBUTTON) & 0x01)
        {
            POINT cursorPoint;
            GetCursorPos(&cursorPoint);
            ScreenToClient(Roblox::getInstance().getRobloxHWnd(), &cursorPoint);

            ImGui::SetWindowPos({ cursorPoint.x - ImGui::GetWindowSize().x / 2.0f, cursorPoint.y - ImGui::GetWindowSize().y / 2.0f });
        }

        if (ImGui::Button("Ok"))
        {
            position.x = ImGui::GetWindowPos().x + ImGui::GetWindowSize().x / 2.0f;
            position.y = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y / 2.0f;
            id = sizeof(position) + position.x + position.y;
            prevId = id;
            shouldRender = false;
        }
    }
    ImGui::End();
    ImGui::PopID();
    ImGui::PopStyleVar();
}

void Gui::regionSetter(Region& region, bool& shouldRender) const
{
    if (!shouldRender)
        return;

    int id = sizeof(region) + region.min.x + region.min.y + region.max.x + region.max.y;
    static int prevId{};
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    ImGui::PushID(id);
    ImGui::Begin("set area", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
    {

        if (id != prevId)
        {
            prevId = id;
            ImGui::SetWindowPos({ static_cast<float>(region.min.x) , static_cast<float>(region.min.y) });
            ImGui::SetWindowSize({ static_cast<float>(region.max.x - region.min.x) , static_cast<float>(region.max.y - region.min.y) });
        }

        if (GetAsyncKeyState(VK_CONTROL) & 0x8000 && GetAsyncKeyState(VK_LBUTTON) & 0x8000)
        {
            while (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
            POINT cursorPoint;
            GetCursorPos(&cursorPoint);
            ScreenToClient(Roblox::getInstance().getRobloxHWnd(), &cursorPoint);

            static std::array<Position, 2> points{};
            for (int i{}; i < points.size(); i++)
            {
                if (!points[i].x && !points[i].y)
                {
                    points[i].x = cursorPoint.x;
                    points[i].y = cursorPoint.y;

                    if (i == 0)
                        break;
                    if (i == points.size() - 1)
                    {
                        ImGui::SetWindowPos({ static_cast<float>(std::min(points[0].x, points[1].x)), static_cast<float>(std::min(points[0].y, points[1].y)) });
                        ImGui::SetWindowSize({ static_cast<float>(std::max(points[0].x, points[1].x) - std::min(points[0].x, points[1].x)), static_cast<float>(std::max(points[0].y, points[1].y) - std::min(points[0].y, points[1].y)) });
                        points = {};
                    }
                }
            }
        }

        ImGui::SetCursorPos({ ImGui::GetWindowWidth() / 2 - 100, ImGui::GetWindowHeight() / 2 - 40 });
        if (ImGui::Button("Ok", { 200, 80 }))
        {
            region.min.x = ImGui::GetWindowPos().x;
            region.min.y = ImGui::GetWindowPos().y;
            region.max.x = ImGui::GetWindowPos().x + ImGui::GetWindowSize().x;
            region.max.y = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y;
            id = sizeof(region) + region.min.x + region.min.y + region.max.x + region.max.y;
            prevId = id;
            shouldRender = false;
        }
    }
    ImGui::End();
    ImGui::PopID();
    ImGui::PopStyleVar();
}

void Gui::helpMarker(const char* desc) const
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

std::pair<bool, bool> Gui::shouldRender() const
{
    static bool shouldRenderMainUI{}, shouldRenderInfoUI = Config::getInstance().getConfig().showInfoUI;
    static bool shouldRenderMainUIOnNextFocus{}, shouldRenderInfoUIOnNextFocus{};

    if (GetForegroundWindow() == Roblox::getInstance().getRobloxHWnd() || GetForegroundWindow() == hWnd_)
    {
        if (GetAsyncKeyState(VK_INSERT) & 0x01)
        {
            shouldRenderMainUI = !shouldRenderMainUI;
            if (shouldRenderMainUI)
                SetForegroundWindow(hWnd_);
            else
                SetForegroundWindow(Roblox::getInstance().getRobloxHWnd());
        }
        if (shouldRenderMainUIOnNextFocus)
        {
            shouldRenderMainUIOnNextFocus = false;
            shouldRenderMainUI = true;
        }
        if (shouldRenderInfoUIOnNextFocus)
        {
            shouldRenderInfoUIOnNextFocus = false;
            shouldRenderInfoUI = true;
        }
        if (Config::getInstance().getConfig().showInfoUI)
            shouldRenderInfoUI = true;
    }
    else
    {
        if (shouldRenderMainUI)
            shouldRenderMainUIOnNextFocus = true;
        if (shouldRenderInfoUI)
            shouldRenderInfoUIOnNextFocus = true;
        shouldRenderMainUI = false;
        shouldRenderInfoUI = false;
    }
    if (shouldRenderMainUI)
    {
        POINT robloxWindowPosition = Roblox::getInstance().getRobloxWindowPosition(),
            robloxWindowSize = Roblox::getInstance().getRobloxWindowSize();

        SetWindowLongPtrW(hWnd_, GWL_EXSTYLE, GetWindowLongPtr(hWnd_, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);
        SetWindowPos(hWnd_, HWND_TOPMOST, robloxWindowPosition.x, robloxWindowPosition.y, robloxWindowSize.x, robloxWindowSize.y, SWP_SHOWWINDOW);
    }
    else
        SetWindowLongPtrW(hWnd_, GWL_EXSTYLE, GetWindowLongPtr(hWnd_, GWL_EXSTYLE) | WS_EX_TRANSPARENT);

    return { shouldRenderMainUI, shouldRenderInfoUI };
}

void Gui::beginRendering() const
{
    MSG msg;
    while (PeekMessageW(&msg, hWnd_, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void Gui::endRendering() const
{
    ImGui::EndFrame();

    d3dDevice_->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
    d3dDevice_->BeginScene();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    d3dDevice_->EndScene();
    d3dDevice_->Present(nullptr, nullptr, nullptr, nullptr);
}

void Gui::renderMainUI() const
{
    static bool setCameraModePosition{},
        setShakeButtonSearchingRegion{},
        setBarSearchingRegion{},
        setBarDeadZoneLeftPosition{},
        setBarDeadZoneRightPosition{},
        setSellButtonPosition{};

    positionSetter(Config::getInstance().getPositions().cameraModePosition, setCameraModePosition);
    regionSetter(Config::getInstance().getPositions().searchShakeRegion, setShakeButtonSearchingRegion);
    regionSetter(Config::getInstance().getPositions().searchBarRegion, setBarSearchingRegion);
    positionSetter(Config::getInstance().getPositions().barDeadZoneLeftPosition, setBarDeadZoneLeftPosition);
    positionSetter(Config::getInstance().getPositions().barDeadZoneRightPosition, setBarDeadZoneRightPosition);
    positionSetter(Config::getInstance().getPositions().sellButtonPosition, setSellButtonPosition);

    ImGui::SetNextWindowSize({ 580, 380 }, ImGuiCond_Once);
    if (ImGui::Begin("Fisch Macro v0.3.0 (insert to show/hide)", &fisch::isRunning))
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
            switch (page)
            {
            case 0:
                ImGui::Text("Macro");
                ImGui::Separator();
                {
                    ImGui::Checkbox("Enabled(F6)", &fisch::enabled);
                }

                ImGui::Text("Auto Shake");
                ImGui::Separator();
                {
                    ImGui::PushID("Auto Shake Enabled");
                    ImGui::Checkbox("Enabled", &Config::getInstance().getConfig().autoShake);
                    ImGui::PopID();

                    ImGui::NewLine();

                    if (!Config::getInstance().getConfig().autoShake) ImGui::BeginDisabled();
                    {
                        ImGui::SetNextItemWidth(100);
                        ImGui::InputInt("Fail Safe Threshold", &Config::getInstance().getConfig().failSafeThreshold);
                        ImGui::SameLine(); helpMarker("Threshold for fail safe count.\nMacro will restart if the threshold is reached.\n\nFail safe count increases if no shake button/bar/line is found, with a 1 second cooldown.");

                        ImGui::NewLine();

                        ImGui::SetNextItemWidth(100);
                        ImGui::InputInt("Cast Time", &Config::getInstance().getConfig().castTime);
                        ImGui::SameLine(); helpMarker("The hold time(ms) for casting the rod.");

                        ImGui::NewLine();

                        ImGui::Checkbox("Auto Enable Camera Mode", &Config::getInstance().getConfig().autoEnableCameraMode);
                        if (!Config::getInstance().getConfig().autoEnableCameraMode) { ImGui::BeginDisabled(); Config::getInstance().getConfig().autoBlur = false; }
                        {
                            ImGui::Checkbox("Auto Blur", &Config::getInstance().getConfig().autoBlur);
                        }
                        if (!Config::getInstance().getConfig().autoEnableCameraMode) ImGui::EndDisabled();
                        ImGui::Checkbox("Auto Look Down", &Config::getInstance().getConfig().autoLookDown);
                        ImGui::Checkbox("Auto Zoom In", &Config::getInstance().getConfig().autoZoomIn);

                        ImGui::NewLine();

                        ImGui::SetNextItemWidth(100);
                        ImGui::InputInt("Minimum Shake Button Area", &Config::getInstance().getConfig().minimumShakeButtonArea);
                        ImGui::SetNextItemWidth(100);
                        ImGui::InputInt("Maximum Shake Button Area", &Config::getInstance().getConfig().maximumShakeButtonArea);

                        ImGui::NewLine();

                        ImGui::Checkbox("Check Click Position", &Config::getInstance().getConfig().checkClickShakePosition);
                        if (Config::getInstance().getConfig().checkClickShakePosition)
                        {
                            ImGui::SameLine();
                            helpMarker("Prevent clicking the same position multiple times by comparing the current position with the last position.\nIf enabled, the click shake delay will be ignored.");
                        }
                        else
                        {
                            ImGui::SameLine();
                            ImGui::SetNextItemWidth(100);
                            ImGui::InputInt("Click Shake Delay", &Config::getInstance().getConfig().clickShakeDelay);
                            ImGui::SameLine();
                            helpMarker("Prevent clicking the same position multiple times by adding a delay(ms) between each click.");
                        }

                        ImGui::NewLine();

                        if (ImGui::Button("Set Camera Mode Button Position"))		setCameraModePosition = !setCameraModePosition;
                        ImGui::SameLine(); helpMarker("Set the position to click to enable camera mode.\n\nMove the box to the desired position by dragging it or right clicking, then click \"Ok\".");
                        if (ImGui::Button("Set Shake Button Searching Region"))		setShakeButtonSearchingRegion = !setShakeButtonSearchingRegion;
                        ImGui::SameLine(); helpMarker("Set the region to search for the shake button.\n\nMove the box to the desired region by resizing and dragging it or ctrl + left clicking two opposite corners, then click \"Ok\".");
                    }
                    if (!Config::getInstance().getConfig().autoShake) ImGui::EndDisabled();
                }

                ImGui::Text("Auto Bar Minigame");
                ImGui::Separator();
                {
                    ImGui::PushID("Auto Bar Minigame Enabled");
                    ImGui::Checkbox("Enabled", &Config::getInstance().getConfig().autoBarMinigame);
                    ImGui::PopID();

                    ImGui::NewLine();

                    if (!Config::getInstance().getConfig().autoBarMinigame) ImGui::BeginDisabled();
                    {
                        ImGui::Checkbox("Auto Calculate Bar Width", &Config::getInstance().getConfig().autoCalculateBarWidth);
                        if (!Config::getInstance().getConfig().autoCalculateBarWidth)
                        {
                            ImGui::SameLine();
                            ImGui::SetNextItemWidth(100);
                            ImGui::InputInt("Bar Width", &Config::getInstance().getConfig().barWidth);
                        }

                        ImGui::NewLine();

                        ImGui::Checkbox("Use Bar Dead Zone Left", &Config::getInstance().getConfig().useBarDeadZoneLeft);
                        ImGui::Checkbox("Use Bar Dead Zone Right", &Config::getInstance().getConfig().useBarDeadZoneRight);

                        ImGui::NewLine();

                        ImGui::SetNextItemWidth(150);
                        ImGui::InputDouble("Kp", &Config::getInstance().getConfig().kp, 0.1, 1.0);
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(150);
                        ImGui::InputDouble("Kd", &Config::getInstance().getConfig().kd, 0.1, 1.0);
                        ImGui::SameLine();helpMarker("Adjust the proportional and derivative gain of the PD controller.\n\nKp: proportional gain\nKd: derivative gain\nError = line x - arrow x\nDerivative = (error - previous error) / delta time");

                        ImGui::NewLine();

                        if (ImGui::Button("Set Bar Searching Region"))				setBarSearchingRegion = !setBarSearchingRegion;
                        ImGui::SameLine(); helpMarker("Set the region to search for the bar.\n\nMove the box to the desired region by resizing and dragging it or ctrl + left clicking two opposite corners, then click \"Ok\".");
                        if (ImGui::Button("Set Bar Dead Zone Left Position"))		setBarDeadZoneLeftPosition = !setBarDeadZoneLeftPosition;
                        ImGui::SameLine(); helpMarker("Set the position of the left bar dead zone.\nWhen the line is on the left of this position, the macro will keep moving the bar to the left.\n\nMove the box to the desired position by dragging it or right clicking, then click \"Ok\".");
                        if (ImGui::Button("Set Bar Dead Zone Right Position"))		setBarDeadZoneRightPosition = !setBarDeadZoneRightPosition;
                        ImGui::SameLine(); helpMarker("Set the position of the right bar dead zone.\nWhen the line is on the right of this position, the macro will keep moving the bar to the right.\n\nMove the box to the desired position by dragging it or right clicking, then click \"Ok\".");
                    }
                    if (!Config::getInstance().getConfig().autoBarMinigame) ImGui::EndDisabled();
                }

                break;
            case 1:
                ImGui::Text("Auto Sell");
                ImGui::Separator();
                {
                    ImGui::Checkbox("Auto Sell", &Config::getInstance().getConfig().autoSell);
                    ImGui::SameLine(); helpMarker("Automatically sell all fish. Requires the \"Sell Anywhere\" gamepass.");
                    if (ImGui::Button("Set Sell Button Position")) setSellButtonPosition = !setSellButtonPosition;
                    ImGui::SameLine(); helpMarker("Set the position to click to activate the \"Sell Inventory\" button.\n\nMove the box to the desired position by dragging it or right clicking, then click \"Ok\".");
                }

                ImGui::Text("Info UI");
                ImGui::Separator();
                {
                    ImGui::Checkbox("Show Info UI", &Config::getInstance().getConfig().showInfoUI);
                    ImGui::Checkbox("Show Run Time", &Config::getInstance().getConfig().showRunTime);
                    ImGui::Checkbox("Show Fail Safe Count", &Config::getInstance().getConfig().showFailSafeCount);
                    ImGui::Checkbox("Show Bar Minigame Hold Duration", &Config::getInstance().getConfig().showBarMinigameHoldDuration);
                }

                break;
            case 2:
                auto [size, configs] = Config::getInstance().getConfigs();
                int& index = Config::getInstance().getSelectedConfigIndex();
                ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 15);
                ImGui::ListBox("", &index, configs, size);

                if (ImGui::Button("Load")) Config::getInstance().userLoad();
                ImGui::SameLine();
                if (ImGui::Button("Save")) Config::getInstance().userSave();

                break;
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void Gui::renderInfoUI() const
{
    if (ImGui::Begin("Info UI"))
    {
        if (Config::getInstance().getConfig().showRunTime) ImGui::Text("Run Time: %d : %d : %d", fisch::runTimeHours, fisch::runTimeMinutes, fisch::runTimeSeconds);
        if (Config::getInstance().getConfig().showFailSafeCount) ImGui::Text("Fail Safe Count: %d", fisch::failSafeCount);
        if (Config::getInstance().getConfig().showBarMinigameHoldDuration) ImGui::Text("Bar Minigame Hold Duration: %d", fisch::output);
    }
    ImGui::End();
}
