#include "controller/messages/TemplateListMessage.h"


TemplateListMessage::TemplateListMessage(const SafePtr<sma::TagTemplate>& temp, const sma::tFieldId& fieldId, const SafePtr<UserList>& newType)
	: m_temp(temp)
	, m_fieldId(fieldId)
	, m_newType(newType)
{}

TemplateListMessage::~TemplateListMessage()
{}

IMessage::MessageType TemplateListMessage::getType() const
{
	return  IMessage::MessageType::TEMPLATE_LIST;
}

IMessage* TemplateListMessage::copy() const
{
	return new TemplateListMessage(*this);
}