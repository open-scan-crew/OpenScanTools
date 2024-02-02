#include "models/3d/RenderingParameters.h"

RenderingParameters::RenderingParameters()
{}

RenderingParameters::RenderingParameters(const ProjectionData& cam, const DisplayParameters& display)
	: ProjectionData(cam)
	, DisplayParameters(display)
{}

void RenderingParameters::setRenderingParameters(const RenderingParameters& parameters)
{
	(*this) = parameters;
}

const DisplayParameters& RenderingParameters::getDisplayParameters() const
{
	return (*this);
}