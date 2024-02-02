#ifndef CONTEXT_CREATE_TAG_H_
#define CONTEXT_CREATE_TAG_H_

#include "controller/functionSystem/ARayTracingContext.h"
#include "models/OpenScanToolsModelEssentials.h"

class ContextCreateTag : public ARayTracingContext
{
public:
	ContextCreateTag(const ContextId& id);
	~ContextCreateTag();
	ContextState start(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller);
	ContextState launch(Controller& controller);

	bool canAutoRelaunch() const;
	ContextType getType() const override;

};

#endif // !CONTEXT_CREATE_TAG_H_
