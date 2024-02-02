#ifndef CONTEXT_EXAMINE_H
#define CONTEXT_EXAMINE_H

#include "controller/functionSystem/ARayTracingContext.h"

class IPanel;

class ContextExamine : public ARayTracingContext
{
public:
	ContextExamine(const ContextId& id);
	~ContextExamine();
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	ContextState abort(Controller& controller) override;

	ContextType getType() const override;
	bool canAutoRelaunch() const;

private:
	SafePtr<CameraNode> m_target;
};

#endif
