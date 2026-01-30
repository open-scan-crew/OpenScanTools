#ifndef TEMPERATURE_SCALE_PARSER_H
#define TEMPERATURE_SCALE_PARSER_H

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace temperature_scale
{
	struct TemperatureScaleEntry
	{
		uint8_t r = 0;
		uint8_t g = 0;
		uint8_t b = 0;
		double temperature = 0.0;
	};

	struct TemperatureScaleData
	{
		std::vector<TemperatureScaleEntry> entries;
		std::unordered_map<uint32_t, double> lookup;
	};

	inline std::unordered_map<std::filesystem::path, TemperatureScaleData>& cacheData()
	{
		static std::unordered_map<std::filesystem::path, TemperatureScaleData> cache;
		return cache;
	}

	inline std::unordered_map<std::filesystem::path, std::filesystem::file_time_type>& cacheTimes()
	{
		static std::unordered_map<std::filesystem::path, std::filesystem::file_time_type> cache;
		return cache;
	}

	inline uint32_t makeRgbKey(uint8_t r, uint8_t g, uint8_t b)
	{
		return (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
	}

	inline bool loadTemperatureScaleFile(const std::filesystem::path& path, TemperatureScaleData& outData, std::string* errorMessage)
	{
		outData.entries.clear();
		outData.lookup.clear();

		std::ifstream file(path);
		if (!file.is_open())
		{
			if (errorMessage)
				*errorMessage = "Unable to open file.";
			return false;
		}

		bool headerSkipped = false;
		std::string line;
		while (std::getline(file, line))
		{
			if (line.empty())
				continue;

			for (char& c : line)
			{
				if (c == ';')
					c = ' ';
			}

			std::istringstream iss(line);
			int r = 0;
			int g = 0;
			int b = 0;
			double temperature = 0.0;

			if (!(iss >> r >> g >> b >> temperature))
			{
				if (!headerSkipped && outData.entries.empty())
				{
					headerSkipped = true;
					continue;
				}
				if (errorMessage)
					*errorMessage = "Invalid line.";
				return false;
			}

			if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255)
			{
				if (errorMessage)
					*errorMessage = "RGB out of range.";
				return false;
			}

			TemperatureScaleEntry entry;
			entry.r = static_cast<uint8_t>(r);
			entry.g = static_cast<uint8_t>(g);
			entry.b = static_cast<uint8_t>(b);
			entry.temperature = temperature;
			outData.entries.push_back(entry);

			uint32_t key = makeRgbKey(entry.r, entry.g, entry.b);
			if (outData.lookup.find(key) == outData.lookup.end())
				outData.lookup.emplace(key, entry.temperature);
		}

		if (outData.entries.empty())
		{
			if (errorMessage)
				*errorMessage = "No valid entries.";
			return false;
		}

		return true;
	}

	inline bool getTemperatureScaleData(const std::filesystem::path& path, TemperatureScaleData& outData, std::string* errorMessage)
	{
		std::error_code ec;
		if (!std::filesystem::exists(path, ec))
		{
			cacheData().erase(path);
			cacheTimes().erase(path);
			if (errorMessage)
				*errorMessage = "File not found.";
			return false;
		}

		auto lastWrite = std::filesystem::last_write_time(path, ec);
		if (!ec)
		{
		auto it = cacheTimes().find(path);
		if (it != cacheTimes().end() && it->second == lastWrite)
		{
			outData = cacheData()[path];
			return true;
		}
		}

		TemperatureScaleData data;
		std::string error;
		if (!loadTemperatureScaleFile(path, data, &error))
		{
			cacheData().erase(path);
			cacheTimes().erase(path);
			if (errorMessage)
				*errorMessage = error;
			return false;
		}

		cacheData()[path] = data;
		cacheTimes()[path] = lastWrite;
		outData = data;
		return true;
	}

	inline void clearTemperatureScaleCache(const std::filesystem::path& path)
	{
		cacheData().erase(path);
		cacheTimes().erase(path);
	}
}

#endif
