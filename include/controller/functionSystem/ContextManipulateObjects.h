#ifndef CONTEXT_MANIPULATE_OBJECTS_H
#define CONTEXT_MANIPULATE_OBJECTS_H

#include "controller/functionSystem/ARayTracingContext.h"
#include "controller/messages/ManipulateMessage.h"

class ContextManipulateObjects : public ARayTracingContext
{
public:
	ContextManipulateObjects(const ContextId& id);
	~ContextManipulateObjects();
    ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;
private:
	std::unordered_set<SafePtr<AGraphNode>> m_toMove;

	bool m_rotate = false;
	ZMovement m_zmove = ZMovement::Default;

};

#endif // !CONTEXT_CREATE_TAG_H_