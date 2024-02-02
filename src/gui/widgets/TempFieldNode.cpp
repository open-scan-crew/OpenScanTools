#include "gui/widgets/TempFieldNode.h"

TempFieldNode::TempFieldNode(QString & data, sma::templateId templateId, sma::tFieldId fieldId, QStandardItem * item)
	: QStandardItem(data)
{
	_tempId = templateId;
	_fieldId = fieldId;
}

TempFieldNode::~TempFieldNode()
{
}

void TempFieldNode::setTemplateId(sma::templateId id)
{
	_tempId = id;
}

void TempFieldNode::setFieldId(sma::tFieldId id)
{
	_fieldId = id;
}

sma::templateId TempFieldNode::getTemplateId() const
{
	return (_tempId);
}

sma::tFieldId TempFieldNode::getFieldId() const
{
	return (_fieldId);
}