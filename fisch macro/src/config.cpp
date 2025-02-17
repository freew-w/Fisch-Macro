#include "pch.h"
#include "config.h"

std::pair<const int, const char* const* const> Config::getConfigs()
{
    if (!configs_.empty())
        configs_.clear();

    for (const auto& file : std::filesystem::directory_iterator(configFolderPath_))
        if (file.is_regular_file())
            configs_.emplace_back(file.path().filename().string());

    configsPtr_ = std::make_unique<const char* []>(configs_.size());
    std::transform(configs_.begin(), configs_.end(), configsPtr_.get(), [](const std::string& str) { return str.c_str(); });

    return { configs_.size(), configsPtr_.get() };
}

int& Config::getSelectedConfigIndex()
{
    if (selectedConfigIndex_ == -1)
    {
        auto it = std::find(configs_.begin(), configs_.end(), data_.config);
        if (it != configs_.end())
            selectedConfigIndex_ = std::distance(configs_.begin(), it);
    }

    return selectedConfigIndex_;
}

bool Config::userSave()
{
    data_.config = configs_[selectedConfigIndex_];

    if (!saveData())
        return false;
    if (!savePositions())
        return false;
    if (!saveConfig())
        return false;

    return true;
}

bool Config::userLoad()
{
    data_.config = configs_[selectedConfigIndex_];

    if (!saveData())
        return false;
    if (!loadPositions())
        return false;
    if (!loadConfig())
        return false;

    return true;
}

Config::Config()
{
    if (!validateFiles())
        MessageBoxW(nullptr, L"Failed to validate files", L"Warning", MB_ICONWARNING | MB_TOPMOST);
    if (!loadData())
        MessageBoxW(nullptr, L"Failed to load data", L"Warning", MB_ICONWARNING | MB_TOPMOST);
    if (!loadPositions())
        MessageBoxW(nullptr, L"Failed to load positions", L"Warning", MB_ICONWARNING | MB_TOPMOST);
    if (!loadConfig())
        MessageBoxW(nullptr, L"Failed to load config", L"Warning", MB_ICONWARNING | MB_TOPMOST);
}

Config::~Config()
{
    if (!saveData())
        MessageBoxW(nullptr, L"Failed to save data", L"Warning", MB_ICONWARNING | MB_TOPMOST);
    if (!savePositions())
        MessageBoxW(nullptr, L"Failed to save positions", L"Warning", MB_ICONWARNING | MB_TOPMOST);
    if (!saveConfig())
        MessageBoxW(nullptr, L"Failed to save config", L"Warning", MB_ICONWARNING | MB_TOPMOST);
}

bool Config::validateFiles() const
{
    bool shouldCreateData = true;
    bool shouldCreatePositions = true;
    bool shouldCreateConfigsFolder = true;
    bool shouldCreateConfig = true;

    for (const auto& file : std::filesystem::directory_iterator(std::filesystem::current_path()))
    {
        if (file.path().filename() == "configs")
        {
            shouldCreateConfigsFolder = false;

            for (const auto& configFile : std::filesystem::directory_iterator(configFolderPath_))
            {
                if (configFile.is_regular_file())
                    shouldCreateConfig = false;
            }
        }
        else if (file.path().filename() == "positions.txt")
            shouldCreatePositions = false;
        else if (file.path().filename() == "data.txt")
            shouldCreateData = false;
    }

    if (shouldCreateData)
        if (!saveData())
            return false;
    if (shouldCreatePositions)
        if (!savePositions())
            return false;
    if (shouldCreateConfigsFolder)
        if (!std::filesystem::create_directory(configFolderPath_))
            return false;
    if (shouldCreateConfig)
        if (!saveConfig())
            return false;

    return true;
}

bool Config::saveData() const
{
    std::ofstream file(dataPath_, std::ios::out);

    if (!file.is_open())
        return false;

    file << "Config: " << data_.config << std::endl;

    return true;
}

bool Config::loadData()
{
    std::ifstream file(dataPath_);

    if (!file.is_open())
        return false;

    std::string line{};
    while (std::getline(file, line))
    {
        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos)
            continue;

        std::string option = line.substr(0, colonPos);
        std::string value = line.substr(colonPos + 2);

        if (option == "Config")					data_.config = value;
    }

    return true;
}

