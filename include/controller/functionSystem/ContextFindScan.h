#ifndef CONTEXT_FIND_SCAN_H_
#define CONTEXT_FIND_SCAN_H_

#include "controller/functionSystem/ARayTracingContext.h"

class ContextFindScan : public ARayTracingContext
{
public:
	ContextFindScan(const ContextId& id);
	~ContextFindScan();
	ContextState start(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller);
	ContextState launch(Controller& controller);

	bool canAutoRelaunch() const;
	ContextType getType() const override;

};

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

#endif // !CONTEXT_CREATE_TAG_H_
