#ifndef CONTEXT_DELETE_TAG_H_
#define CONTEXT_DELETE_TAG_H_

#include "controller/functionSystem/AContext.h"
#include "utils/safe_ptr.h"
#include <unordered_set>

class AGraphNode;

class ContextDeleteTag : public AContext
{
public:
	ContextDeleteTag(const ContextId& id);
	~ContextDeleteTag();
	ContextState start(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;

private:
	std::unordered_set<SafePtr<AGraphNode>> m_list;
};

#endif // !CONTEXT_DELETE_TAG_H_

