#ifndef TEMPERATURE_SCALE_UTILS_H
#define TEMPERATURE_SCALE_UTILS_H

#include "models/3d/TemperatureScaleData.h"

#include <string>

namespace TemperatureScaleUtils
{
    TemperatureScaleData loadTemperatureScaleFile(const std::filesystem::path& filePath, std::string& errorMessage);
}

#endif // TEMPERATURE_SCALE_UTILS_H
