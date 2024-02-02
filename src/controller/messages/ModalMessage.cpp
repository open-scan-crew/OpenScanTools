#include "controller/messages/ModalMessage.h"


ModalMessage::ModalMessage(const uint32_t& value)
	: m_returnedValue(value)
{}

ModalMessage::~ModalMessage()
{}
	
IMessage::MessageType ModalMessage::getType() const
{
	return IMessage::MessageType::MODAL;
}

IMessage* ModalMessage::copy() const
{
	return new ModalMessage(*this);
}