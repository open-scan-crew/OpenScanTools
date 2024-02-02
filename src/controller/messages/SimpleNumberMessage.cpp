#include "controller/messages/SimpleNumberMessage.h"


SimpleNumberMessage::SimpleNumberMessage(const uint64_t& value)
	: m_returnedValue(value)
{}

SimpleNumberMessage::~SimpleNumberMessage()
{}

IMessage::MessageType SimpleNumberMessage::getType() const
{
	return IMessage::MessageType::SIMPLE_NUMBER;
}

IMessage* SimpleNumberMessage::copy() const
{
	return new SimpleNumberMessage(*this);
}

DoubleVectorMessage::DoubleVectorMessage(const std::vector<double>& value)
	: m_returnedValue(value)
{
}

DoubleVectorMessage::~DoubleVectorMessage()
{
}

IMessage::MessageType DoubleVectorMessage::getType() const
{
	return MessageType::DOUBLE_VECTOR;
}

IMessage* DoubleVectorMessage::copy() const
{
	return new DoubleVectorMessage(*this);
}
