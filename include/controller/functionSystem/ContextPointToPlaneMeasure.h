#ifndef POINT_TO_PLANE_H
#define POINT_TO_PLANE_H

#include "controller/functionSystem/ARayTracingContext.h"

class ContextPointToPlaneMeasure : public ARayTracingContext
{
public:
	ContextPointToPlaneMeasure(const ContextId& id);
	~ContextPointToPlaneMeasure();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;

	bool canAutoRelaunch() const;
	ContextType getType() const override;

};

#endif //POINT_TO_PLANE_H