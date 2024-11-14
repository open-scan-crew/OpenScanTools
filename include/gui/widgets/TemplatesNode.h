#include <QtGui/qstandarditemmodel.h>
#include "models/application/TagTemplate.h"
#include "models/OpenScanToolsModelEssentials.h"

#ifndef TEMPLATE_NODE_H_
#define TEMPLATE_NODE_H_

class TemplateListNode : public QStandardItem
{
public:
	explicit TemplateListNode(SafePtr<sma::TagTemplate> tagTemplate, QStandardItem *item = nullptr);
	~TemplateListNode();

	void setTemplate(SafePtr<sma::TagTemplate> tagTemplate);

	SafePtr<sma::TagTemplate> getTemplate() const;
	bool getOriginTemplate() const;

private:
	SafePtr<sma::TagTemplate> _tagTemplate;
	bool m_originNode;
};

#endif // !TEMPLATE_NODE_H_