#ifndef PLANE_TO_CYLINDER_H
#define PLANE_TO_CYLINDER_H

#include "controller/functionSystem/ARayTracingContext.h"

class ContextPointToCylinderMeasure : public ARayTracingContext
{
public:
	ContextPointToCylinderMeasure(const ContextId& id);
	~ContextPointToCylinderMeasure();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;

	bool canAutoRelaunch() const;
	ContextType getType() const override;

};

#endif //PLANE_TO_CYLINDER_H