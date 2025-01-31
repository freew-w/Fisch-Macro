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

bool Fisch::setPos(ImVec2& pos)
{
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    ImGui::Begin("set pos", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

    static int prevWindowPos{};
    static bool positioned{};
    if (prevWindowPos != static_cast<int>(pos.x + pos.y))
    {
        prevWindowPos = static_cast<int>(pos.x + pos.y);
        positioned = false;
    }
    if (!positioned)
    {
        positioned = true;
        ImGui::SetWindowPos({ pos.x - ImGui::GetWindowSize().x / 2.0f, pos.y - ImGui::GetWindowSize().y / 2.0f });
    }

    if (ImGui::Button("Ok"))
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(ImGui::GetWindowPos(), { ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y }, IM_COL32(255, 0, 0, 255));
        pos.x = ImGui::GetWindowPos().x + ImGui::GetWindowSize().x / 2.0f;
        pos.y = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y / 2.0f;
        return true;
    }

    ImGui::End();
    ImGui::PopStyleVar();

    return false;
}

bool Fisch::setArea(ImRect& rect)
{
    static int prevPos{};
    static bool positioned{};
    if (prevPos != static_cast<int>(rect.Min.x + rect.Min.y + rect.Max.x + rect.Max.y))
    {
        prevPos = static_cast<int>(rect.Min.x + rect.Min.y + rect.Max.x + rect.Max.y);
        positioned = false;
    }
    if (!positioned)
    {
        positioned = true;
        ImGui::SetNextWindowPos({ rect.Min.x,  rect.Min.y });
        ImGui::SetNextWindowSize({ rect.Max.x - rect.Min.x, rect.Max.y - rect.Min.y });
    }

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    ImGui::Begin("set area", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

    if (ImGui::Button("Ok"))
    {
        rect.Min.x = ImGui::GetWindowPos().x;
        rect.Min.y = ImGui::GetWindowPos().y;
        rect.Max.x = ImGui::GetWindowPos().x + ImGui::GetWindowSize().x;
        rect.Max.y = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y;
        return true;
    }

    ImGui::End();
    ImGui::PopStyleVar();

    return false;
}

cv::Mat Fisch::screenshot(const ImRect& rect)
{
    if (rect.Min.x == 0 && rect.Min.y == 0 && rect.Max.x == 0 && rect.Max.y == 0)
        return cv::Mat();

    POINT robloxClientToScreenPoint{};
    ClientToScreen(robloxHWnd, &robloxClientToScreenPoint);
    int w = static_cast<int>(rect.Max.x - rect.Min.x), h = static_cast<int>(rect.Max.y - rect.Min.y);

    HDC hDC = GetDC(nullptr);
    HDC hMemDC = CreateCompatibleDC(hDC);
    HBITMAP hBitmap = CreateCompatibleBitmap(hDC, w, h);
    HGDIOBJ hOldBitmap = SelectObject(hMemDC, hBitmap);

    StretchBlt(hMemDC, 0, 0, w, h, hDC, static_cast<int>(robloxClientToScreenPoint.x + rect.Min.x), static_cast<int>(robloxClientToScreenPoint.y + rect.Min.y), w, h, SRCCOPY);

    cv::Mat mat(h, w, CV_8UC4);
    BITMAPINFOHEADER bih = { sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB };
    GetDIBits(hDC, hBitmap, 0, h, mat.data, reinterpret_cast<BITMAPINFO*>(&bih), DIB_RGB_COLORS);

    SelectObject(hMemDC, hOldBitmap);
    DeleteDC(hMemDC);
    DeleteObject(hBitmap);
    ReleaseDC(nullptr, hDC);

    return mat;
}

void Fisch::enableCameraMode(const ImVec2& pos)
{
    POINT robloxClientToScreenPoint{};
    ClientToScreen(robloxHWnd, &robloxClientToScreenPoint);

    SetCursorPos(static_cast<int>(robloxClientToScreenPoint.x + pos.x), static_cast<int>(robloxClientToScreenPoint.y + pos.y));
    SendInput(1, &mouseMove, sizeof(INPUT));
    SendInput(2, leftMouseClick, sizeof(INPUT));
}

void Fisch::blurCamera()
{
    SendInput(2, mClick, sizeof(INPUT));
}

void Fisch::lookDown()
{
    RECT robloxClientRect{};
    POINT robloxClientToScreenPoint{};
    GetClientRect(robloxHWnd, &robloxClientRect);
    ClientToScreen(robloxHWnd, &robloxClientToScreenPoint);

    SetCursorPos(robloxClientToScreenPoint.x + robloxClientRect.right / 2, robloxClientToScreenPoint.y + robloxClientRect.bottom / 2);
    SendInput(1, &mouseMove, sizeof(INPUT));
    SendInput(1, &rightMouseClick[0], sizeof(INPUT));
    MOUSEINPUT mi{};
    INPUT input{};
    mi.dwFlags = MOUSEEVENTF_MOVE;
    input.type = INPUT_MOUSE;
    for (int i{}; i < 50; i++) {
        mi.dy = 700 / 50;
        input.mi = mi;
        SendInput(1, &input, sizeof(INPUT));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    SendInput(1, &rightMouseClick[1], sizeof(INPUT));
}

void Fisch::zoomIn()
{
    RECT robloxClientRect{};
    POINT robloxClientToScreenPoint{};
    GetClientRect(robloxHWnd, &robloxClientRect);
    ClientToScreen(robloxHWnd, &robloxClientToScreenPoint);

    SetCursorPos(robloxClientToScreenPoint.x + robloxClientRect.right / 2, robloxClientToScreenPoint.y + robloxClientRect.bottom / 2);
    for (int i{}; i < 40; i++)
    {
        SendInput(1, &mouseScroll[0], sizeof(INPUT));
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    SendInput(1, &mouseScroll[1], sizeof(INPUT));
}

void Fisch::castRod()
{
    RECT robloxClientRect{};
    POINT robloxClientToScreenPoint{};
    GetClientRect(robloxHWnd, &robloxClientRect);
    ClientToScreen(robloxHWnd, &robloxClientToScreenPoint);

    SetCursorPos(robloxClientToScreenPoint.x + robloxClientRect.right / 2, robloxClientToScreenPoint.y + robloxClientRect.bottom / 2);
    SendInput(1, &mouseMove, sizeof(INPUT));
    SendInput(1, &leftMouseClick[0], sizeof(INPUT));
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    SendInput(1, &leftMouseClick[1], sizeof(INPUT));
}

cv::Rect Fisch::findShakeButton(cv::Mat mat)
{
    cv::cvtColor(mat, mat, cv::COLOR_RGBA2GRAY);
    cv::dilate(mat, mat, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));
    cv::Canny(mat, mat, 200, 300);
    std::vector<std::vector<cv::Point>> contours{};
    cv::findContours(mat, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (auto& contour : contours)
    {
        double area = cv::contourArea(contour);
        if (area < 7000 || area > 12500 || contour.size() < 100)
            continue;
        cv::approxPolyDP(contour, contour, 0.05 * cv::arcLength(contour, true), true);

        RECT robloxClientRect{};
        GetClientRect(robloxHWnd, &robloxClientRect);
        cv::Rect rect = cv::boundingRect(contour);
        rect.x += static_cast<int>(config.coordinates.searchShakeRect.Min.x - robloxClientRect.left);
        rect.y += static_cast<int>(config.coordinates.searchShakeRect.Min.y - robloxClientRect.top);
        return rect;
    }

    return cv::Rect();
}

void Fisch::clickShakeButton(const cv::Rect& rect)
{
    POINT robloxClientToScreenPoint{};
    ClientToScreen(robloxHWnd, &robloxClientToScreenPoint);

    int posX = rect.x + rect.width / 2, posY = rect.y + rect.height / 2;

    if (!config.config.checkClickShakePosition)
        goto click;

    static int lastPosX{}, lastPosY{};
    if (abs((posX + posY) - (lastPosX + lastPosY)) <= 2)
    {
        lastPosX = posX;
        lastPosY = posY;
        return;
    }
    lastPosX = posX;
    lastPosY = posY;

click:
    SetCursorPos(robloxClientToScreenPoint.x + posX, robloxClientToScreenPoint.y + posY);
    SendInput(1, &mouseMove, sizeof(INPUT));
    SendInput(2, leftMouseClick, sizeof(INPUT));
}

// POSITION IS RELATIVE TO THE IMAGE
//cv::Rect Fisch::findBar(cv::Mat mat, bool isArrowOnLeft)
//{
//	cv::Rect rect = findArrow(mat);
//
//	if (!rect.x)
//		return cv::Rect();
//
//	static auto [offset, widthHeight] = getArrowToBarDistanceAndBarWidthHeight(mat);
//
//	//if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
//	//	return { rect.x + rect.width + offset.x - widthHeight.x, rect.y - offset.y, widthHeight.x, widthHeight.y };
//	//else
//	//	return { rect.x - offset.x, rect.y - offset.y, widthHeight.x, widthHeight.y };
//
//	//static int prevX = rect.x;
//	//static bool isArrowOnLeft = true;
//	//if (rect.x - prevX > 60)
//	//{
//	//	isArrowOnLeft = false;
//	//	prevX = rect.x;
//	//}
//	//else if (prevX - rect.x > 60)
//	//{
//	//	isArrowOnLeft = true;
//	//	prevX = rect.x;
//	//}
//
//	//static auto lastResetTime = std::chrono::high_resolution_clock::now();
//	//auto currentTime = std::chrono::high_resolution_clock::now();
//	//if (std::chrono::duration_cast<std::chrono::milliseconds>(lastResetTime - currentTime).count() >= 10) {
//	//	prevX = rect.x;
//	//	lastResetTime = currentTime;
//	//}
//
//	if (isArrowOnLeft)
//		return { rect.x - offset.x, rect.y - offset.y, widthHeight.x, widthHeight.y };
//	else
//		return { rect.x + rect.width + offset.x - widthHeight.x, rect.y - offset.y, widthHeight.x, widthHeight.y };
//}

cv::Rect Fisch::findLine(cv::Mat mat)
{
    cv::cvtColor(mat, mat, cv::COLOR_BGR2HSV);

    cv::inRange(mat, cv::Scalar(110, 50, 80), cv::Scalar(110, 70, 100), mat);
    cv::medianBlur(mat, mat, 3);
    std::vector<std::vector<cv::Point>> contours{};
    cv::findContours(mat, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (auto& contour : contours)
    {
        cv::approxPolyDP(contour, contour, 0.02 * cv::arcLength(contour, true), true);
        RECT robloxClientRect{};
        GetClientRect(robloxHWnd, &robloxClientRect);
        cv::Rect rect = cv::boundingRect(contour);
        rect.x += static_cast<int>(config.coordinates.searchBarRect.Min.x - robloxClientRect.left);
        rect.y += static_cast<int>(config.coordinates.searchBarRect.Min.y - robloxClientRect.top);
        return rect;
    }

    return cv::Rect();
}

cv::Rect Fisch::findArrow(cv::Mat mat)
{
    //cv::cvtColor(mat, mat, cv::COLOR_RGBA2RGB);
    //cv::inRange(mat, cv::Scalar(116, 114, 116), cv::Scalar(143, 144, 146), mat);
    cv::cvtColor(mat, mat, cv::COLOR_BGR2HSV);
    cv::inRange(mat, cv::Scalar(0, 0, 120), cv::Scalar(180, 20, 140), mat);
    cv::medianBlur(mat, mat, 3);
    std::vector<std::vector<cv::Point>> contours{};
    cv::findContours(mat, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (auto& contour : contours)
    {
        cv::approxPolyDP(contour, contour, 0.02 * cv::arcLength(contour, true), true);
        if (contour.size() < 5 || cv::contourArea(contour) < 20)
            continue;
        RECT robloxClientRect{};
        GetClientRect(robloxHWnd, &robloxClientRect);
        cv::Rect rect = cv::boundingRect(contour);
        rect.x += static_cast<int>(config.coordinates.searchBarRect.Min.x - robloxClientRect.left);
        rect.y += static_cast<int>(config.coordinates.searchBarRect.Min.y - robloxClientRect.top);
        return rect;
    }

    return cv::Rect();
}

std::tuple<cv::Point, cv::Point> Fisch::getArrowToBarDistanceAndBarWidthHeight(cv::Mat mat)
{
    cv::Mat findArrowMat = mat;

    cv::cvtColor(mat, mat, cv::COLOR_RGBA2GRAY);
    cv::morphologyEx(mat, mat, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));
    cv::GaussianBlur(mat, mat, cv::Size(3, 3), 0, 0);
    cv::threshold(mat, mat, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    cv::Canny(mat, mat, 50, 150);
    std::vector<std::vector<cv::Point>> contours{};
    cv::findContours(mat, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<cv::Rect> barRects{};
    for (auto& contour : contours)
    {
        double area = cv::contourArea(contour);
        if (area < 50 || area > 3000)
            continue;
        cv::approxPolyDP(contour, contour, 0.05 * cv::arcLength(contour, true), true);
        if (contour.size() > 4)
            continue;
        barRects.emplace_back(cv::boundingRect(contour));
    }
    if (barRects.empty())
        return std::tuple<cv::Point, cv::Point>();

    int xMin = barRects[0].x, yMin = barRects[0].y, xMax{}, yMax{};
    for (const auto& rect : barRects) {
        xMin = std::min(xMin, rect.x);
        yMin = std::min(yMin, rect.y);
        xMax = std::max(xMax, rect.x + rect.width);
        yMax = std::max(yMax, rect.y + rect.height);
    }

    cv::Rect barRect = cv::Rect(xMin, yMin, xMax - xMin, yMax - yMin);
    if (barRect.area() < 1000 || barRect.area() > 3000)
        return std::tuple<cv::Point, cv::Point>();

    cv::Rect arrowRect = findArrow(findArrowMat);
    return {
        {arrowRect.x - barRect.x, arrowRect.y - barRect.y},
        {barRect.width , barRect.height}
    };
}

Fisch::Fisch()
{
    robloxHWnd = FindWindowW(nullptr, L"Roblox");
    if (!robloxHWnd)
        error(L"Roblox not found");
}
