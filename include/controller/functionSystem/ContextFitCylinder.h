#ifndef CONTEXT_FIT_CYLINDER_H
#define CONTEXT_FIT_CYLINDER_H

#include "controller/functionSystem/ARayTracingContext.h"
#include <glm/glm.hpp>
#include <list>
#include <map>
#include "controller/functionSystem/PipeDetectionOptions.h"

class ContextFitCylinder : public ARayTracingContext
{
public:
	ContextFitCylinder(const ContextId& id);
	~ContextFitCylinder();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;

	PipeDetectionOptions m_options;

private:
    void  resetClickUsages(Controller& controller);
};

#endif // !CONTEXT_FAST_CYLINDER_H_
