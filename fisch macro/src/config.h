#pragma once

class Config
{
public:
	struct
	{
		int clickShakeDelay = 14;
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