bool Config::savePositions() const
{
    std::ofstream file(positionsPath_, std::ios::out);

    if (!file.is_open())
        return false;

    file << "Camera Mode Position X: " << positions_.cameraModePosition.x << std::endl;
    file << "Camera Mode Position Y: " << positions_.cameraModePosition.y << std::endl;

    file << "Search Shake Region Min X: " << positions_.searchShakeRegion.min.x << std::endl;
    file << "Search Shake Region Min Y: " << positions_.searchShakeRegion.min.y << std::endl;
    file << "Search Shake Region Max X: " << positions_.searchShakeRegion.max.x << std::endl;
    file << "Search Shake Region Max Y: " << positions_.searchShakeRegion.max.y << std::endl;

    file << "Search Bar Region Min X: " << positions_.searchBarRegion.min.x << std::endl;
    file << "Search Bar Region Min Y: " << positions_.searchBarRegion.min.y << std::endl;
    file << "Search Bar Region Max X: " << positions_.searchBarRegion.max.x << std::endl;
    file << "Search Bar Region Max Y: " << positions_.searchBarRegion.max.y << std::endl;
    file << "Bar Dead Zone Left Position X: " << positions_.barDeadZoneLeftPosition.x << std::endl;
    file << "Bar Dead Zone Left Position Y: " << positions_.barDeadZoneLeftPosition.y << std::endl;
    file << "Bar Dead Zone Right Position X: " << positions_.barDeadZoneRightPosition.x << std::endl;
    file << "Bar Dead Zone Right Position Y: " << positions_.barDeadZoneRightPosition.y << std::endl;

    file << "Sell Button Position X: " << positions_.sellButtonPosition.x << std::endl;
    file << "Sell Button Position Y: " << positions_.sellButtonPosition.y << std::endl;

    return true;
}

bool Config::loadPositions()
{
    std::ifstream file(positionsPath_);

    if (!file.is_open())
        return false;

    std::string line{};
    while (std::getline(file, line))
    {
        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos)
            continue;

        std::string option = line.substr(0, colonPos);
        std::string value = line.substr(colonPos + 2);

        if (option == "Camera Mode Position X")			            positions_.cameraModePosition.x = std::stoi(value);
        else if (option == "Camera Mode Position Y")	            positions_.cameraModePosition.y = std::stoi(value);

        else if (option == "Search Shake Region Min X")	            positions_.searchShakeRegion.min.x = std::stoi(value);
        else if (option == "Search Shake Region Min Y")	            positions_.searchShakeRegion.min.y = std::stoi(value);
        else if (option == "Search Shake Region Max X")	            positions_.searchShakeRegion.max.x = std::stoi(value);
        else if (option == "Search Shake Region Max Y")             positions_.searchShakeRegion.max.y = std::stoi(value);

        else if (option == "Search Bar Region Min X")	            positions_.searchBarRegion.min.x = std::stoi(value);
        else if (option == "Search Bar Region Min Y")	            positions_.searchBarRegion.min.y = std::stoi(value);
        else if (option == "Search Bar Region Max X")	            positions_.searchBarRegion.max.x = std::stoi(value);
        else if (option == "Search Bar Region Max Y")               positions_.searchBarRegion.max.y = std::stoi(value);
        else if (option == "Bar Dead Zone Left Position X")	    	positions_.barDeadZoneLeftPosition.x = std::stoi(value);
        else if (option == "Bar Dead Zone Left Position Y")	    	positions_.barDeadZoneLeftPosition.y = std::stoi(value);
        else if (option == "Bar Dead Zone Right Position X")        positions_.barDeadZoneRightPosition.x = std::stoi(value);
        else if (option == "Bar Dead Zone Right Position Y")        positions_.barDeadZoneRightPosition.y = std::stoi(value);

        else if (option == "Sell Button Position X")            	positions_.sellButtonPosition.x = std::stoi(value);
        else if (option == "Sell Button Position Y")            	positions_.sellButtonPosition.y = std::stoi(value);
    }

    return true;
}

