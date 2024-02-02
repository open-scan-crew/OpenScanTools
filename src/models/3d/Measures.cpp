#include "models/3d/Measures.h"

#define M_PI       3.14159265358979323846

double Measure::getDistanceTotal(glm::dmat4 const& toLocal) const
{
    glm::dvec3 vector(final - origin);
    return (glm::length(vector));
}

double Measure::getDistanceAlongX(glm::dmat4 const& toLocal) const
{
    glm::dvec3 vector(final - origin);
    return abs(vector.x * toLocal[0][0] + vector.y * toLocal[1][0] + vector.z * toLocal[2][0]);
}

double Measure::getDistanceAlongY(glm::dmat4 const& toLocal) const
{
    glm::dvec3 vector(final - origin);
    return abs(vector.x * toLocal[0][1] + vector.y * toLocal[1][1] + vector.z * toLocal[2][1]);
}

double Measure::getDistanceAlongZ(glm::dmat4 const& toLocal) const
{
    glm::dvec3 vector(final - origin);
    return abs(vector.x * toLocal[0][2] + vector.y * toLocal[1][2] + vector.z * toLocal[2][2]);
}

double Measure::getDistanceHorizontal(glm::dmat4 const& toLocal) const
{
    glm::dvec3 vector(final - origin);
    double xLocal = vector.x * toLocal[0][0] + vector.y * toLocal[1][0] + vector.z * toLocal[2][0];
    double yLocal = vector.x * toLocal[0][1] + vector.y * toLocal[1][1] + vector.z * toLocal[2][1];

    return (std::sqrt(xLocal * xLocal + yLocal * yLocal));
}

double Measure::getAngleHorizontal(glm::dmat4 const& toLocal) const
{
    glm::dvec4 vector(final - origin, 0.0);
    vector = toLocal * vector;
    double hori = std::sqrt(vector.x * vector.x + vector.y * vector.y);
    double angleRad = std::atan2(vector.z, hori);
    return (angleRad / M_PI * 180.0);
}