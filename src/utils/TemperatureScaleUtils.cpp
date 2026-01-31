#include "utils/TemperatureScaleUtils.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace
{
    std::string stripBom(const std::string& line)
    {
        if (line.size() >= 3 &&
            static_cast<unsigned char>(line[0]) == 0xEF &&
            static_cast<unsigned char>(line[1]) == 0xBB &&
            static_cast<unsigned char>(line[2]) == 0xBF)
        {
            return line.substr(3);
        }
        return line;
    }

    bool parseTemperatureScaleLine(const std::string& rawLine, TemperatureScaleEntry& entry)
    {
        if (rawLine.empty())
            return false;

        std::string line = rawLine;
        std::replace(line.begin(), line.end(), ';', ' ');

        std::istringstream stream(line);
        std::string token;
        std::vector<std::string> tokens;
        while (stream >> token)
        {
            tokens.push_back(token);
        }

        if (tokens.size() < 4)
            return false;

        try
        {
            int r = std::stoi(tokens[0]);
            int g = std::stoi(tokens[1]);
            int b = std::stoi(tokens[2]);
            double temp = std::stod(tokens[3]);

            if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255)
                return false;

            entry.r = static_cast<uint8_t>(r);
            entry.g = static_cast<uint8_t>(g);
            entry.b = static_cast<uint8_t>(b);
            entry.temperature = temp;
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
}

TemperatureScaleData TemperatureScaleUtils::loadTemperatureScaleFile(const std::filesystem::path& filePath, std::string& errorMessage)
{
    TemperatureScaleData data;
    data.filePath = filePath;

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        errorMessage = "Unable to open file.";
        return data;
    }

    bool headerSkipped = false;
    std::string line;
    while (std::getline(file, line))
    {
        line = stripBom(line);
        line.erase(std::remove_if(line.begin(), line.end(), [](unsigned char c) { return c == '\r' || c == '\n'; }), line.end());
        if (line.empty())
            continue;

        TemperatureScaleEntry entry;
        if (!parseTemperatureScaleLine(line, entry))
        {
            if (!headerSkipped && data.entries.empty())
            {
                headerSkipped = true;
                continue;
            }
            errorMessage = "Invalid temperature scale file line.";
            return data;
        }

        data.entries.push_back(entry);
    }

    if (data.entries.empty())
    {
        errorMessage = "No data found.";
        return data;
    }

    data.rgbToTemperature.clear();
    data.rgbToTemperature.reserve(data.entries.size());
    for (const TemperatureScaleEntry& entry : data.entries)
    {
        uint32_t key = makeTemperatureScaleKey(entry.r, entry.g, entry.b);
        if (data.rgbToTemperature.find(key) == data.rgbToTemperature.end())
            data.rgbToTemperature.emplace(key, entry.temperature);
    }

    data.isValid = true;
    return data;
}
