#ifndef IMPORTMESSAGE_H_
#define IMPORTMESSAGE_H_

#include <filesystem>
#include <vector>

#include "controller/messages/IMessage.h"

class FilesMessage : public IMessage
{
public:
	FilesMessage(const std::vector<std::filesystem::path>& inputFile, int typeOfInput = 0);
	~FilesMessage();
	MessageType getType() const;	
	IMessage* copy() const;

public:
	const std::vector<std::filesystem::path> m_inputFiles;
	int									   m_typeOfInput;
};

#endif //! IMPORTMESSAGE_H_