#pragma once

class Config
{
public:
    struct
    {
        int clickShakeDelay = 20;
        bool checkClickShakePosition = false;

        bool autoEnableCameraMode = true;
        bool autoBlur = true;
        bool autoLookDown = true;
        bool autoZoomIn = true;

        double kp = 0.1;
        double kd = 2.0;
    } config;

    struct
    {
        ImVec2 cameraModePos{};

        ImVec2 barDeadZoneLeftPos{};
        ImVec2 barDeadZoneRightPos{};

        ImRect searchShakeRect{};
        ImRect searchBarRect{};
    } coordinates;

    static Config& get();

private:
    std::filesystem::path configFolderPath = std::filesystem::current_path() / "configs/";
    std::filesystem::path coordinatesTXTPath = std::filesystem::current_path() / "coordinates.txt";
    std::filesystem::path macroTXTPath = std::filesystem::current_path() / "macro.txt";
    std::string configTXTname = "config.txt";

    Config();
    ~Config();
    bool validateFiles();
    bool saveConfig();
    bool loadConfig();
    bool saveCoordinates();
    bool loadCoordinates();
    bool saveMacro();
    bool loadMacro();
};

inline Config& config = Config::get();
