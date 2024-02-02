#include "controller/messages/ClickMessage.h"

ClickMessage::ClickMessage(const Pos3D& pos, const IPanel* target)
	: m_pos(pos)
	, m_target(target)
{};

ClickMessage::~ClickMessage()
{}
	
IMessage::MessageType ClickMessage::getType() const
{
	return  IMessage::MessageType::CLICK;
}

IMessage* ClickMessage::copy() const
{
	return new ClickMessage(*this);
}

