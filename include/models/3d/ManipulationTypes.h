#ifndef MANIPULATION_TYPES_H
#define MANIPULATION_TYPES_H

#include <glm/glm.hpp>
#include "glm/gtc/quaternion.hpp"

enum class ManipulationMode { Translation, Rotation, Scale, Extrusion };
enum class Selection { None, X, _X, Y, _Y, Z, _Z, XY, _XY, X_Y, _X_Y, XZ, _XZ, X_Z, _X_Z, YZ, _YZ, Y_Z, _Y_Z, MAX_ENUM };

struct ManipulateData
{
	glm::dvec3	translation = glm::dvec3(0.);
	glm::dvec3	addScale = glm::dvec3(0.);
	glm::dquat	addRotation = glm::dquat(1., 0., 0., 0.);
	glm::dvec3  globalRotationCenter = glm::dvec3(0.);

	void addManipulateData(const ManipulateData& manipData)
	{
		translation += manipData.translation;
		addScale += manipData.addScale;
		addRotation = manipData.addRotation * addRotation;
		globalRotationCenter = manipData.globalRotationCenter;
	}
};

#endif