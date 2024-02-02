#include "controller/messages/ColorMessage.h"

ColorMessage::ColorMessage(const glm::vec3& color)
	: m_color(color)
{}

ColorMessage::~ColorMessage()
{}

IMessage::MessageType ColorMessage::getType() const
{
	return IMessage::MessageType::COLOR;
}

IMessage* ColorMessage::copy() const
{
	return new ColorMessage(*this);
}