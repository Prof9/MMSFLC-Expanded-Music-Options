#include <Enums_Internal.hpp>

#include <fstream>

#include <reframework/API.hpp>

#include "Common.hpp"
#include "MusicSystemMod.hpp"

/// @brief Load mod settings from file
void MusicSystemMod::loadSettings()
{
	auto &api = reframework::API::get();
	try
	{
		std::ifstream file;
		file.open(CONFIG_FILE("config.bin"), std::fstream::in | std::fstream::binary);
		if (file)
		{
			file.read((char *)&s_settings, sizeof(s_settings));
			if (file.gcount() < sizeof(s_settings))
			{
				api->log_warn("Not all settings could be loaded from file");
			}
		}
	}
	catch (...)
	{
		s_settings = {};
		api->log_error("Failed to load settings from file!");
	}
}

/// @brief Save mod settings to file
void MusicSystemMod::saveSettings()
{
	auto &api = reframework::API::get();
	try
	{
		// Write settings to file
		std::ofstream file;
		file.open(CONFIG_FILE("config.bin"), std::fstream::out | std::fstream::binary | std::fstream::trunc);
		if (!file)
		{
			api->log_error("Failed to create config file");
			return;
		}
		if (!file.write((char *)&s_settings, sizeof(s_settings)))
		{
			api->log_error("Failed to write to config file");
			return;
		}
	}
	catch (...)
	{
		api->log_error("Failed to write to config file");
	}
}
