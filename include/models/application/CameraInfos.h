#ifndef CAMERAINFOS_H_
#define CAMERAINFOS_H_

#include <glm/glm.hpp>
#include "pointCloudEngine/RenderingTypes.h"

struct CameraInfos
{
	glm::dvec3 position = glm::dvec3(NAN);
	double theta = NAN;
	double phi = NAN;
	ProjectionMode mode = ProjectionMode::Perspective;
};

#endif // !CAMERAINFOS_H_