#ifndef CONTEXT_CLIPPING_BOX_CREATION_H
#define CONTEXT_CLIPPING_BOX_CREATION_H

#include "controller/functionSystem/ARayTracingContext.h"

class ContextClippingBoxCreation : public ARayTracingContext
{
public:
	ContextClippingBoxCreation(const ContextId& id);
	~ContextClippingBoxCreation();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;
	ContextType getType() const override;
};

#endif
