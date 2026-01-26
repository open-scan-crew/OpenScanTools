#ifndef COLOR_BALANCE_SETTINGS_H
#define COLOR_BALANCE_SETTINGS_H

#include "models/pointCloud/PointXYZIRGB.h"

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

struct ColorBalanceSettings
{
    size_t neighborCount = 24;
    double trimRatio = 0.2;
    double beta = 2.5;
    bool applyIntensity = true;
    bool applyRGB = true;
};

struct ColorBalanceGlobalGrid
{
    glm::dvec3 origin = { 0.0, 0.0, 0.0 };
    double voxelSize = 0.01;
    size_t maxSamplesPerVoxel = 64;
    std::unordered_map<int64_t, std::vector<PointXYZIRGB>> samples;
};

#endif
