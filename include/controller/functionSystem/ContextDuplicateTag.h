#ifndef CONTEXT_DUPLICATE_TAG_H
#define CONTEXT_DUPLICATE_TAG_H

#include "controller/functionSystem/ARayTracingContext.h"
#include "models/OpenScanToolsModelEssentials.h"

class AGraphNode;

class ContextDuplicateTag : public ARayTracingContext
{
public:
	ContextDuplicateTag(const ContextId& id);
	~ContextDuplicateTag();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	bool refreshDupId(Controller& controller);

	ContextType getType() const override;

private:
	SafePtr<AGraphNode>	m_toDup;
};

#endif
