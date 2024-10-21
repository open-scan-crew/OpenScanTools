#ifndef CONTEXT_FIT_TORUS_H_
#define CONTEXT_FIT_TORUS_H_

#include "controller/functionSystem/ARayTracingContext.h"

class ContextFitTorus : public ARayTracingContext
{
public:
	ContextFitTorus(const ContextId& id);
	~ContextFitTorus();
	ContextState start(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller);
	ContextState launch(Controller& controller);

	bool canAutoRelaunch() const;
	ContextType getType() const override;

};

#endif // !CONTEXT_FIT_TORUS_H_
