#ifndef CYLINDER_TO_PLANE3_H
#define CYLINDER_TO_PLANE3_H

#include "controller/functionSystem/ARayTracingContext.h"
#include <glm/glm.hpp>

class ContextCylinderToPlane3Measure : public ARayTracingContext
{
public:
	ContextCylinderToPlane3Measure(const ContextId& id);
	~ContextCylinderToPlane3Measure();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;

	bool canAutoRelaunch() const;
	ContextType getType() const override;

private:
	double m_cylinderRadius;
	glm::dvec3 m_cylinderCenter, m_cylinderDirection;
};

#endif //CYLINDER_TO_PLANE3_H