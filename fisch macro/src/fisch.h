#pragma once

class Fisch
{
public:
    bool isRunning = true;
    bool enabled{};
    HWND robloxHWnd{};

    static Fisch& get();

    void warn(LPCWSTR msg);
    void error(LPCWSTR msg);

    void startMacro();

    void setRegion(ImRect& rect, bool& shouldShow);
    void setPosition(ImVec2& pos, bool& shouldShow);

private:
    INPUT mouseMove{ .type = INPUT_MOUSE, .mi{1, 0, 0, MOUSEEVENTF_MOVE, 0, 0} };
    INPUT mouseScroll[2]{ {.type = INPUT_MOUSE, .mi{0, 0, static_cast<DWORD>(WHEEL_DELTA), MOUSEEVENTF_WHEEL, 0, 0}}, {.type = INPUT_MOUSE, .mi{0, 0, static_cast<DWORD>(-WHEEL_DELTA), MOUSEEVENTF_WHEEL, 0, 0}} };
    INPUT leftMouseClick[2]{ {.type = INPUT_MOUSE, .mi{0, 0, 0, MOUSEEVENTF_LEFTDOWN, 0, 0}}, {.type = INPUT_MOUSE, .mi{0, 0, 0, MOUSEEVENTF_LEFTUP, 0, 0}} };
    INPUT rightMouseClick[2]{ {.type = INPUT_MOUSE, .mi{0, 0, 0, MOUSEEVENTF_RIGHTDOWN, 0, 0}}, {.type = INPUT_MOUSE, .mi{0, 0, 0, MOUSEEVENTF_RIGHTUP, 0, 0}} };
    INPUT mClick[2]{ {.type = INPUT_KEYBOARD, .ki{0, static_cast<WORD>(MapVirtualKey(0x4d, MAPVK_VK_TO_VSC)), KEYEVENTF_SCANCODE, 0, 0}}, {.type = INPUT_KEYBOARD, .ki{0, static_cast<WORD>(MapVirtualKey(0x4d, MAPVK_VK_TO_VSC)), KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP, 0, 0}} };
    INPUT graveClick[2]{ {.type = INPUT_KEYBOARD, .ki{0, static_cast<WORD>(MapVirtualKey(VK_OEM_3, MAPVK_VK_TO_VSC)), KEYEVENTF_SCANCODE, 0, 0}}, {.type = INPUT_KEYBOARD, .ki{0, static_cast<WORD>(MapVirtualKey(VK_OEM_3, MAPVK_VK_TO_VSC)), KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP, 0, 0}} };

    Fisch();

    inline void toggleCameraMode(const ImVec2& pos);
    inline void blurCamera();
    inline void lookDown();
    inline void zoomIn();
    inline void sell(const ImVec2& pos);
    inline void castRod();

    inline cv::Mat screenshot(const ImRect& rect);

    inline cv::Rect findShakeButton(cv::Mat mat);
    inline int getBarWidth(cv::Mat mat);
    inline std::pair<cv::Rect, cv::Rect> findLineAndArrow(cv::Mat mat);

    inline void clickShakeButton(const cv::Rect& rect);
    inline void doBarMinigame(const cv::Rect& lineRect, const cv::Rect& arrowRect);
    inline bool failSafe(bool reset = false);
};

inline Fisch& fisch = Fisch::get();
