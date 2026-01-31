#ifndef CONTEXT_PICK_TEMPERATURE_H
#define CONTEXT_PICK_TEMPERATURE_H

#include "controller/functionSystem/ARayTracingContext.h"

class ContextPickTemperature : public ARayTracingContext
{
public:
	explicit ContextPickTemperature(const ContextId& id);
	~ContextPickTemperature() override;

	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const override;
	ContextType getType() const override;
};

#endif // CONTEXT_PICK_TEMPERATURE_H
