#ifndef OUTLIER_STATS_H
#define OUTLIER_STATS_H

#include <cstdint>

struct OutlierStats
{
    uint64_t count = 0;
    double mean = 0.0;
    double stddev = 0.0;
};

enum class SofPreAnalysisRiskClass
{
    Low,
    Borderline,
    High
};

enum class SofPreAnalysisAction
{
    NoSubdivision,
    SubdivisionRecommended
};

struct SofPreAnalysisStats
{
    uint64_t occupiedCells = 0;
    uint64_t testedPointsEstimate = 0;
    double spacingMean = 0.0;
    double spacingStddev = 0.0;
    double spacingCv = 0.0;
    double meanDistanceMean = 0.0;
    double meanDistanceStddev = 0.0;
    double meanDistanceCv = 0.0;
    double interZoneVarMeanDistance = 0.0;
    double interZoneVarSpacing = 0.0;
    double nonEmptyZoneRatio = 0.0;
    double riskScore = 0.0;
    SofPreAnalysisRiskClass riskClass = SofPreAnalysisRiskClass::Low;
    SofPreAnalysisAction recommendedAction = SofPreAnalysisAction::NoSubdivision;
};

#endif
