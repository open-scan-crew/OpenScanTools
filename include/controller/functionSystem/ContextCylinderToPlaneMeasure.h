#ifndef CYLINDER_TO_PLANE_H
#define CYLINDER_TO_PLANE_H

#include "controller/functionSystem/ARayTracingContext.h"
#include <glm/glm.hpp>
#include <list>
#include <map>

class ContextCylinderToPlaneMeasure : public ARayTracingContext
{
public:
	ContextCylinderToPlaneMeasure(const ContextId& id);
	~ContextCylinderToPlaneMeasure();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;

	bool canAutoRelaunch() const;
	ContextType getType() const override;

private:
	double m_cylinderRadius;
	glm::dvec3 m_cylinderCenter, m_cylinderDirection;

};

#endif //CYLINDER_TO_PLANE_H