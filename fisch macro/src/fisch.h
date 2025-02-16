#pragma once

namespace fisch
{
    inline bool isRunning = true;
    inline bool enabled{};

    inline int runTimeHours{}, runTimeMinutes{}, runTimeSeconds{};
    inline int failSafeCount{};
    inline int output{};

    cv::Mat screenshot(const Region& region);

    void toggleCameraMode();
    void toggleCameraBlur();
    void lookDown();
    void zoomIn();
    void sell();
    void castRod();

    cv::Rect findShakeButton(cv::Mat mat);
    int getBarWidth(cv::Mat mat);
    std::pair<cv::Rect, cv::Rect> findLineAndArrow(cv::Mat mat);

    void clickShakeButton(const cv::Rect& rect);
    void doBarMinigame(const cv::Rect& lineRect, const cv::Rect& arrowRect);
    bool failSafe(bool reset = false);
}
