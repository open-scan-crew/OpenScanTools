#ifndef GRID_BOX_H
#define GRID_BOX_H

#include "models/graph/TransformationModule.h"

class GridBox : public TransformationModule
{
public:
	GridBox();
	GridBox(const glm::vec3& _center, const glm::quat& _orientation, const glm::vec3& _size);
public:
	union
	{
		uint64_t id;
		glm::lowp_i16vec4 position;
	};
};
#endif