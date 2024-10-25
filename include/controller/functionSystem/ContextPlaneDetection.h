#ifndef CONTEXT_PLANE_DETECTION_H
#define CONTEXT_PLANE_DETECTION_H

#include "controller/functionSystem/ARayTracingContext.h"
#include "controller/functionSystem/PlaneDetectionOptions.h"

class ContextPlaneDetection : public ARayTracingContext
{
public:
	ContextPlaneDetection(const ContextId& id);
	~ContextPlaneDetection();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;

	PlaneDetectionOptions m_options;

private:
	void  resetClickUsages();
};

#endif // !CONTEXT_PLANE_DETECTION_H

