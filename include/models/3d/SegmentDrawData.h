#ifndef SEGMENT_DRAW_DATA_H
#define SEGMENT_DRAW_DATA_H

#include "pointCloudEngine/ShowTypes.h"

#include <cstdint>

struct SegmentDrawData
{
    float pointA[3];
    float pointB[3];
    uint32_t colorRGBA;
    uint32_t index;
    MeasureShowMask showMask;
};

#endif