#ifndef CONVERTION_MESSAGE_H_
#define CONVERTION_MESSAGE_H_

#include <filesystem>
#include "controller/controls/ControlProject.h"
#include "controller/messages/IMessage.h"
#include "models/pointCloud/TLS.h"
#include "io/FileUtils.h"
#include "io/ConvertProperties.h"
#include "io/imports/ImportTypes.h"

class ConvertionMessage : public IMessage
{
public:
    ConvertionMessage(const ConvertProperties& prop);
	~ConvertionMessage();
	MessageType getType() const;	
	IMessage* copy() const;

	tls::PrecisionType getPrecision() const;
	FileType getFileType() const;
	const uint64_t& getMask() const;
	const bool& isCSV() const;
	const bool& isImport() const;
	const std::filesystem::path& getOutput() const;

public:
    ConvertProperties m_properties;
};

class AsciiInfoMessage : public IMessage
{
public:
	AsciiInfoMessage(const std::map<std::filesystem::path, Import::AsciiInfo>& asciiColumnValues);
	~AsciiInfoMessage();
	MessageType getType() const;
	IMessage* copy() const;

public:
	std::map<std::filesystem::path, Import::AsciiInfo> m_asciiInfo;
};

#endif //! CONVERTIONMESSAGE_H_