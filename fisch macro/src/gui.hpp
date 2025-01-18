#pragma once

namespace gui
{
    inline HWND hWnd{};
    inline WNDCLASSEX wc{};
    inline LPDIRECT3D9 d3d{};
    inline LPDIRECT3DDEVICE9 d3dDevice{};
    inline D3DPRESENT_PARAMETERS d3dpp{};

    inline bool shouldRender = true;

    LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    bool init();
    void startRendering();
    void cleanup();
}
