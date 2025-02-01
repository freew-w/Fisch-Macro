#include "pch.h"
#include "fisch.h"
#include "config.h"

Config& Config::get()
{
    static Config config{};
    return config;
}

Config::Config()
{
    if (!validateFiles())
        fisch.warn(L"Failed to validate config");
    if (!loadConfig())
        fisch.warn(L"Failed to load config");
    if (!loadCoordinates())
        fisch.warn(L"Failed to load coordinates.txt");
    if (!loadMacro())
        fisch.warn(L"Failed to load macro.txt");
}

Config::~Config()
{
    if (!validateFiles())
        fisch.warn(L"Failed to validate config");
    if (!saveConfig())
        fisch.warn(L"Failed to save config");
    if (!saveCoordinates())
        fisch.warn(L"Failed to save coordinates.txt");
    if (!saveMacro())
        fisch.warn(L"Failed to save macro.txt");
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
        if (!saveMacro())
            return false;

    return true;
}

bool Config::saveConfig()
{
    std::ofstream file(configFolderPath / configTXTname, std::ios::out);

    if (!file.is_open())
        return false;

    file << "Click Shake Delay: " << config.clickShakeDelay << std::endl;
    file << "Check Click Shake Position: " << config.checkClickShakePosition << std::endl;

    file << "Auto Enable Camera Mode: " << config.autoEnableCameraMode << std::endl;
    file << "Auto Blur: " << config.autoBlur << std::endl;
    file << "Auto Look Down: " << config.autoLookDown << std::endl;
    file << "Auto Zoom In: " << config.autoZoomIn << std::endl;

    file << "Kp: " << config.kp << std::endl;
    file << "Kd: " << config.kd << std::endl;

    file.close();

    return true;
}

bool Config::loadConfig()
{
    std::ifstream file(configFolderPath / configTXTname);

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

        if (option == "Click Shake Delay")				config.clickShakeDelay = std::stoi(value);
        else if (option == "Check Click Shake Position")config.checkClickShakePosition = std::stoi(value);

        else if (option == "Auto Enable Camera Mode")	config.autoEnableCameraMode = std::stoi(value);
        else if (option == "Auto Blur")					config.autoBlur = std::stoi(value);
        else if (option == "Auto Look Down")			config.autoLookDown = std::stoi(value);
        else if (option == "Auto Zoom In")				config.autoZoomIn = std::stoi(value);

        else if (option == "Kp")			            config.kp = std::stod(value);
        else if (option == "Kd")				        config.kd = std::stod(value);
    }

    file.close();

    return true;
}

bool Config::saveCoordinates()
{
    std::ofstream file(coordinatesTXTPath, std::ios::out);

    if (!file.is_open())
        return false;

    file << "Camera Mode Pos X: " << coordinates.cameraModePos.x << std::endl;
    file << "Camera Mode Pos Y: " << coordinates.cameraModePos.y << std::endl;

    file << "Bar Dead Zone Left X: " << coordinates.barDeadZoneLeftPos.x << std::endl;
    file << "Bar Dead Zone Left Y: " << coordinates.barDeadZoneLeftPos.y << std::endl;
    file << "Bar Dead Zone Right X: " << coordinates.barDeadZoneRightPos.x << std::endl;
    file << "Bar Dead Zone Right Y: " << coordinates.barDeadZoneRightPos.y << std::endl;

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

        if (option == "Camera Mode Pos X")				coordinates.cameraModePos.x = std::stoi(value);
        else if (option == "Camera Mode Pos Y")			coordinates.cameraModePos.y = std::stoi(value);

        else if (option == "Bar Dead Zone Left X")		coordinates.barDeadZoneLeftPos.x = std::stoi(value);
        else if (option == "Bar Dead Zone Left Y")		coordinates.barDeadZoneLeftPos.y = std::stoi(value);
        else if (option == "Bar Dead Zone Right X")		coordinates.barDeadZoneRightPos.x = std::stoi(value);
        else if (option == "Bar Dead Zone Right Y")		coordinates.barDeadZoneRightPos.y = std::stoi(value);

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

bool Config::saveMacro()
{
    std::ofstream file(macroTXTPath, std::ios::out);

    if (!file.is_open())
        return false;

    file << "Config To Load: " << configTXTname << std::endl;

    return true;
}

bool Config::loadMacro()
{
    std::ifstream file(macroTXTPath);

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

        if (option == "Config To Load")					configTXTname = value;
    }

    return true;
}
