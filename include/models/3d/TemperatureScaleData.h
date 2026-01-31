#ifndef TEMPERATURE_SCALE_DATA_H
#define TEMPERATURE_SCALE_DATA_H

#include <cstdint>
#include <filesystem>
#include <unordered_map>
#include <vector>

struct TemperatureScaleEntry
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    double temperature = 0.0;
};

struct TemperatureScaleData
{
    std::filesystem::path filePath;
    std::vector<TemperatureScaleEntry> entries;
    std::unordered_map<uint32_t, double> rgbToTemperature;
    bool isValid = false;
};

inline uint32_t makeTemperatureScaleKey(uint8_t r, uint8_t g, uint8_t b)
{
    return (static_cast<uint32_t>(r) << 16)
        | (static_cast<uint32_t>(g) << 8)
        | static_cast<uint32_t>(b);
}

#endif // TEMPERATURE_SCALE_DATA_H
