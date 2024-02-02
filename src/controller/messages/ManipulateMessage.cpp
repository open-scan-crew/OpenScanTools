#include "controller/messages/ManipulateMessage.h"

ManipulateMessage::ManipulateMessage(bool rotate, ZMovement zmove)
	: m_rotate(rotate)
	, m_zmove(zmove)
{}

ManipulateMessage::~ManipulateMessage()
{}

IMessage::MessageType ManipulateMessage::getType() const
{
	return  IMessage::MessageType::MANIPULATE;
}

IMessage* ManipulateMessage::copy() const
{
	return new ManipulateMessage(*this);
}