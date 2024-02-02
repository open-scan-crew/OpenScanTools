#include "controller/messages/Vec3Message.h"

Vec3Message::Vec3Message(const glm::vec3& vec3)
	:m_vec3(vec3)
{}

Vec3Message::~Vec3Message()
{}

IMessage::MessageType Vec3Message::getType() const
{
	return MessageType::VECTOR_3;
}

IMessage* Vec3Message::copy() const
{
	return new Vec3Message(*this);
}