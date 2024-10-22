#ifndef SEGMENT_DRAW_DATA_H
#define SEGMENT_DRAW_DATA_H

#include "pointCloudEngine/ShowTypes.h"

#include <cstdint>

#include <glm/glm.hpp>

struct SegmentDrawData
{
    SegmentDrawData(glm::dvec4 A, glm::dvec4 B, uint32_t color_, uint32_t index_, MeasureShowMask mask_)
        : pointA{ (float)A.x, (float)A.y, (float)A.z }
        , pointB{ (float)B.x, (float)B.y, (float)B.z }
        , colorRGBA(color_)
        , index(index_)
        , showMask(mask_)
    {}

    float pointA[3];
    float pointB[3];
    uint32_t colorRGBA;
    uint32_t index;
    MeasureShowMask showMask;
};

#endif