#include "controller/messages/ColorAvailableListMessage.h"


ColorAvailableListMessage::ColorAvailableListMessage(const std::list<glm::vec3>& colors)
	: m_colorList(colors)
{}

IMessage::MessageType ColorAvailableListMessage::getType() const
{
	return IMessage::MessageType::COLOR_AVAILABLE_LIST;
}

IMessage* ColorAvailableListMessage::copy() const
{
	return new ColorAvailableListMessage(*this);
}