#ifndef CONTEXT_MULTIPLE_CYLINDERS_MEASURE_H
#define CONTEXT_MULTIPLE_CYLINDERS_MEASURE_H

#include "controller/functionSystem/ARayTracingContext.h"

class ContextMultipleCylinders : public ARayTracingContext
{
public:
	ContextMultipleCylinders(const ContextId& id);
	~ContextMultipleCylinders();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;
};

#endif // !CCONTEXT_MULTIPLE_CYLINDERS_MEASURE_H
