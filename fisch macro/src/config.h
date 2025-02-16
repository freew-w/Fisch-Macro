#pragma once

class Config
{
private:
    struct Data
    {
        std::string config = "config.txt";
    };

    struct Positions
    {
        Position cameraModePosition{};

        Region searchShakeRegion{};

        Region searchBarRegion{};
        Position barDeadZoneLeftPosition{};
        Position barDeadZoneRightPosition{};

        Position sellButtonPosition{};
    };

    struct Options
    {
        int failSafeThreshold = 2;

        bool autoEnableCameraMode = true;
        bool autoBlur = true;
        bool autoLookDown = true;
        bool autoZoomIn = true;
        int castTime = 200;

        bool autoShake = true;
        int minimumShakeButtonArea = 7000;
        int maximumShakeButtonArea = 12500;
        bool checkClickShakePosition = false;
        int clickShakeDelay = 30;

        bool autoBarMinigame = true;
        bool autoCalculateBarWidth = true;
        int barWidth = 0;
        bool useBarDeadZoneLeft = true;
        bool useBarDeadZoneRight = true;
        double kp = 0.5;
        double kd = 10.0;

        bool autoSell = false;
        bool showInfoUI = true;
        bool showRunTime = true;
        bool showFailSafeCount = true;
        bool showBarMinigameHoldDuration = true;
    };

public:
    inline static Config& getInstance() { static Config config{}; return config; }

    inline const Data& getData() const { return data_; }
    inline Positions& getPositions() { return positions_; }
    inline const Positions& getPositions() const { return positions_; }
    inline Options& getConfig() { return config_; }
    inline const Options& getConfig() const { return config_; }
    std::pair<int, const char**> getConfigs();
    int& getSelectedConfigIndex();

    bool userSave();
    bool userLoad();

private:
    const std::filesystem::path dataPath_ = std::filesystem::current_path() / "data.txt";
    const std::filesystem::path positionsPath_ = std::filesystem::current_path() / "positions.txt";
    const std::filesystem::path configFolderPath_ = std::filesystem::current_path() / "configs/";

    std::vector<std::string> configs_{};
    std::unique_ptr<const char* []> configsPtr_{};
    int selectedConfigIndex_ = -1;

    Data data_{};
    Positions positions_{};
    Options config_{};

    Config();
    ~Config();

    bool validateFiles() const;
    bool saveData() const;
    bool loadData();
    bool savePositions() const;
    bool loadPositions();
    bool saveConfig() const;
    bool loadConfig();
};
