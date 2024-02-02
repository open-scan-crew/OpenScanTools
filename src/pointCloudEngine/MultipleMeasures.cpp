#include "pointCloudEngine/MeasureClass.h"

glm::dvec3  MeasureClass::projectPointToPlane(const glm::dvec3& point, const std::vector<double>& plane)
{
	glm::dvec3 normalVector(plane[0], plane[1], plane[2]);
	normalVector /= glm::length(normalVector);
	glm::dvec3 planePoint;
	if ((abs(plane[0]) >= abs(plane[1])) && (abs(plane[0]) >= abs(plane[2])))
		planePoint = glm::dvec3(-plane[3] / plane[0], 0.0, 0.0);
	if ((abs(plane[1]) >= abs(plane[0])) && (abs(plane[1]) >= abs(plane[2])))
		planePoint = glm::dvec3(0.0, -plane[3] / plane[1], 0.0);
	if ((abs(plane[2]) >= abs(plane[0])) && (abs(plane[2]) >= abs(plane[1])))
		planePoint = glm::dvec3(0.0, 0.0, -plane[3] / plane[2]);
	return (point + glm::dot(planePoint - point, normalVector)*normalVector);
}

glm::dvec3 MeasureClass::projectPointToLine(const glm::dvec3& point, const glm::dvec3& lineDirection, const glm::dvec3& linePoint)
{
	return(linePoint - glm::dot(linePoint - point, lineDirection)*lineDirection);
}

glm::dvec3 MeasureClass::projectPointToPlaneAlongVector(const glm::dvec3& point, const glm::dvec3& projDirection, const std::vector<double>& plane)
{
	// result= point+ t * projDir
	// glm::dot(result, planeNormel)=-plane[3]
	// glm::dot(point,planeNormal) + t* glm::dot(projDir,planeNormal)=-plane[3]
	// t = (-plane[3]-glm::dot(point,planeNormal) / glm::dot(projDir,planeNormal)

	if ((int)plane.size() != 4)
		return glm::dvec3(0.0, 0.0, 0.0);
	glm::dvec3 normalVector(plane[0], plane[1], plane[2]);
	normalVector /= glm::length(normalVector);
	return point + (-plane[3] - glm::dot(point, normalVector)) / glm::dot(projDirection, normalVector) * projDirection;
}

glm::dvec3 MeasureClass::intersectionLinePlane(const glm::dvec3& planePoint, const glm::dvec3 planeNormal, const glm::dvec3& linePoint, const glm::dvec3& lineDirection)
{
	// result= linePoint+t*lineDirection st
	// dot(result-planePoint,normal)=0
	// dot(lineP - planeP,normal) + t*(lineD,normal)=0
	//break if dot(lineD,normal)=0
	//otherwise
	//result=linePoint - dot(lineP-planeP,normal)/dot(lineD,normal) * lineD

	if (abs(glm::dot(lineDirection, planeNormal)) < 0.00001)
		return glm::dvec3(0.0, 0.0, 0.0);
	else
		return linePoint - (glm::dot(linePoint - planePoint, planeNormal) / glm::dot(lineDirection, planeNormal)) * lineDirection;
}