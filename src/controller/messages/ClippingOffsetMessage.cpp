#include "controller/messages/ClippingOffsetMessage.h"

ClippingOffsetMessage::ClippingOffsetMessage(const ClippingOffset& offset)
	: m_offset(offset)
{}

ClippingOffsetMessage::~ClippingOffsetMessage()
{}

IMessage::MessageType ClippingOffsetMessage::getType() const
{
	return IMessage::MessageType::CLIPPING_OFFSET;
}

const ClippingOffset& ClippingOffsetMessage::getOffset() const
{
	return m_offset;
}