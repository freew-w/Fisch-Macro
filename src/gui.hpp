#pragma once

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class Gui
{
public:
    ~Gui();
    Gui(const Gui &) = delete;
    Gui(Gui &&) = delete;
    Gui &operator=(const Gui &) = delete;
    Gui &operator=(Gui &&) = delete;

    inline static Gui &get()
    {
        static Gui instance;
        return instance;
    }
    inline const bool &getIsRunning() const { return isRunning_; }

    inline void startRendering()
    {
        Fisch &fisch = Fisch::get();

        while (isRunning_)
        {
            MSG msg;
            while (PeekMessageW(&msg, nullptr, 0U, 0U, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
                if (msg.message == WM_QUIT)
                    isRunning_ = false;
            }

            static bool shouldShowOnNextFocus{};
            if (GetForegroundWindow() == fisch.getRobloxHWnd())
            {
                if (GetAsyncKeyState(VK_INSERT) & 0x01)
                    shouldRender_ = !shouldRender_;
                if (shouldShowOnNextFocus)
                {
                    shouldShowOnNextFocus = false;
                    shouldRender_ = true;
                }
            }
            else
            {
                if (shouldRender_)
                    shouldShowOnNextFocus = true;
                shouldRender_ = false;
            }
            if (shouldRender_)
            {
                SetWindowLongPtrW(hWnd_, GWL_EXSTYLE, GetWindowLongPtrW(hWnd_, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);
                fisch.updateRobloxClientRectAndRobloxClientToWindowPoint();
                SetWindowPos(hWnd_, HWND_TOPMOST, fisch.getRobloxClientToWindowPoint().x, fisch.getRobloxClientToWindowPoint().y, fisch.getRobloxClientRect().right, fisch.getRobloxClientRect().bottom, SWP_SHOWWINDOW);
            }
            else
                SetWindowLongPtrW(hWnd_, GWL_EXSTYLE, GetWindowLongPtrW(hWnd_, GWL_EXSTYLE) | WS_EX_TRANSPARENT);

            ImGui_ImplDX9_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
            render_();
            ImGui::EndFrame();

            d3dDevice_->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
            d3dDevice_->BeginScene();
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            d3dDevice_->EndScene();
            d3dDevice_->Present(nullptr, nullptr, nullptr, nullptr);
        }
    }

private:
    LPDIRECT3D9 d3d_{};
    LPDIRECT3DDEVICE9 d3dDevice_{};
    D3DPRESENT_PARAMETERS d3dpp_{};
    WNDCLASSEXW wc_{};
    HWND hWnd_{};
    bool isRunning_ = true;
    bool shouldRender_ = true;

    Gui();
    void resetDevice_();
    static LRESULT CALLBACK sWndProc_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    inline void render_()
    {
        if (shouldRender_)
        {
            ImGui::Begin("test", &isRunning_);
            ImGui::Text("Hello, world!");
            ImGui::End();
        }
        static Fisch &fisch = Fisch::get();
        ImDrawList *drawList = ImGui::GetBackgroundDrawList();
        std::vector<std::tuple<int, int, int, int, int>> detectedObjects = fisch.detectObjects();
        for (const auto &object : detectedObjects)
        {
            auto [x, y, w, h, classId] = object;
            drawList->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), ImColor(255, 0, 0), 0.0f, 0, 2.0f);
        }
    }

    inline LRESULT WndProc_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
            return true;

        switch (uMsg)
        {
        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED || !d3dDevice_)
                return 0;
            d3dpp_.BackBufferWidth = (UINT)LOWORD(lParam);
            d3dpp_.BackBufferHeight = (UINT)HIWORD(lParam);
            resetDevice_();
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU)
                return 0;
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
};
