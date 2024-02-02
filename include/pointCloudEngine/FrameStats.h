#ifndef FRAME_STATS_H
#define FRAME_STATS_H

#include <cstdint>

struct FrameStats {
    uint64_t cellCount;
    uint64_t pointCount;
    float bakeGraph;
    float prepareScansTime;
    float prepareObjectsTime;
    float renderTime;
    float decimation;
};

#endif