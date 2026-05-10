#ifndef RAY_TRACING_DISPLAY_FILTER_H
#define RAY_TRACING_DISPLAY_FILTER_H

#include "models/3d/DisplayParameters.h"

struct RayTracingDisplayFilterSettings
{
    bool enabled = false;
    UiRenderMode renderMode = UiRenderMode::RGB;
    ColorimetricFilterSettings colorimetricFilter = {};
    PolygonalSelectorSettings polygonalSelector = {};
};

#endif
