#ifndef COLORIMETRIC_FILTER_UTILS_H
#define COLORIMETRIC_FILTER_UTILS_H

#include "models/3d/DisplayParameters.h"
#include "pointCloudEngine/RenderingTypes.h"
#include "utils/Color32.hpp"

#include <array>
#include <vector>

namespace ColorimetricFilterUtils
{
    struct OrderedColorEntry
    {
        Color32 color;
        bool enabled = false;
    };

    std::vector<Color32> getOrderedActiveColors(const ColorimetricFilterSettings& settings, UiRenderMode mode);
    std::array<OrderedColorEntry, 4> normalizeSettings(const ColorimetricFilterSettings& settings, UiRenderMode mode);
    float clampTolerancePercent(float tolerance);
    glm::vec3 normalizeRgb(const Color32& color);
    float normalizeIntensity(const Color32& color);
}

#endif // COLORIMETRIC_FILTER_UTILS_H
