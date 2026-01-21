#ifndef POINT_CLOUD_SMOOTHING_ENGINE_H
#define POINT_CLOUD_SMOOTHING_ENGINE_H

#include <filesystem>
#include <unordered_map>

#include "glm/glm.hpp"

struct PointXYZIRGB;

class TlsFileWriter;

struct SmoothPointCloudParameters
{
    double maxDisplacementMm = 1.0;
    double voxelSizeMm = 2.0;
    bool adaptiveVoxel = true;
    bool preserveEdges = true;
};

class PointCloudSmoothingEngine
{
public:
    bool smoothScan(const std::filesystem::path& scanPath,
                    TlsFileWriter* writer,
                    const SmoothPointCloudParameters& parameters,
                    uint64_t& outProcessedPoints,
                    std::wstring& log) const;

private:
    struct VoxelKey
    {
        int64_t x;
        int64_t y;
        int64_t z;

        bool operator==(const VoxelKey& other) const;
    };

    struct VoxelKeyHasher
    {
        std::size_t operator()(const VoxelKey& key) const;
    };

    struct VoxelAccum
    {
        uint64_t count = 0;
        glm::dvec3 sum = glm::dvec3(0.0);
        glm::dvec3 sumSq = glm::dvec3(0.0);
    };

    using VoxelMap = std::unordered_map<VoxelKey, VoxelAccum, VoxelKeyHasher>;

    static VoxelKey makeKey(const glm::dvec3& point, double voxelSizeMeters);
    static void accumulate(VoxelMap& map, const VoxelKey& key, const glm::dvec3& point);
    static glm::dvec3 computeCentroid(const VoxelAccum& accum);
    static glm::dvec3 computeVariance(const VoxelAccum& accum);
};

#endif
