#ifndef CONTEXT_MOVE_MANIP_H
#define CONTEXT_MOVE_MANIP_H

#include "controller/functionSystem/ARayTracingContext.h"

class ContextMoveManip : public ARayTracingContext
{
public:
	ContextMoveManip(const ContextId& id);
	~ContextMoveManip();
    ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;
};

#endif // !CONTEXT_CREATE_TAG_H_