#ifndef GUI_RENDERING_TYPES_H
#define GUI_RENDERING_TYPES_H

#include <filesystem>

struct RampScale
{
    bool showScale;
    bool centerBoxScale;
    int graduationCount;
    bool showTemperatureScale;
    std::filesystem::path temperatureScaleFile;
    // enum colorScheme ??
};

#endif
