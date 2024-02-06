#include "controller/messages/ConvertionMessage.h"


ConvertionMessage::ConvertionMessage(const ConvertProperties& prop)
	: m_properties(prop)
{}

//Note (Aurélien) Unused for the moment
/*
ConvertionMessage::ConvertionMessage(const PrecisionType& precision)
	: m_precision(precision)
	, m_type(FileType::UNKNOW)
	, m_isImport(false)
	, m_isCSV(false)
	, m_output("")
	, m_mask(0)
{}

ConvertionMessage::ConvertionMessage(const uint64_t& precision)
	: m_precision((PrecisionType)precision)
	, m_type(FileType::UNKNOW)
	, m_isImport(false)
	, m_isCSV(false)
	, m_output("")
	, m_mask(0)
{}

ConvertionMessage::ConvertionMessage(const FileType& type)
	: m_type(type)
	, m_precision(PrecisionType::TL_OCTREE_100UM)
	, m_isImport(false)
	, m_isCSV(false)
	, m_output("")
	, m_mask(0)
{}*/

ConvertionMessage::~ConvertionMessage()
{}

IMessage::MessageType ConvertionMessage::getType() const
{
	return IMessage::MessageType::CONVERTION_OPTION;
}

IMessage* ConvertionMessage::copy() const
{
	return new ConvertionMessage(*this);
}

tls::PrecisionType ConvertionMessage::getPrecision() const
{
	return (tls::PrecisionType)(m_properties.filePrecision);
}

FileType ConvertionMessage::getFileType() const
{
    return (FileType)(m_properties.fileFormat);
}

const uint64_t& ConvertionMessage::getMask() const
{
	return m_properties.mask;
}

const bool& ConvertionMessage::isCSV() const
{
	return m_properties.isCSV;
}

const bool& ConvertionMessage::isImport() const
{
	return m_properties.isImport;
}

const std::filesystem::path& ConvertionMessage::getOutput() const
{
	return m_properties.output;
}

AsciiInfoMessage::AsciiInfoMessage(const std::map<std::filesystem::path, Import::AsciiInfo>& asciiInfo)
	: m_asciiInfo(asciiInfo)
{}

AsciiInfoMessage::~AsciiInfoMessage()
{}

IMessage::MessageType AsciiInfoMessage::getType() const
{
	return MessageType::ASCII_INFO;
}

IMessage* AsciiInfoMessage::copy() const
{
	return new AsciiInfoMessage(*this);
}
