#include "pch.h"
#include "config.h"
#include "fisch.h"
#include "input.h"
#include "roblox.h"

cv::Mat fisch::screenshot(const Region& region)
{
    static HDC hScreenDC = GetDC(nullptr);
    static HDC hMemDC = CreateCompatibleDC(hScreenDC);
    static HBITMAP hBitmap{};
    static int prevW{}, prevH{};

    POINT robloxWindowPosition = Roblox::getInstance().getRobloxWindowPosition();
    int w = region.max.x - region.min.x;
    int h = region.max.y - region.min.y;

    if (prevW != w || prevH != h)
    {
        if (hBitmap)
            DeleteObject(hBitmap);
        hBitmap = CreateCompatibleBitmap(hScreenDC, w, h);
        prevW = w;
        prevH = h;
    }

    HGDIOBJ hOldBitmap = SelectObject(hMemDC, hBitmap);
    BitBlt(hMemDC, 0, 0, w, h, hScreenDC, robloxWindowPosition.x + region.min.x, robloxWindowPosition.y + region.min.y, SRCCOPY);

    cv::Mat mat(h, w, CV_8UC4);
    BITMAPINFOHEADER bih = { sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB };
    GetDIBits(hMemDC, hBitmap, 0, h, mat.data, reinterpret_cast<BITMAPINFO*>(&bih), DIB_RGB_COLORS);
    SelectObject(hMemDC, hOldBitmap);

    return mat;
}

void fisch::toggleCameraMode()
{
    POINT robloxWindowPosition = Roblox::getInstance().getRobloxWindowPosition();

    input::moveMouse(robloxWindowPosition.x + Config::getInstance().getPositions().cameraModePosition.x, robloxWindowPosition.y + Config::getInstance().getPositions().cameraModePosition.y);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    input::clickMouse(input::MouseButton::left);
}

void fisch::toggleCameraBlur()
{
    input::clickKey(0x4d);
}

