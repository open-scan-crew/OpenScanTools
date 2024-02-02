#include "controller/messages/GeneralMessage.h"

GeneralMessage::GeneralMessage(GeneralInfo info)
	: m_info(info)
{}

GeneralMessage::~GeneralMessage()
{}

IMessage::MessageType GeneralMessage::getType() const
{
	return IMessage::MessageType::GENERALMESSAGE;
}

IMessage* GeneralMessage::copy() const
{
	return new GeneralMessage(*this);
}
