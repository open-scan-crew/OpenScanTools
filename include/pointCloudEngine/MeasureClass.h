#ifndef MEASURE_CLASS_H
#define MEASURE_CLASS_H

#include <glm/glm.hpp>
#include <vector>

class MeasureClass
{
	MeasureClass() = delete;
	~MeasureClass() = delete;

public:
	static glm::dvec3 projectPointToPlane(const glm::dvec3& point, const std::vector<double>& plane);
	static glm::dvec3 projectPointToLine(const glm::dvec3& point, const glm::dvec3& lineDirection, const glm::dvec3& linePoint);
	static glm::dvec3 projectPointToPlaneAlongVector(const glm::dvec3& point, const glm::dvec3& projDirection, const std::vector<double>& plane);
	static glm::dvec3 intersectionLinePlane(const glm::dvec3& planePoint, const glm::dvec3 planeNormal, const glm::dvec3& linePoint, const glm::dvec3& lineDirection);

};

#endif //MULTIPLE_MEASURES_H