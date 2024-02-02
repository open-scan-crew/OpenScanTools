#include "controller/messages/FilesMessage.h"

FilesMessage::FilesMessage(const std::vector<std::filesystem::path>& inputFiles, int typeOfInput)
	: m_inputFiles(inputFiles)
	, m_typeOfInput(typeOfInput)
{}

FilesMessage::~FilesMessage()
{}

IMessage::MessageType FilesMessage::getType() const
{
	return IMessage::MessageType::FILES;
}

IMessage* FilesMessage::copy() const
{
	return new FilesMessage(*this);
}