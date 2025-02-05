#include "pch.h"
#include "fisch.h"
#include "config.h"

Config& Config::get()
{
    static Config config{};
    return config;
}

bool Config::loadConfig()
{
    std::ifstream file(configFolderPath / data.configTXTname);

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

        if (option == "Fail Safe Count")		            config.failSafeCount = std::stoi(value);

        else if (option == "Auto Enable Camera Mode")	    config.autoEnableCameraMode = std::stoi(value);
        else if (option == "Auto Blur")					    config.autoBlur = std::stoi(value);
        else if (option == "Auto Look Down")			    config.autoLookDown = std::stoi(value);
        else if (option == "Auto Zoom In")				    config.autoZoomIn = std::stoi(value);

        else if (option == "Cast Time")				        config.castTime = std::stoi(value);

        else if (option == "Minimum Shake Button Area")		config.minimumShakeButtonArea = std::stoi(value);
        else if (option == "Maximum Shake Button Area")	    config.maximumShakeButtonArea = std::stoi(value);
        else if (option == "Check Click Shake Position")    config.checkClickShakePosition = std::stoi(value);
        else if (option == "Click Shake Delay")			    config.clickShakeDelay = std::stoi(value);

        else if (option == "Auto Calculate Bar Width")	    config.autoCalculateBarWidth = std::stoi(value);
        else if (option == "Bar Width")			            config.barWidth = std::stoi(value);
        else if (option == "Use Bar Dead Zone Left")	    config.useBarDeadZoneLeft = std::stoi(value);
        else if (option == "Use Bar Dead Zone Right")		config.useBarDeadZoneRight = std::stoi(value);
        else if (option == "Kp")			                config.kp = std::stod(value);
        else if (option == "Kd")				            config.kd = std::stod(value);
    }

    file.close();

    return true;
}

bool Config::saveConfig()
{
    std::ofstream file(configFolderPath / data.configTXTname, std::ios::out);

    if (!file.is_open())
        return false;

    file << "Fail Safe Count: " << config.failSafeCount << std::endl;

    file << "Auto Enable Camera Mode: " << config.autoEnableCameraMode << std::endl;
    file << "Auto Blur: " << config.autoBlur << std::endl;
    file << "Auto Look Down: " << config.autoLookDown << std::endl;
    file << "Auto Zoom In: " << config.autoZoomIn << std::endl;

    file << "Cast Time: " << config.castTime << std::endl;

    file << "Minimum Shake Button Area: " << config.minimumShakeButtonArea << std::endl;
    file << "Maximum Shake Button Area: " << config.maximumShakeButtonArea << std::endl;
    file << "Check Click Shake Position: " << config.checkClickShakePosition << std::endl;
    file << "Click Shake Delay: " << config.clickShakeDelay << std::endl;

    file << "Auto Calculate Bar Width: " << config.autoCalculateBarWidth << std::endl;
    file << "Bar Width: " << config.barWidth << std::endl;
    file << "Use Bar Dead Zone Left: " << config.useBarDeadZoneLeft << std::endl;
    file << "Use Bar Dead Zone Right: " << config.useBarDeadZoneRight << std::endl;
    file << "Kp: " << config.kp << std::endl;
    file << "Kd: " << config.kd << std::endl;

    file.close();

    return true;
}

Config::Config()
{
    if (!validateFiles())
        fisch.warn(L"Failed to validate config");
    if (!loadData())
        fisch.warn(L"Failed to load macro.txt");
    if (!loadCoordinates())
        fisch.warn(L"Failed to load coordinates.txt");
    if (!loadConfig())
        fisch.warn(L"Failed to load config");
}

Config::~Config()
{
    if (!validateFiles())
        fisch.warn(L"Failed to validate config");
    if (!saveData())
        fisch.warn(L"Failed to save macro.txt");
    if (!saveCoordinates())
        fisch.warn(L"Failed to save coordinates.txt");
    if (!saveConfig())
        fisch.warn(L"Failed to save config");
}

bool Config::validateFiles()
{
    bool shouldCreateConfigFolder = true;
    bool shouldCreateConfigTXT = true;
    bool shouldCreateCoordinatesTXT = true;
    bool shouldCreateMacroTXT = true;

    for (const auto& file : std::filesystem::directory_iterator(std::filesystem::current_path()))
    {
        if (file.path().filename() == "configs")
        {
            shouldCreateConfigFolder = false;

            for (const auto& configFile : std::filesystem::directory_iterator(configFolderPath))
            {
                if (configFile.is_regular_file())
                    shouldCreateConfigTXT = false;
            }
        }
        else if (file.path().filename() == "coordinates.txt")
            shouldCreateCoordinatesTXT = false;
        else if (file.path().filename() == "macro.txt")
            shouldCreateMacroTXT = false;
    }

    if (shouldCreateConfigFolder)
        if (!std::filesystem::create_directory(configFolderPath))
            return false;
    if (shouldCreateConfigTXT)
        if (!saveConfig())
            return false;
    if (shouldCreateCoordinatesTXT)
        if (!saveCoordinates())
            return false;
    if (shouldCreateMacroTXT)
        if (!saveData())
            return false;

    return true;
}

