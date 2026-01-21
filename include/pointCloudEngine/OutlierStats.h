#ifndef OUTLIER_STATS_H
#define OUTLIER_STATS_H

#include <cstdint>

struct OutlierStats
{
    uint64_t count = 0;
    double mean = 0.0;
    double stddev = 0.0;
};

#endif