void fisch::lookDown()
{
    POINT robloxWindowPosition = Roblox::getInstance().getRobloxWindowPosition(),
        robloxWindowSize = Roblox::getInstance().getRobloxWindowSize();

    input::moveMouse(robloxWindowPosition.x + robloxWindowSize.x / 2, robloxWindowPosition.y + robloxWindowSize.y / 2);
    input::pressMouse(input::MouseButton::right);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    for (int i{}; i < 50; i++)
    {
        input::moveMouse(0, 100, false);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    input::releaseMouse(input::MouseButton::right);
}

void fisch::zoomIn()
{
    POINT robloxWindowPosition = Roblox::getInstance().getRobloxWindowPosition(),
        robloxWindowSize = Roblox::getInstance().getRobloxWindowSize();

    input::moveMouse(robloxWindowPosition.x + robloxWindowSize.x / 2, robloxWindowPosition.y + robloxWindowSize.y / 2);
    for (int i{}; i < 50; i++)
    {
        input::rotateMouseWheel(WHEEL_DELTA);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    input::rotateMouseWheel(-WHEEL_DELTA);
}

void fisch::sell()
{
    POINT robloxWindowPosition = Roblox::getInstance().getRobloxWindowPosition();

    if (Config::getInstance().getConfig().autoEnableCameraMode)
        toggleCameraMode();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    input::clickKey(VK_OEM_3);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    input::moveMouse(robloxWindowPosition.x + Config::getInstance().getPositions().sellButtonPosition.x, robloxWindowPosition.y + Config::getInstance().getPositions().sellButtonPosition.y);
    input::clickMouse(input::MouseButton::left);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    input::clickKey(VK_OEM_3);
    if (Config::getInstance().getConfig().autoEnableCameraMode)
        toggleCameraMode();
}

void fisch::castRod()
{
    POINT robloxWindowPosition = Roblox::getInstance().getRobloxWindowPosition(),
        robloxWindowSize = Roblox::getInstance().getRobloxWindowSize();

    input::moveMouse(robloxWindowPosition.x + robloxWindowSize.x / 2, robloxWindowPosition.y + robloxWindowSize.y / 2);
    input::pressMouse(input::MouseButton::left);
    std::this_thread::sleep_for(std::chrono::milliseconds(Config::getInstance().getConfig().castTime));
    input::releaseMouse(input::MouseButton::left);
}

cv::Rect fisch::findShakeButton(cv::Mat mat)
{
    cv::cvtColor(mat, mat, cv::COLOR_RGBA2GRAY);
    cv::dilate(mat, mat, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));
    cv::Canny(mat, mat, 200, 300);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mat, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (auto& contour : contours)
    {
        double area = cv::contourArea(contour);
        if (area < Config::getInstance().getConfig().minimumShakeButtonArea || area > Config::getInstance().getConfig().maximumShakeButtonArea || contour.size() < 100)
            continue;
        cv::approxPolyDP(contour, contour, 0.05 * cv::arcLength(contour, true), true);

        RECT robloxClientRect;
        GetClientRect(Roblox::getInstance().getRobloxHWnd(), &robloxClientRect);
        cv::Rect rect = cv::boundingRect(contour);
        rect.x += Config::getInstance().getPositions().searchShakeRegion.min.x - robloxClientRect.left;
        rect.y += Config::getInstance().getPositions().searchShakeRegion.min.y - robloxClientRect.top;
        return rect;
    }

    return cv::Rect();
}

int fisch::getBarWidth(cv::Mat mat)
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

std::pair<cv::Rect, cv::Rect> fisch::findLineAndArrow(cv::Mat mat)
{
    POINT robloxWindowSize = Roblox::getInstance().getRobloxWindowSize();
    int offsetX = Config::getInstance().getPositions().searchBarRegion.min.x;
    int offsetY = Config::getInstance().getPositions().searchBarRegion.min.y;

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

void fisch::clickShakeButton(const cv::Rect& rect)
{
    POINT robloxWindowPosition = Roblox::getInstance().getRobloxWindowPosition();

    if (!Config::getInstance().getConfig().checkClickShakePosition)
    {
        input::moveMouse(robloxWindowPosition.x + rect.x + rect.width / 2, robloxWindowPosition.y + rect.y + rect.height / 2);
        input::clickMouse(input::MouseButton::left);
        std::this_thread::sleep_for(std::chrono::milliseconds(Config::getInstance().getConfig().clickShakeDelay));
        return;
    }

    int posX = rect.x + rect.width / 2;
    int posY = rect.y + rect.height / 2;
    static int lastPosX{}, lastPosY{};
    if (abs(lastPosX - posX) <= 5 && abs(lastPosY - posY) <= 5)
        return;
    lastPosX = posX;
    lastPosY = posY;

    input::moveMouse(robloxWindowPosition.x + posX, robloxWindowPosition.y + posY);
    input::clickMouse(input::MouseButton::left);
}

void fisch::doBarMinigame(const cv::Rect& lineRect, const cv::Rect& arrowRect)
{
    static auto lastLoopTime = std::chrono::steady_clock::now();

    auto currentTime = std::chrono::steady_clock::now();
    auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastLoopTime).count();

    static double prevError{};

    double error = lineRect.x - arrowRect.x;
    double derivative = (error - prevError) / deltaTime;
    output = static_cast<int>(Config::getInstance().getConfig().kp * error + Config::getInstance().getConfig().kd * derivative);
    output = std::clamp(output, -Config::getInstance().getConfig().barWidth / 2, Config::getInstance().getConfig().barWidth / 2);

    if (Config::getInstance().getConfig().useBarDeadZoneLeft && lineRect.x < Config::getInstance().getPositions().barDeadZoneLeftPosition.x)
    {
        input::releaseMouse(input::MouseButton::left);
        return;
    }
    else if (Config::getInstance().getConfig().useBarDeadZoneRight && lineRect.x > Config::getInstance().getPositions().barDeadZoneRightPosition.x)
    {
        input::pressMouse(input::MouseButton::left);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        return;
    }

    if (output > 0)
        input::pressMouse(input::MouseButton::left);
    else
        input::releaseMouse(input::MouseButton::left);

    std::this_thread::sleep_for(std::chrono::milliseconds(abs(output)));

    prevError = error;
    lastLoopTime = currentTime;
}

bool fisch::failSafe(bool reset)
{
    if (reset)
    {
        failSafeCount = 0;
        return false;
    }

    static auto lastIncrementTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();

    if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastIncrementTime).count() >= 1)
    {
        failSafeCount++;
        lastIncrementTime = currentTime;
    }

    if (failSafeCount >= Config::getInstance().getConfig().failSafeThreshold)
    {
        failSafeCount = 0;
        return true;
    }
    return false;
}
