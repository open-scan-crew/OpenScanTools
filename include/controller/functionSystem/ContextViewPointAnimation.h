#ifndef CONTEXT_VIEWPOINT_ANIMATION_H_
#define CONTEXT_VIEWPOINT_ANIMATION_H_

#include "controller/functionSystem/AContext.h"
#include "models/OpenScanToolsModelEssentials.h"

class ContextViewPointAnimation : public AContext
{
public:
	ContextViewPointAnimation(const ContextId& id);
	~ContextViewPointAnimation();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;

private:
	//ViewPoint m_currentContext;
};

#endif // !CONTEXT_VIEWPOINT_ANIMATION_H_