bool Config::saveConfig() const
{
    std::ofstream file(configFolderPath_ / data_.config, std::ios::out);

    if (!file.is_open())
        return false;

    file << "Fail Safe Threshold: " << config_.failSafeThreshold << std::endl;

    file << "Auto Enable Camera Mode: " << config_.autoEnableCameraMode << std::endl;
    file << "Auto Blur: " << config_.autoBlur << std::endl;
    file << "Auto Look Down: " << config_.autoLookDown << std::endl;
    file << "Auto Zoom In: " << config_.autoZoomIn << std::endl;

    file << "Cast Time: " << config_.castTime << std::endl;

    file << "Auto Shake: " << config_.autoShake << std::endl;
    file << "Minimum Shake Button Area: " << config_.minimumShakeButtonArea << std::endl;
    file << "Maximum Shake Button Area: " << config_.maximumShakeButtonArea << std::endl;
    file << "Check Click Shake Position: " << config_.checkClickShakePosition << std::endl;
    file << "Click Shake Delay: " << config_.clickShakeDelay << std::endl;

    file << "Auto Bar Minigame: " << config_.autoBarMinigame << std::endl;
    file << "Auto Calculate Bar Width: " << config_.autoCalculateBarWidth << std::endl;
    file << "Bar Width: " << config_.barWidth << std::endl;
    file << "Use Bar Dead Zone Left: " << config_.useBarDeadZoneLeft << std::endl;
    file << "Use Bar Dead Zone Right: " << config_.useBarDeadZoneRight << std::endl;
    file << "Kp: " << config_.kp << std::endl;
    file << "Kd: " << config_.kd << std::endl;

    file << "Auto Sell: " << config_.autoSell << std::endl;
    file << "Show Info UI: " << config_.showInfoUI << std::endl;
    file << "Show Run Time: " << config_.showRunTime << std::endl;
    file << "Show Fail Safe Count: " << config_.showFailSafeCount << std::endl;
    file << "Show Bar Minigame Hold Duration: " << config_.showBarMinigameHoldDuration << std::endl;

    file.close();

    return true;
}

bool Config::loadConfig()
{
    std::ifstream file(configFolderPath_ / data_.config);

    if (!file.is_open())
        return false;

    std::string line{};
    while (std::getline(file, line))
    {
        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos)
            continue;

        std::string option = line.substr(0, colonPos);
        std::string value = line.substr(colonPos + 2);

        if (option == "Fail Safe Threshold")                        config_.failSafeThreshold = std::stoi(value);

        else if (option == "Auto Enable Camera Mode")               config_.autoEnableCameraMode = std::stoi(value);
        else if (option == "Auto Blur")                             config_.autoBlur = std::stoi(value);
        else if (option == "Auto Look Down")                        config_.autoLookDown = std::stoi(value);
        else if (option == "Auto Zoom In")                          config_.autoZoomIn = std::stoi(value);

        else if (option == "Cast Time")                             config_.castTime = std::stoi(value);

        else if (option == "Auto Shake")                            config_.autoShake = std::stoi(value);
        else if (option == "Minimum Shake Button Area")             config_.minimumShakeButtonArea = std::stoi(value);
        else if (option == "Maximum Shake Button Area")             config_.maximumShakeButtonArea = std::stoi(value);
        else if (option == "Check Click Shake Position")            config_.checkClickShakePosition = std::stoi(value);
        else if (option == "Click Shake Delay")                     config_.clickShakeDelay = std::stoi(value);

        else if (option == "Auto Bar Minigame")                     config_.autoBarMinigame = std::stoi(value);
        else if (option == "Auto Calculate Bar Width")              config_.autoCalculateBarWidth = std::stoi(value);
        else if (option == "Bar Width")                             config_.barWidth = std::stoi(value);
        else if (option == "Use Bar Dead Zone Left")                config_.useBarDeadZoneLeft = std::stoi(value);
        else if (option == "Use Bar Dead Zone Right")               config_.useBarDeadZoneRight = std::stoi(value);
        else if (option == "Kp")                                    config_.kp = std::stod(value);
        else if (option == "Kd")                                    config_.kd = std::stod(value);

        else if (option == "Auto Sell")                             config_.autoSell = std::stoi(value);
        else if (option == "Show Info UI")                          config_.showInfoUI = std::stoi(value);
        else if (option == "Show Run Time")                         config_.showRunTime = std::stoi(value);
        else if (option == "Show Fail Safe Count")                  config_.showFailSafeCount = std::stoi(value);
        else if (option == "Show Bar Minigame Hold Duration")       config_.showBarMinigameHoldDuration = std::stoi(value);
    }

    file.close();

    return true;
}
