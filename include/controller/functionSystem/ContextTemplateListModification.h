#ifndef CONTEXT_TEMPLATE_LIST_MODIFICATION_H_
#define CONTEXT_TEMPLATE_LIST_MODIFICATION_H_

#include "controller/functionSystem/AContext.h"
#include "models/OpenScanToolsModelEssentials.h"
#include "controller/messages/TemplateListMessage.h"
#include <unordered_set>

class AGraphNode;

class ContextTemplateListModification : public AContext
{
public:
	ContextTemplateListModification(const ContextId& id);
	~ContextTemplateListModification();
	ContextState start(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;

private:
	std::unordered_set<SafePtr<AGraphNode>>	m_affectedTags;
	SafePtr<sma::TagTemplate>				m_temp;
	sma::tFieldId							m_fieldId;
	SafePtr<UserList>						m_newType;

};

#endif // !CONTEXT_TEMPLATE_LIST_MODIFICATION_H_

