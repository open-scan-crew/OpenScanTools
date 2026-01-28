#ifndef COLOR_BALANCE_TYPES_H
#define COLOR_BALANCE_TYPES_H

#include <array>
#include <cstdint>
#include <unordered_map>
#include <cmath>

#include "glm/glm.hpp"

struct ColorBalanceCellKey
{
    int64_t x = 0;
    int64_t y = 0;
    int64_t z = 0;

    bool operator==(const ColorBalanceCellKey& other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct ColorBalanceCellKeyHasher
{
    size_t operator()(const ColorBalanceCellKey& key) const noexcept
    {
        size_t h1 = std::hash<int64_t>{}(key.x);
        size_t h2 = std::hash<int64_t>{}(key.y);
        size_t h3 = std::hash<int64_t>{}(key.z);
        size_t seed = h1;
        seed ^= h2 + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
        seed ^= h3 + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
        return seed;
    }
};

struct ColorBalanceCellHistogram
{
    uint64_t count = 0;
    std::array<uint32_t, 256> lumaHist{};
    std::array<uint32_t, 256> intensityHist{};
};

struct ColorBalanceCellCorrection
{
    double gainL = 1.0;
    double offsetL = 0.0;
    double gainI = 1.0;
    double offsetI = 0.0;
    float confidenceL = 0.0f;
    float confidenceI = 0.0f;
    bool hasColor = false;
    bool hasIntensity = false;
};

using ColorBalanceHistogramMap = std::unordered_map<ColorBalanceCellKey, ColorBalanceCellHistogram, ColorBalanceCellKeyHasher>;
using ColorBalanceCorrectionMap = std::unordered_map<ColorBalanceCellKey, ColorBalanceCellCorrection, ColorBalanceCellKeyHasher>;

inline int64_t colorBalanceFloorDiv(int64_t value, int64_t divisor)
{
    if (divisor == 0)
        return 0;
    if (value >= 0)
        return value / divisor;
    return -(((-value) + divisor - 1) / divisor);
}

inline ColorBalanceCellKey colorBalanceCellKeyFromGlobal(const glm::dvec3& globalCoord, double cellSize)
{
    if (cellSize <= 0.0)
        return {};
    return {
        static_cast<int64_t>(std::floor(globalCoord.x / cellSize)),
        static_cast<int64_t>(std::floor(globalCoord.y / cellSize)),
        static_cast<int64_t>(std::floor(globalCoord.z / cellSize))
    };
}

inline ColorBalanceCellKey colorBalanceParentCellKey(const ColorBalanceCellKey& key, int level)
{
    if (level <= 0)
        return key;
    int64_t divisor = static_cast<int64_t>(1) << level;
    return {
        colorBalanceFloorDiv(key.x, divisor),
        colorBalanceFloorDiv(key.y, divisor),
        colorBalanceFloorDiv(key.z, divisor)
    };
}

#endif
