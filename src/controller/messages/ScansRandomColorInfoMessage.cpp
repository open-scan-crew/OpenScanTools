#include "controller/messages/ScansRandomColorInfoMessage.h"

ScansRandomColorInfoMessage::ScansRandomColorInfoMessage(const std::vector<ScanColorInfo>& scans)
	: m_scans(scans)
{}

IMessage::MessageType ScansRandomColorInfoMessage::getType() const
{
	return MessageType::SCANS_RANDOM_COLOR_INFO;
}

IMessage* ScansRandomColorInfoMessage::copy() const
{
	return new ScansRandomColorInfoMessage(*this);
}