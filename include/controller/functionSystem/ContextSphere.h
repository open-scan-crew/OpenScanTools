#ifndef CONTEXT_SPHERE_H_
#define CONTEXT_SPHERE_H_

#include "controller/functionSystem/ARayTracingContext.h"

class ContextSphere : public ARayTracingContext
{
public:
	ContextSphere(const ContextId& id);
	~ContextSphere();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;
};

#endif // !CONTEXT_SPHERE_H_