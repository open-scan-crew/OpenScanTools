#ifndef COLORIMETRIC_FILTER_UTILS_H
#define COLORIMETRIC_FILTER_UTILS_H

#include "pointCloudEngine/GuiRenderingTypes.h"

#include <array>

#include <glm/glm.hpp>

namespace ColorimetricFilterUtils
{
    struct OrderedColorSet
    {
        std::array<Color32, 4> colors = { Color32(0, 0, 0), Color32(0, 0, 0), Color32(0, 0, 0), Color32(0, 0, 0) };
        int count = 0;
    };

    OrderedColorSet buildOrderedColorSet(const ColorimetricFilterSettings& settings);
    std::array<Color32, 4> reorderQuadColors(const std::array<Color32, 4>& colors);
    glm::vec3 toVec3(const Color32& color);
}

#endif
