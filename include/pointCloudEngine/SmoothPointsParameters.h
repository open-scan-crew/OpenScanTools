#ifndef SMOOTH_POINTS_PARAMETERS_H
#define SMOOTH_POINTS_PARAMETERS_H

struct SmoothPointsParameters
{
    double maxDistanceMm = 0.5;
    int kNeighbors = 16;
    double alpha = 3.0;
    bool planeFilterEnabled = true;
    double planeDistanceFactor = 0.5;
    bool normalFilterEnabled = false;
    double normalAngleDeg = 20.0;
    int keepBestPercent = 80;
};

#endif
