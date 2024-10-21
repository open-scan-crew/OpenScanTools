#ifndef CONTEXT_BIG_CYLINDER_FIT_H
#define CONTEXT_BIG_CYLINDER_FIT_H

#include "controller/functionSystem/ARayTracingContext.h"

class ContextBigCylinderFit : public ARayTracingContext
{
public:
	ContextBigCylinderFit(const ContextId& id);
	~ContextBigCylinderFit();
	void resetClickUsages();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;
};

#endif //CONTEXT_BIG_CYLINDER_FIT_H