bool Config::loadData()
{
    std::ifstream file(dataTXTPath);

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

        if (option == "Config To Load")					data.configTXTname = value;
    }

    if (!data.configsString.empty())
        data.configsString.clear();
    for (const auto& file : std::filesystem::directory_iterator(configFolderPath))
        data.configsString.emplace_back(file.path().filename().string());

    data.configsCString = std::make_unique<const char* []>(data.configsString.size());
    std::transform(data.configsString.begin(), data.configsString.end(), data.configsCString.get(), [](const std::string& str) { return str.c_str(); });

    return true;
}

bool Config::saveData()
{
    std::ofstream file(dataTXTPath, std::ios::out);

    if (!file.is_open())
        return false;

    file << "Config To Load: " << data.configTXTname << std::endl;

    return true;
}

bool Config::loadCoordinates()
{
    std::ifstream file(coordinatesTXTPath);

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

        if (option == "Camera Mode Pos X")				coordinates.cameraModePosition.x = std::stoi(value);
        else if (option == "Camera Mode Pos Y")			coordinates.cameraModePosition.y = std::stoi(value);

        else if (option == "Bar Dead Zone Left X")		coordinates.barDeadZoneLeftPosition.x = std::stoi(value);
        else if (option == "Bar Dead Zone Left Y")		coordinates.barDeadZoneLeftPosition.y = std::stoi(value);
        else if (option == "Bar Dead Zone Right X")		coordinates.barDeadZoneRightPosition.x = std::stoi(value);
        else if (option == "Bar Dead Zone Right Y")		coordinates.barDeadZoneRightPosition.y = std::stoi(value);

        else if (option == "Search Shake Rect Min X")	coordinates.searchShakeRect.Min.x = std::stoi(value);
        else if (option == "Search Shake Rect Min Y")	coordinates.searchShakeRect.Min.y = std::stoi(value);
        else if (option == "Search Shake Rect Max X")	coordinates.searchShakeRect.Max.x = std::stoi(value);
        else if (option == "Search Shake Rect Max Y")	coordinates.searchShakeRect.Max.y = std::stoi(value);

        else if (option == "Search Bar Rect Min X")		coordinates.searchBarRect.Min.x = std::stoi(value);
        else if (option == "Search Bar Rect Min Y")		coordinates.searchBarRect.Min.y = std::stoi(value);
        else if (option == "Search Bar Rect Max X")	    coordinates.searchBarRect.Max.x = std::stoi(value);
        else if (option == "Search Bar Rect Max Y")     coordinates.searchBarRect.Max.y = std::stoi(value);
    }

    return true;
}

bool Config::saveCoordinates()
{
    std::ofstream file(coordinatesTXTPath, std::ios::out);

    if (!file.is_open())
        return false;

    file << "Camera Mode Pos X: " << coordinates.cameraModePosition.x << std::endl;
    file << "Camera Mode Pos Y: " << coordinates.cameraModePosition.y << std::endl;

    file << "Bar Dead Zone Left X: " << coordinates.barDeadZoneLeftPosition.x << std::endl;
    file << "Bar Dead Zone Left Y: " << coordinates.barDeadZoneLeftPosition.y << std::endl;
    file << "Bar Dead Zone Right X: " << coordinates.barDeadZoneRightPosition.x << std::endl;
    file << "Bar Dead Zone Right Y: " << coordinates.barDeadZoneRightPosition.y << std::endl;

    file << "Search Shake Rect Min X: " << coordinates.searchShakeRect.Min.x << std::endl;
    file << "Search Shake Rect Min Y: " << coordinates.searchShakeRect.Min.y << std::endl;
    file << "Search Shake Rect Max X: " << coordinates.searchShakeRect.Max.x << std::endl;
    file << "Search Shake Rect Max Y: " << coordinates.searchShakeRect.Max.y << std::endl;

    file << "Search Bar Rect Min X: " << coordinates.searchBarRect.Min.x << std::endl;
    file << "Search Bar Rect Min Y: " << coordinates.searchBarRect.Min.y << std::endl;
    file << "Search Bar Rect Max X: " << coordinates.searchBarRect.Max.x << std::endl;
    file << "Search Bar Rect Max Y: " << coordinates.searchBarRect.Max.y << std::endl;

    return true;
}
