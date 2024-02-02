#ifndef CYLINDER_TO_CYLINDER_H
#define CYLINDER_TO_CYLINDER_H

#include "controller/functionSystem/ARayTracingContext.h"
#include <glm/glm.hpp>

class ContextCylinderToCylinderMeasure : public ARayTracingContext
{
public:
	ContextCylinderToCylinderMeasure(const ContextId& id);
	~ContextCylinderToCylinderMeasure();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;

	bool canAutoRelaunch() const;
	ContextType getType() const override;

private:
    bool findCylinder(Controller& controller, const glm::dvec3& seedPoint, double& _cylinderRadius, glm::dvec3& _cylinderCenter, glm::dvec3& _cylinderDir);
    void createCylinder(Controller& controller, double radius, glm::dvec3 center, glm::dvec3 direction, double lenght);

private:
	double m_cylinder1Radius;
	glm::dvec3 m_cylinder1Center;
	glm::dvec3 m_cylinder1Direction;
};

#endif //CYLINDER_TO_CYLINDER_H