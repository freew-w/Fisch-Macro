#include "pch.h"
#include "fisch.h"
#include "config.h"

Fisch& Fisch::get()
{
    static Fisch fisch{};
    return fisch;
}

void Fisch::warn(LPCWSTR msg)
{
    MessageBoxW(nullptr, msg, L"Error", MB_ICONWARNING);
}

void Fisch::error(LPCWSTR msg)
{
    MessageBoxW(nullptr, msg, L"Error", MB_ICONERROR);
    exit(EXIT_FAILURE);
}

void Fisch::startMacro()
{
    bool firstRun = true;
    bool newRun = true;

    while (isRunning)
    {
        cv::waitKey(1);
        if (GetAsyncKeyState(VK_F6) & 0x01) enabled = !enabled;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (!enabled)
        {
            firstRun = true;
            newRun = true;
            continue;
        }

        if (firstRun)
        {
            firstRun = false;
            if (config.config.autoEnableCameraMode) toggleCameraMode(config.coordinates.cameraModePosition);
            if (config.config.autoLookDown) lookDown();
            if (config.config.autoZoomIn) zoomIn();
        }

        cv::Rect shakeButtonRect = findShakeButton(screenshot(config.coordinates.searchShakeRect));
        if (shakeButtonRect.width)
        {
            failSafe(true);

            if (!config.config.autoShake)
                continue;

            clickShakeButton(shakeButtonRect);
            continue;
        }

        cv::Mat minigameMat = screenshot(config.coordinates.searchBarRect);
        auto [lineRect, arrowRect] = findLineAndArrow(minigameMat);
        if (lineRect.x && arrowRect.width)
        {
            failSafe(true);

            if (!config.config.autoBarMinigame)
                continue;

            if (newRun)
            {
                newRun = false;
                if (config.config.autoCalculateBarWidth)
                    config.config.barWidth = getBarWidth(minigameMat);
            }

            doBarMinigame(lineRect, arrowRect);
            continue;
        }

        if (failSafe())
        {
            newRun = true;
            if (config.config.autoSell) sell(config.coordinates.sellButtonPosition);
            if (config.config.autoBlur) blurCamera();
            if (config.config.autoLookDown) lookDown();
            castRod();
        }
    }
}

Fisch::Fisch()
{
    robloxHWnd = FindWindowW(nullptr, L"Roblox");
    if (!robloxHWnd)
        error(L"Roblox not found");
}

inline void Fisch::toggleCameraMode(const ImVec2& pos)
{
    POINT robloxClientToScreenPoint{};
    ClientToScreen(robloxHWnd, &robloxClientToScreenPoint);

    SetCursorPos(static_cast<int>(robloxClientToScreenPoint.x + pos.x), static_cast<int>(robloxClientToScreenPoint.y + pos.y));
    SendInput(1, &mouseMove, sizeof(INPUT));
    SendInput(2, leftMouseClick, sizeof(INPUT));
}

inline void Fisch::blurCamera()
{
    SendInput(2, mClick, sizeof(INPUT));
}

