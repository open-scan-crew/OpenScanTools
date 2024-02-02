#include "models/3d/GridBox.h"

GridBox::GridBox()
{}

GridBox::GridBox(const glm::vec3& _center, const glm::quat& _orientation, const glm::vec3& _size)
	:TransformationModule(_center,_orientation,_size)
{}