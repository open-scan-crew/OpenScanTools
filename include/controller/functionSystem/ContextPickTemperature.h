#ifndef CONTEXT_PICK_TEMPERATURE_H_
#define CONTEXT_PICK_TEMPERATURE_H_

#include "controller/functionSystem/ARayTracingContext.h"

class ContextPickTemperature : public ARayTracingContext
{
public:
	ContextPickTemperature(const ContextId& id);
	~ContextPickTemperature();
	ContextState start(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller);
	ContextState launch(Controller& controller);

	bool canAutoRelaunch() const;
	ContextType getType() const override;

private:
	bool findTemperatureFromPick(Controller& controller, double& temperature, uint8_t& r, uint8_t& g, uint8_t& b);
};

#endif // CONTEXT_PICK_TEMPERATURE_H_
