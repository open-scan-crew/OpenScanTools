#include "gui/widgets/TemplatesNode.h"

TemplateListNode::TemplateListNode(SafePtr<sma::TagTemplate> tagTemplate, QStandardItem * item)
	: QStandardItem()
{
	setTemplate(tagTemplate);
}

TemplateListNode::~TemplateListNode()
{
}

void TemplateListNode::setTemplate(SafePtr<sma::TagTemplate> tagTemplate)
{
	_tagTemplate = tagTemplate;
	ReadPtr<sma::TagTemplate> rTagTemp = tagTemplate.cget();
	if (rTagTemp)
	{
		m_originNode = rTagTemp->isAOriginTemplate();
		setText(QString::fromStdWString(rTagTemp->getName()));
	}
}

SafePtr<sma::TagTemplate> TemplateListNode::getTemplate() const
{
	return (_tagTemplate);
}

bool TemplateListNode::getOriginTemplate() const
{
	return (m_originNode);
}
