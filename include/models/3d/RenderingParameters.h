#ifndef RENDERING_PARAMETERS_H_
#define RENDERING_PARAMETERS_H_

#include "models/3d/ProjectionData.h"
#include "models/3d/DisplayParameters.h"

class RenderingParameters : public ProjectionData, public DisplayParameters
{

public:
	RenderingParameters();
	RenderingParameters(const ProjectionData& cam, const DisplayParameters& display);

	void setRenderingParameters(const RenderingParameters& parameters);
	const DisplayParameters& getDisplayParameters() const;

};

#endif // !RENDERING_PARAMETERS_H_