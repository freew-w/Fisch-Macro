#pragma once

class Config
{
public:
	struct
	{
		int clickShakeDelay = 20;
		bool autoEnableCameraMode = true;
		bool autoBlur = true;
		bool autoLookDown = true;
		bool autoZoomIn = true;
		ImVec2 cameraModePos{};
		ImRect searchShakeRect{};
		ImRect searchBarRect{};
	} config;

	static Config& get();

private:
	std::vector<std::string> comments{};

	Config();
	~Config();
	bool load();
	bool save();
};

inline Config& config = Config::get();
