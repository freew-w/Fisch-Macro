#pragma once

class Gui
{
public:
    inline static Gui& getInstance() { static Gui gui{}; return gui; }

    void renderFrame();

private:
    HWND hWnd_{};
    WNDCLASSEX wc_{};
    LPDIRECT3D9 d3d_{};
    LPDIRECT3DDEVICE9 d3dDevice_{};
    D3DPRESENT_PARAMETERS d3dpp_{};

    Gui();
    ~Gui();

    static LRESULT CALLBACK sWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void positionSetter(Position& position, bool& shouldRender) const;
    void regionSetter(Region& region, bool& shouldRender) const;
    void helpMarker(const char* desc) const;

    std::pair<bool, bool> shouldRender() const;
    void beginRendering() const;
    void endRendering() const;
    void renderMainUI() const;
    void renderInfoUI() const;
};
