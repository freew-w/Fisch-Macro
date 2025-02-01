#pragma once

class Fisch
{
public:
    INPUT mouseMove{ .type = INPUT_MOUSE, .mi{1, 0, 0, MOUSEEVENTF_MOVE, 0, 0} };
    INPUT mouseScroll[2]{ {.type = INPUT_MOUSE, .mi{0, 0, static_cast<DWORD>(WHEEL_DELTA), MOUSEEVENTF_WHEEL, 0, 0}}, {.type = INPUT_MOUSE, .mi{0, 0, static_cast<DWORD>(-WHEEL_DELTA), MOUSEEVENTF_WHEEL, 0, 0}} };
    INPUT leftMouseClick[2]{ {.type = INPUT_MOUSE, .mi{0, 0, 0, MOUSEEVENTF_LEFTDOWN, 0, 0}}, {.type = INPUT_MOUSE, .mi{0, 0, 0, MOUSEEVENTF_LEFTUP, 0, 0}} };
    INPUT rightMouseClick[2]{ {.type = INPUT_MOUSE, .mi{0, 0, 0, MOUSEEVENTF_RIGHTDOWN, 0, 0}}, {.type = INPUT_MOUSE, .mi{0, 0, 0, MOUSEEVENTF_RIGHTUP, 0, 0}} };
    INPUT mClick[2]{ {.type = INPUT_KEYBOARD, .ki{0, static_cast<WORD>(MapVirtualKey(0x4d, MAPVK_VK_TO_VSC)), KEYEVENTF_SCANCODE, 0, 0}}, {.type = INPUT_KEYBOARD, .ki{0, static_cast<WORD>(MapVirtualKey(0x4d, MAPVK_VK_TO_VSC)), KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP, 0, 0}} };

    bool isRunning = true;
    bool enabled{};
    HWND robloxHWnd{};

    static Fisch& get();

    void warn(LPCWSTR msg);
    void error(LPCWSTR msg);

    cv::Mat screenshot(const ImRect& rect);

    void toggleCameraMode(const ImVec2& pos);
    void blurCamera();
    void lookDown();
    void zoomIn();
    void castRod();

    cv::Rect findShakeButton(cv::Mat mat);
    void clickShakeButton(const cv::Rect& rect);

    //cv::Rect findBar(cv::Mat mat, bool isArrowOnLeft = true);
    cv::Rect findLine(cv::Mat mat);
    cv::Rect findArrow(cv::Mat mat);
    //std::tuple<cv::Point, cv::Point> getArrowToBarDistanceAndBarWidthHeight(cv::Mat mat);

private:
    Fisch();
};

class SetPos
{
public:
    SetPos(ImVec2& pos, bool shouldShow);
};

class SetArea
{
public:
    SetArea(ImRect& rect, bool shouldShow);
};

inline Fisch& fisch = Fisch::get();
