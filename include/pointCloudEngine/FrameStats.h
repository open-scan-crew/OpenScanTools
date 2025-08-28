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

    float getCumulValue(int j) const {
        float value = 0.0f;
        switch (j)
        {
        case 2:
            value += prepareScansTime;
        case 1:
            value += prepareObjectsTime;
        case 0:
            value += bakeGraph;
            break;
        case 3:
            return renderTime;
        default:
            return 0.0f;
        }
        return value;
    }
};

#endif