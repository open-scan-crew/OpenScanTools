#ifndef MEASURE_H
#define MEASURE_H

#include <glm/glm.hpp>

struct Measure
{
	glm::dvec3 origin;
	glm::dvec3 final;

    double getDistanceTotal(glm::dmat4 const& toLocal = glm::dmat4(1.0)) const;
    double getDistanceAlongX(glm::dmat4 const& toLocal = glm::dmat4(1.0)) const;
    double getDistanceAlongY(glm::dmat4 const& toLocal = glm::dmat4(1.0)) const;
    double getDistanceAlongZ(glm::dmat4 const& toLocal = glm::dmat4(1.0)) const;
    double getDistanceHorizontal(glm::dmat4 const& toLocal = glm::dmat4(1.0)) const;
    double getAngleHorizontal(glm::dmat4 const& toLocal = glm::dmat4(1.0)) const;
};

enum class Reliability
{
	NA,
	reliable,
	unreliable
};

enum class RatioSup
{
	NA,
	yes,
	no
};

#endif // ! MEASURE_H_