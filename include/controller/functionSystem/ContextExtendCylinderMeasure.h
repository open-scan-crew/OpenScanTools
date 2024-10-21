#ifndef CONTEXT_EXTEND_CYLINDER_H_
#define CONTEXT_EXTEND_CYLINDER_H_

#include "controller/functionSystem/ARayTracingContext.h"

class ContextExtendCylinder : public ARayTracingContext
{
public:
	ContextExtendCylinder(const ContextId& id);
	~ContextExtendCylinder();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;

};

#endif // !CONTEXT_EXTEND_CYLINDER_H_
