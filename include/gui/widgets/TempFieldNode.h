#include <QtGui/QStandardItemModel>
#include "models/application/TagTemplate.h"

#ifndef TEMPFIELD_NODE_H_
#define TEMPFIELD_NODE_H_

class TempFieldNode : public QStandardItem
{
public:
	explicit TempFieldNode(QString& data, sma::templateId templateId, sma::tFieldId fieldId, QStandardItem *item = nullptr);
	~TempFieldNode();

	void setTemplateId(sma::templateId id);
	void setFieldId(sma::tFieldId id);

	sma::templateId getTemplateId() const;
	sma::tFieldId getFieldId() const;

private:
	sma::templateId _tempId;
	sma::tFieldId _fieldId;
};

#endif // !TEMPFIELD_NODE_H_