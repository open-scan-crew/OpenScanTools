#ifndef CONTEXT_TEMPLATE_MODIFICATION_H_
#define CONTEXT_TEMPLATE_MODIFICATION_H_

#include "controller/functionSystem/AContext.h"
#include "models/OpenScanToolsModelEssentials.h"
#include "controller/messages/TemplateMessage.h"
#include <unordered_set>

class AGraphNode;

class ContextTemplateModification : public AContext
{
public:
	ContextTemplateModification(const ContextId& id);
	~ContextTemplateModification();
	ContextState start(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;

private:
	std::unordered_set<SafePtr<AGraphNode>>	m_affectedTags;
	SafePtr<sma::TagTemplate> m_temp;
	sma::tFieldId		m_fieldId;
	sma::tFieldType		m_newType;

};

#endif // !CONTEXT_TEMPLATE_MODIFICATION_H_

