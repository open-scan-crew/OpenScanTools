#include "pointCloudEngine/PointCloudSmoothingEngine.h"

#include <cmath>

#include "io/exports/TlsFileWriter.h"
#include "io/imports/TlsFileReader.h"
#include "models/pointCloud/PointXYZIRGB.h"

namespace
{
    constexpr uint64_t kBufferPointCount = 1 << 16;
    constexpr uint64_t kMinPointsPerVoxel = 10;
    constexpr uint64_t kMinPointsPerCoarseVoxel = 6;
    constexpr double kMillimetersToMeters = 0.001;
}

bool PointCloudSmoothingEngine::smoothScan(const std::filesystem::path& scanPath,
                                           TlsFileWriter* writer,
                                           const SmoothPointCloudParameters& parameters,
                                           uint64_t& outProcessedPoints,
                                           std::wstring& log) const
{
    if (writer == nullptr)
        return false;

    TlsFileReader* reader = nullptr;
    if (!TlsFileReader::getReader(scanPath, log, &reader) || reader == nullptr)
        return false;

    if (!reader->startReadingScan(0))
    {
        delete reader;
        return false;
    }

    const double voxelSizeMeters = parameters.voxelSizeMm * kMillimetersToMeters;
    const double coarseVoxelSizeMeters = voxelSizeMeters * 2.0;

    VoxelMap baseVoxels;
    VoxelMap coarseVoxels;
    baseVoxels.reserve(1024);
    coarseVoxels.reserve(1024);

    std::vector<PointXYZIRGB> buffer(kBufferPointCount);
    uint64_t readCount = 0;
    outProcessedPoints = 0;

    do
    {
        if (!reader->readPoints(buffer.data(), buffer.size(), readCount))
            break;

        for (uint64_t i = 0; i < readCount; ++i)
        {
            glm::dvec3 point(buffer[i].x, buffer[i].y, buffer[i].z);
            VoxelKey baseKey = makeKey(point, voxelSizeMeters);
            accumulate(baseVoxels, baseKey, point);

            if (parameters.adaptiveVoxel)
            {
                VoxelKey coarseKey = makeKey(point, coarseVoxelSizeMeters);
                accumulate(coarseVoxels, coarseKey, point);
            }
        }
        outProcessedPoints += readCount;
    } while (readCount > 0);

    delete reader;

    reader = nullptr;
    if (!TlsFileReader::getReader(scanPath, log, &reader) || reader == nullptr)
        return false;

    if (!reader->startReadingScan(0))
    {
        delete reader;
        return false;
    }

    const double maxDisplacementMeters = parameters.maxDisplacementMm * kMillimetersToMeters;
    outProcessedPoints = 0;

    do
    {
        if (!reader->readPoints(buffer.data(), buffer.size(), readCount))
            break;

        for (uint64_t i = 0; i < readCount; ++i)
        {
            PointXYZIRGB& pointRef = buffer[i];
            glm::dvec3 point(pointRef.x, pointRef.y, pointRef.z);
            VoxelKey baseKey = makeKey(point, voxelSizeMeters);

            const VoxelAccum* accum = nullptr;
            double voxelSizeForPoint = voxelSizeMeters;
            uint64_t minPoints = kMinPointsPerVoxel;

            auto baseIt = baseVoxels.find(baseKey);
            if (baseIt != baseVoxels.end() && baseIt->second.count >= kMinPointsPerVoxel)
            {
                accum = &baseIt->second;
            }
            else if (parameters.adaptiveVoxel)
            {
                VoxelKey coarseKey = makeKey(point, coarseVoxelSizeMeters);
                auto coarseIt = coarseVoxels.find(coarseKey);
                if (coarseIt != coarseVoxels.end())
                {
                    accum = &coarseIt->second;
                    voxelSizeForPoint = coarseVoxelSizeMeters;
                    minPoints = kMinPointsPerCoarseVoxel;
                }
            }

            if (accum == nullptr || accum->count < minPoints)
                continue;

            if (parameters.preserveEdges)
            {
                glm::dvec3 variance = computeVariance(*accum);
                double varianceMagnitude = variance.x + variance.y + variance.z;
                double varianceThreshold = (voxelSizeForPoint * 0.45) * (voxelSizeForPoint * 0.45);
                if (varianceMagnitude > varianceThreshold)
                    continue;
            }

            glm::dvec3 centroid = computeCentroid(*accum);
            glm::dvec3 delta = centroid - point;
            double distance = glm::length(delta);
            if (distance > maxDisplacementMeters && distance > 0.0)
            {
                delta *= maxDisplacementMeters / distance;
            }

            pointRef.x = static_cast<float>(point.x + delta.x);
            pointRef.y = static_cast<float>(point.y + delta.y);
            pointRef.z = static_cast<float>(point.z + delta.z);
        }

        if (!writer->addPoints(buffer.data(), readCount))
        {
            delete reader;
            return false;
        }
        outProcessedPoints += readCount;
    } while (readCount > 0);

    delete reader;
    return true;
}

bool PointCloudSmoothingEngine::VoxelKey::operator==(const VoxelKey& other) const
{
    return x == other.x && y == other.y && z == other.z;
}

std::size_t PointCloudSmoothingEngine::VoxelKeyHasher::operator()(const VoxelKey& key) const
{
    std::size_t seed = 0;
    seed ^= std::hash<int64_t>{}(key.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= std::hash<int64_t>{}(key.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= std::hash<int64_t>{}(key.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
}

PointCloudSmoothingEngine::VoxelKey PointCloudSmoothingEngine::makeKey(const glm::dvec3& point, double voxelSizeMeters)
{
    return {
        static_cast<int64_t>(std::floor(point.x / voxelSizeMeters)),
        static_cast<int64_t>(std::floor(point.y / voxelSizeMeters)),
        static_cast<int64_t>(std::floor(point.z / voxelSizeMeters))
    };
}

void PointCloudSmoothingEngine::accumulate(VoxelMap& map, const VoxelKey& key, const glm::dvec3& point)
{
    VoxelAccum& accum = map[key];
    accum.count += 1;
    accum.sum += point;
    accum.sumSq += glm::dvec3(point.x * point.x, point.y * point.y, point.z * point.z);
}

glm::dvec3 PointCloudSmoothingEngine::computeCentroid(const VoxelAccum& accum)
{
    if (accum.count == 0)
        return glm::dvec3(0.0);
    double invCount = 1.0 / static_cast<double>(accum.count);
    return accum.sum * invCount;
}

glm::dvec3 PointCloudSmoothingEngine::computeVariance(const VoxelAccum& accum)
{
    if (accum.count == 0)
        return glm::dvec3(0.0);
    double invCount = 1.0 / static_cast<double>(accum.count);
    glm::dvec3 mean = accum.sum * invCount;
    glm::dvec3 meanSq = accum.sumSq * invCount;
    return glm::max(meanSq - glm::dvec3(mean.x * mean.x, mean.y * mean.y, mean.z * mean.z), glm::dvec3(0.0));
}