inline void Fisch::lookDown()
{
    RECT robloxClientRect;
    POINT robloxClientToScreenPoint{};
    GetClientRect(robloxHWnd, &robloxClientRect);
    ClientToScreen(robloxHWnd, &robloxClientToScreenPoint);

    SetCursorPos(robloxClientToScreenPoint.x + robloxClientRect.right / 2, robloxClientToScreenPoint.y + robloxClientRect.bottom / 2);
    SendInput(1, &mouseMove, sizeof(INPUT));
    SendInput(1, &rightMouseClick[0], sizeof(INPUT));
    INPUT input{ .type = INPUT_MOUSE, .mi{0, 800 / 50, 0, MOUSEEVENTF_MOVE, 0, 0} };
    for (int i{}; i < 50; i++) {
        SendInput(1, &input, sizeof(INPUT));
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    SendInput(1, &rightMouseClick[1], sizeof(INPUT));
}

inline void Fisch::zoomIn()
{
    RECT robloxClientRect;
    POINT robloxClientToScreenPoint{};
    GetClientRect(robloxHWnd, &robloxClientRect);
    ClientToScreen(robloxHWnd, &robloxClientToScreenPoint);

    SetCursorPos(robloxClientToScreenPoint.x + robloxClientRect.right / 2, robloxClientToScreenPoint.y + robloxClientRect.bottom / 2);
    for (int i{}; i < 40; i++)
    {
        SendInput(1, &mouseScroll[0], sizeof(INPUT));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    SendInput(1, &mouseScroll[1], sizeof(INPUT));
}

inline void Fisch::sell(const ImVec2& pos)
{
    POINT robloxClientToScreenPoint{};
    ClientToScreen(robloxHWnd, &robloxClientToScreenPoint);


    if (config.config.autoEnableCameraMode)
        toggleCameraMode(config.coordinates.cameraModePosition);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    SendInput(2, graveClick, sizeof(INPUT));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    SetCursorPos(static_cast<int>(robloxClientToScreenPoint.x + pos.x), static_cast<int>(robloxClientToScreenPoint.y + pos.y));
    SendInput(1, &mouseMove, sizeof(INPUT));
    SendInput(2, leftMouseClick, sizeof(INPUT));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    SendInput(2, graveClick, sizeof(INPUT));
    if (config.config.autoEnableCameraMode)
        toggleCameraMode(config.coordinates.cameraModePosition);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

inline void Fisch::castRod()
{
    RECT robloxClientRect;
    POINT robloxClientToScreenPoint{};
    GetClientRect(robloxHWnd, &robloxClientRect);
    ClientToScreen(robloxHWnd, &robloxClientToScreenPoint);

    SetCursorPos(robloxClientToScreenPoint.x + robloxClientRect.right / 2, robloxClientToScreenPoint.y + robloxClientRect.bottom / 2);
    SendInput(1, &mouseMove, sizeof(INPUT));
    SendInput(1, &leftMouseClick[0], sizeof(INPUT));
    std::this_thread::sleep_for(std::chrono::milliseconds(config.config.castTime));
    SendInput(1, &leftMouseClick[1], sizeof(INPUT));
}

inline cv::Mat Fisch::screenshot(const ImRect& rect)
{
    static HDC hScreenDC = GetDC(nullptr);
    static HDC hMemDC = CreateCompatibleDC(hScreenDC);
    static HBITMAP hBitmap{};
    static int prevW{}, prevH{};

    POINT robloxClientToScreenPoint{};
    ClientToScreen(robloxHWnd, &robloxClientToScreenPoint);
    int w = static_cast<int>(rect.Max.x - rect.Min.x);
    int h = static_cast<int>(rect.Max.y - rect.Min.y);

    if (prevW != w || prevH != h)
    {
        if (hBitmap)
            DeleteObject(hBitmap);
        hBitmap = CreateCompatibleBitmap(hScreenDC, w, h);
        prevW = w;
        prevH = h;
    }

    HGDIOBJ hOldBitmap = SelectObject(hMemDC, hBitmap);
    BitBlt(hMemDC, 0, 0, w, h, hScreenDC, static_cast<int>(robloxClientToScreenPoint.x + rect.Min.x), static_cast<int>(robloxClientToScreenPoint.y + rect.Min.y), SRCCOPY);

    cv::Mat mat(h, w, CV_8UC4);
    BITMAPINFOHEADER bih = { sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB };
    GetDIBits(hMemDC, hBitmap, 0, h, mat.data, reinterpret_cast<BITMAPINFO*>(&bih), DIB_RGB_COLORS);
    SelectObject(hMemDC, hOldBitmap);

    return mat;
}

inline cv::Rect Fisch::findShakeButton(cv::Mat mat)
{
    cv::cvtColor(mat, mat, cv::COLOR_RGBA2GRAY);
    cv::dilate(mat, mat, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));
    cv::Canny(mat, mat, 200, 300);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mat, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (auto& contour : contours)
    {
        double area = cv::contourArea(contour);
        if (area < config.config.minimumShakeButtonArea || area > config.config.maximumShakeButtonArea || contour.size() < 100)
            continue;
        cv::approxPolyDP(contour, contour, 0.05 * cv::arcLength(contour, true), true);

        RECT robloxClientRect;
        GetClientRect(robloxHWnd, &robloxClientRect);
        cv::Rect rect = cv::boundingRect(contour);
        rect.x += static_cast<int>(config.coordinates.searchShakeRect.Min.x - robloxClientRect.left);
        rect.y += static_cast<int>(config.coordinates.searchShakeRect.Min.y - robloxClientRect.top);
        return rect;
    }

    return cv::Rect();
}

inline int Fisch::getBarWidth(cv::Mat mat)
{
    cv::cvtColor(mat, mat, cv::COLOR_RGBA2GRAY);
    cv::morphologyEx(mat, mat, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));
    cv::GaussianBlur(mat, mat, cv::Size(3, 3), 0, 0);
    cv::threshold(mat, mat, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    cv::Canny(mat, mat, 50, 150);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mat, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<cv::Rect> rects;
    for (auto& contour : contours)
    {
        cv::approxPolyDP(contour, contour, 0.05 * cv::arcLength(contour, true), true);
        if (contour.size() != 4)
            continue;
        double area = cv::contourArea(contour);
        if (area < 50 || area > 10000)
            continue;
        rects.emplace_back(cv::boundingRect(contour));
    }
    if (rects.empty())
        return 0;

    int xMin = rects[0].x, xMax{};
    for (const auto& rect : rects)
    {
        xMin = std::min(xMin, rect.x);
        xMax = std::max(xMax, rect.x + rect.width);
    }

    return xMax - xMin;
}

inline std::pair<cv::Rect, cv::Rect> Fisch::findLineAndArrow(cv::Mat mat)
{
    RECT robloxClientRect;
    GetClientRect(robloxHWnd, &robloxClientRect);
    int offsetX = static_cast<int>(config.coordinates.searchBarRect.Min.x - robloxClientRect.left);
    int offsetY = static_cast<int>(config.coordinates.searchBarRect.Min.y - robloxClientRect.top);

    cv::cvtColor(mat, mat, cv::COLOR_BGR2HSV);

    cv::Mat lineMat;
    cv::inRange(mat, cv::Scalar(110, 50, 80), cv::Scalar(110, 70, 100), lineMat);
    cv::medianBlur(lineMat, lineMat, 3);
    std::vector<std::vector<cv::Point>> lineContours;
    cv::findContours(lineMat, lineContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    cv::Rect lineRect;
    for (auto& contour : lineContours)
    {
        cv::approxPolyDP(contour, contour, 0.02 * cv::arcLength(contour, true), true);
        lineRect = cv::boundingRect(contour);
        lineRect.x += offsetX;
        lineRect.y += offsetY;
    }

    cv::Mat arrowMat;
    cv::inRange(mat, cv::Scalar(0, 0, 120), cv::Scalar(180, 20, 140), arrowMat);
    cv::medianBlur(arrowMat, arrowMat, 3);
    std::vector<std::vector<cv::Point>> arrowContours;
    cv::findContours(arrowMat, arrowContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    cv::Rect arrowRect;
    for (auto& contour : arrowContours)
    {
        cv::approxPolyDP(contour, contour, 0.02 * cv::arcLength(contour, true), true);
        if (contour.size() < 5 || cv::contourArea(contour) < 20)
            continue;
        arrowRect = cv::boundingRect(contour);
        arrowRect.x += offsetX;
        arrowRect.y += offsetY;
    }

    return { lineRect, arrowRect };
}

inline void Fisch::clickShakeButton(const cv::Rect& rect)
{
    POINT robloxClientToScreenPoint{};
    ClientToScreen(robloxHWnd, &robloxClientToScreenPoint);

    if (!config.config.checkClickShakePosition)
    {
        SetCursorPos(robloxClientToScreenPoint.x + rect.x + rect.width / 2, robloxClientToScreenPoint.y + rect.y + rect.height / 2);
        SendInput(1, &mouseMove, sizeof(INPUT));
        SendInput(2, leftMouseClick, sizeof(INPUT));
        std::this_thread::sleep_for(std::chrono::milliseconds(config.config.clickShakeDelay));
        return;
    }

    int posX = rect.x + rect.width / 2;
    int posY = rect.y + rect.height / 2;
    static int lastPosX{}, lastPosY{};
    if (abs(lastPosX - posX) <= 10 && abs(lastPosY - posY) <= 10)
        return;
    lastPosX = posX;
    lastPosY = posY;

    SetCursorPos(robloxClientToScreenPoint.x + posX, robloxClientToScreenPoint.y + posY);
    SendInput(1, &mouseMove, sizeof(INPUT));
    SendInput(2, leftMouseClick, sizeof(INPUT));
}

inline void Fisch::doBarMinigame(const cv::Rect& lineRect, const cv::Rect& arrowRect)
{
    static auto lastLoopTime = std::chrono::steady_clock::now();

    auto currentTime = std::chrono::steady_clock::now();
    auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastLoopTime).count();

    static double prevError{};

    double error = lineRect.x - arrowRect.x;
    double derivative = (error - prevError) / deltaTime;
    int output = static_cast<int>(config.config.kp * error + config.config.kd * derivative);
    output = std::clamp(output, -config.config.barWidth / 2, config.config.barWidth / 2);

    if (config.config.useBarDeadZoneLeft && lineRect.x < config.coordinates.barDeadZoneLeftPosition.x)
    {
        SendInput(1, &fisch.leftMouseClick[1], sizeof(INPUT));
        return;
    }
    else if (config.config.useBarDeadZoneRight && lineRect.x > config.coordinates.barDeadZoneRightPosition.x)
    {
        SendInput(1, &fisch.leftMouseClick[0], sizeof(INPUT));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return;
    }

    if (output > 0)
        SendInput(1, &leftMouseClick[0], sizeof(INPUT));
    else
        SendInput(1, &leftMouseClick[1], sizeof(INPUT));

    std::this_thread::sleep_for(std::chrono::milliseconds(abs(output)));

    prevError = error;
    lastLoopTime = currentTime;
}

inline bool Fisch::failSafe(bool reset)
{
    static int failSafe{};
    if (reset)
    {
        failSafe = 0;
        return false;
    }

    static auto lastIncrementTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();

    if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastIncrementTime).count() >= 1)
    {
        failSafe++;
        lastIncrementTime = currentTime;
    }

    if (failSafe >= config.config.failSafeCount)
    {
        failSafe = 0;
        return true;
    }
    return false;
}
