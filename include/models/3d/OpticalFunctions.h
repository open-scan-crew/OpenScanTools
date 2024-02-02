#ifndef OPTICAL_FUNCTIONS_H
#define OPTICAL_FUNCTIONS_H

#include "pointCloudEngine/RenderingTypes.h"

constexpr int c_min_near_plan_log2 = -12;
constexpr int c_max_near_plan_log2 = 8;
constexpr int c_max_near_far_ratio_log2 = 16;
constexpr int c_min_ortho_range_log2 = 0;
constexpr int c_max_ortho_range_log2 = 20;

namespace tls
{
    double getFarValue(PerspectiveZBounds zBounds);
    double getNearValue(PerspectiveZBounds zBounds);
    double getOrthographicZBoundsValue(OrthographicZBounds zBounds);
}

#endif