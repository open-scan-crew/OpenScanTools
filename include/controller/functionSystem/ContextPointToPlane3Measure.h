#ifndef POINT_TO_PLANE3_H
#define POINT_TO_PLANE3_H

#include "controller/functionSystem/ARayTracingContext.h"

class ContextPointToPlane3Measure : public ARayTracingContext
{
public:
	ContextPointToPlane3Measure(const ContextId& id);
	~ContextPointToPlane3Measure();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;

	bool canAutoRelaunch() const;
	ContextType getType() const override;

};

#endif //POINT_TO_PLANE3_H
