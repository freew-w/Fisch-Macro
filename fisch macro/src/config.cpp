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
	if (!load())
		fisch.warn(L"Failed to load config");
}

Config::~Config()
{
	if (!save())
		fisch.warn(L"Failed to save config");
}

bool Config::load()
{
	std::ifstream file(std::filesystem::current_path().concat("/configs/config.txt").c_str());

	if (!file.is_open())
		if (!save())
			return false;

	std::string line{};
	while (std::getline(file, line))
	{
		if (line.find("//") == 0)
		{
			comments.emplace_back(line);
			std::cout << line << std::endl;
			continue;
		}

		size_t colonPos = line.find(':');
		if (colonPos == std::string::npos)
			continue;

		std::string option = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 2);

		if (option == "Click Shake Delay")				config.clickShakeDelay = std::stoi(value);

		if (option == "Auto Enable Camera Mode")		config.autoEnableCameraMode = value == "1" ? true : false;
		if (option == "Auto Blur")						config.autoBlur = value == "1" ? true : false;
		if (option == "Auto Look Down")					config.autoLookDown = value == "1" ? true : false;
		if (option == "Auto Zoom In")					config.autoZoomIn = value == "1" ? true : false;

		else if (option == "Camera Mode Pos X")			config.cameraModePos.x = std::stoi(value);
		else if (option == "Camera Mode Pos Y")			config.cameraModePos.y = std::stoi(value);

		else if (option == "Bar Dead Zone Left X")		config.barDeadZoneLeftPos.x = std::stoi(value);
		else if (option == "Bar Dead Zone Left Y")		config.barDeadZoneLeftPos.y = std::stoi(value);
		else if (option == "Bar Dead Zone Right X")		config.barDeadZoneRightPos.x = std::stoi(value);
		else if (option == "Bar Dead Zone Right Y")		config.barDeadZoneRightPos.y = std::stoi(value);

		else if (option == "Search Shake Rect Min X")	config.searchShakeRect.Min.x = std::stoi(value);
		else if (option == "Search Shake Rect Min Y")	config.searchShakeRect.Min.y = std::stoi(value);
		else if (option == "Search Shake Rect Max X")	config.searchShakeRect.Max.x = std::stoi(value);
		else if (option == "Search Shake Rect Max Y")	config.searchShakeRect.Max.y = std::stoi(value);

		else if (option == "Search Bar Rect Min X")		config.searchBarRect.Min.x = std::stoi(value);
		else if (option == "Search Bar Rect Min Y")		config.searchBarRect.Min.y = std::stoi(value);
		else if (option == "Search Bar Rect Max X")		config.searchBarRect.Max.x = std::stoi(value);
		else if (option == "Search Bar Rect Max Y")		config.searchBarRect.Max.y = std::stoi(value);
	}

	file.close();

	return true;
}

bool Config::save()
{
	std::ofstream file(std::filesystem::current_path().concat("/configs/config.txt").c_str(), std::ios::out);

	if (!file.is_open())
		if (!std::filesystem::create_directory(std::filesystem::current_path().concat("/configs/")))
			return false;
		else
			file = std::ofstream(std::filesystem::current_path().concat("/configs/config.txt").c_str(), std::ios::out);

	for (const auto& comment : comments)
		file << comment << std::endl;
	file << "Click Shake Delay: " << config.clickShakeDelay << std::endl;

	file << "Auto Enable Camera Mode: " << config.autoEnableCameraMode << std::endl;
	file << "Auto Blur: " << config.autoBlur << std::endl;
	file << "Auto Look Down: " << config.autoLookDown << std::endl;
	file << "Auto Zoom In: " << config.autoZoomIn << std::endl;

	file << "Camera Mode Pos X: " << config.cameraModePos.x << std::endl;
	file << "Camera Mode Pos Y: " << config.cameraModePos.y << std::endl;

	file << "Bar Dead Zone Left X: " << config.barDeadZoneLeftPos.x << std::endl;
	file << "Bar Dead Zone Left Y: " << config.barDeadZoneLeftPos.y << std::endl;
	file << "Bar Dead Zone Right X: " << config.barDeadZoneRightPos.x << std::endl;
	file << "Bar Dead Zone Right Y: " << config.barDeadZoneRightPos.y << std::endl;

	file << "Search Shake Rect Min X: " << config.searchShakeRect.Min.x << std::endl;
	file << "Search Shake Rect Min Y: " << config.searchShakeRect.Min.y << std::endl;
	file << "Search Shake Rect Max X: " << config.searchShakeRect.Max.x << std::endl;
	file << "Search Shake Rect Max Y: " << config.searchShakeRect.Max.y << std::endl;

	file << "Search Bar Rect Min X: " << config.searchBarRect.Min.x << std::endl;
	file << "Search Bar Rect Min Y: " << config.searchBarRect.Min.y << std::endl;
	file << "Search Bar Rect Max X: " << config.searchBarRect.Max.x << std::endl;
	file << "Search Bar Rect Max Y: " << config.searchBarRect.Max.y << std::endl;

	file.close();

	return true;
}
