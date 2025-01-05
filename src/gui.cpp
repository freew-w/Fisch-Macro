#include "pch.hpp"
#include "fisch.hpp"
#include "gui.hpp"

Gui::~Gui()
{
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    d3dDevice_->Release();
    d3dDevice_ = nullptr;
    d3d_->Release();
    d3d_ = nullptr;
    DestroyWindow(hWnd_);
    UnregisterClassW(wc_.lpszClassName, wc_.hInstance);
}

Gui::Gui()
{
    Fisch &fisch = Fisch::get();

    wc_.cbSize = sizeof(WNDCLASSEXW);
    wc_.style = CS_CLASSDC;
    wc_.lpfnWndProc = sWndProc_;
    wc_.cbClsExtra = 0;
    wc_.cbWndExtra = 0;
    wc_.hInstance = GetModuleHandleW(nullptr);
    wc_.hIcon = nullptr;
    wc_.hCursor = nullptr;
    wc_.hbrBackground = nullptr;
    wc_.lpszMenuName = nullptr;
    wc_.lpszClassName = L"fisch-macro";
    wc_.hIconSm = nullptr;

    RegisterClassExW(&wc_);
    hWnd_ = CreateWindowExW(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE, wc_.lpszClassName, L"fisch-macro", WS_POPUP, fisch.getRobloxClientToWindowPoint().x, fisch.getRobloxClientToWindowPoint().y, fisch.getRobloxClientRect().right, fisch.getRobloxClientRect().bottom, nullptr, nullptr, wc_.hInstance, this);
    SetLayeredWindowAttributes(hWnd_, RGB(0, 0, 0), 255, LWA_ALPHA);
    MARGINS margins{-1, -1, -1, -1};
    DwmExtendFrameIntoClientArea(hWnd_, &margins);
    ShowWindow(hWnd_, SW_SHOWDEFAULT);
    UpdateWindow(hWnd_);

    d3d_ = Direct3DCreate9(D3D_SDK_VERSION);
    d3dpp_.Windowed = TRUE;
    d3dpp_.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp_.BackBufferFormat = D3DFMT_A8R8G8B8;
    d3dpp_.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    d3d_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd_, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp_, &d3dDevice_);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;

    // ImGui::StyleColorsClassic();
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    ImGui_ImplWin32_Init(hWnd_);
    ImGui_ImplDX9_Init(d3dDevice_);
}

void Gui::resetDevice_()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    if (d3dDevice_->Reset(&d3dpp_) == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

LRESULT Gui::sWndProc_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    Gui *pThis = reinterpret_cast<Gui *>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));

    switch (uMsg)
    {
    case WM_NCCREATE:
        pThis = reinterpret_cast<Gui *>(reinterpret_cast<CREATESTRUCT *>(lParam)->lpCreateParams);
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        break;
    }

    if (pThis)
        return pThis->WndProc_(hWnd, uMsg, wParam, lParam);

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
