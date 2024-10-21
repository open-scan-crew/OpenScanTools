#ifndef TEMPLATE_MESSAGE_H_
#define TEMPLATE_MESSAGE_H_

#include "controller/messages/IMessage.h"
#include "models/application/TagTemplate.h"

class TemplateMessage : public IMessage
{
public:
	TemplateMessage(const SafePtr<sma::TagTemplate>& temp, const sma::tFieldId& fieldId, const sma::tFieldType& newType);
	~TemplateMessage();
	MessageType getType() const;	
	IMessage* copy() const;

public:
	const SafePtr<sma::TagTemplate> m_temp;
	const sma::tFieldId m_fieldId;
	const sma::tFieldType m_newType;
};

#endif //! TEMPLATE_MESSAGE_H_