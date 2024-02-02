#include "controller/messages/DataIdListMessage.h"

DataListMessage::DataListMessage(const std::unordered_set<SafePtr<AGraphNode>>& dataPtrs)
	: m_dataPtrs(dataPtrs)
	, m_type(ElementType::None)
{}

DataListMessage::DataListMessage(const std::unordered_set<SafePtr<AGraphNode>>& dataPtrs, const ElementType& type)
	: m_dataPtrs(dataPtrs)
	, m_type(type)
{}

DataListMessage::~DataListMessage()
{}

IMessage::MessageType DataListMessage::getType() const
{
	return (IMessage::MessageType::DATAID_LIST);
}

IMessage* DataListMessage::copy() const
{
	return new DataListMessage(*this);
}