#ifndef GUI_RENDERING_TYPES_H
#define GUI_RENDERING_TYPES_H

#include "utils/Color32.hpp"

struct RampScale
{
    bool showScale;
    bool centerBoxScale;
    int graduationCount;
    bool showTemperatureScale;
    // enum colorScheme ??
};

struct ColorimetricFilterSettings
{
    bool enabled = false;
    bool showColors = true;
    float tolerance = 0.0f; // percent [0..100]
    Color32 colors[4] = { Color32(0, 0, 0), Color32(0, 0, 0), Color32(0, 0, 0), Color32(0, 0, 0) };
    bool colorsEnabled[4] = { false, false, false, false };
};

#endif
