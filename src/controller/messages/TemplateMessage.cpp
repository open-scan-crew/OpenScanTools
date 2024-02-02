#include "controller/messages/TemplateMessage.h"


TemplateMessage::TemplateMessage(const SafePtr<sma::TagTemplate>& temp, const sma::tFieldId& fieldId, const sma::tFieldType& newType)
	: m_temp(temp)
	, m_fieldId(fieldId)
	, m_newType(newType)
{}

TemplateMessage::~TemplateMessage()
{}

IMessage::MessageType TemplateMessage::getType() const
{
	return  IMessage::MessageType::TEMPLATE;
}

IMessage* TemplateMessage::copy() const
{
	return new TemplateMessage(*this);
}