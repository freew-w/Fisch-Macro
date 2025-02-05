#pragma once

class Config
{
public:
    struct
    {
        int failSafeCount = 2;

        int castTime = 200;

        bool autoEnableCameraMode = true;
        bool autoBlur = true;
        bool autoLookDown = true;
        bool autoZoomIn = true;

        int minimumShakeButtonArea = 7000;
        int maximumShakeButtonArea = 12500;
        bool checkClickShakePosition = false;
        int clickShakeDelay = 30;

        bool autoCalculateBarWidth = true;
        int barWidth = 0;
        bool useBarDeadZoneLeft = true;
        bool useBarDeadZoneRight = true;
        double kp = 0.8;
        double kd = 10.0;
    } config;

    struct
    {
        ImVec2 cameraModePosition{};

        ImVec2 barDeadZoneLeftPosition{};
        ImVec2 barDeadZoneRightPosition{};

        ImRect searchShakeRect{};
        ImRect searchBarRect{};
    } coordinates;

    struct
    {
        std::string configTXTname = "config.txt";
        std::vector<std::string> configsString{};
        std::unique_ptr<const char* []> configsCString;
    } data;

    static Config& get();

    bool loadData();
    bool loadConfig();
    bool saveConfig();

private:
    std::filesystem::path configFolderPath = std::filesystem::current_path() / "configs/";
    std::filesystem::path coordinatesTXTPath = std::filesystem::current_path() / "coordinates.txt";
    std::filesystem::path dataTXTPath = std::filesystem::current_path() / "data.txt";

    Config();
    ~Config();
    bool validateFiles();
    bool saveData();
    bool loadCoordinates();
    bool saveCoordinates();
};

inline Config& config = Config::get();
