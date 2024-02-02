#ifndef CONTEXT_MOVE_TAG_H
#define CONTEXT_MOVE_TAG_H

#include "controller/functionSystem/ARayTracingContext.h"

class ContextMoveTag : public ARayTracingContext
{
public:
	ContextMoveTag(const ContextId& id);
	~ContextMoveTag();
    ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;

private:
	SafePtr<AGraphNode> m_toMoveData;

};

#endif // !CONTEXT_CREATE_TAG_H_