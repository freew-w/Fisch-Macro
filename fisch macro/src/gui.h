#pragma once

class Gui
{
public:
	static Gui& get();
	void startRendering();

private:
	bool shouldRender = true;
	HWND hWnd{};
	WNDCLASSEX wc{};
	LPDIRECT3D9 d3d{};
	LPDIRECT3DDEVICE9 d3dDevice{};
	D3DPRESENT_PARAMETERS d3dpp{};


	Gui();
	~Gui();
	static LRESULT CALLBACK sWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

inline Gui& gui = Gui::get();
