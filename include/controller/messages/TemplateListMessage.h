#ifndef TEMPLATE_LIST_MESSAGE_H_
#define TEMPLATE_LIST_MESSAGE_H_

#include "controller/messages/IMessage.h"
#include "pointCloudEngine/RenderingTypes.h"
#include "models/application/TagTemplate.h"

class TemplateListMessage : public IMessage
{
public:
	TemplateListMessage(const SafePtr<sma::TagTemplate>& temp, const sma::tFieldId& fieldId, const SafePtr<UserList>& newType);
	~TemplateListMessage();
	MessageType getType() const;	
	IMessage* copy() const;

public:
	const SafePtr<sma::TagTemplate> m_temp;
	const sma::tFieldId m_fieldId;
	const SafePtr<UserList> m_newType;
};

#endif //! TEMPLATE_LIST_MESSAGE_H_