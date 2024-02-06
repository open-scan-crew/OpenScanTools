#ifndef IMPORT_WAVEFRONT_MESSAGE_H_
#define IMPORT_WAVEFRONT_MESSAGE_H_

#include "controller/messages/IMessage.h"
#include "io/MeshObjectTypes.h"
#include "io/imports/ImportTypes.h"
#include "io/imports/step-simplification/step-simplification.h"

class ImportScanMessage : public IMessage
{
public:
	ImportScanMessage(const Import::ScanInfo& data);
	~ImportScanMessage();
	MessageType getType() const;
	IMessage* copy() const;

public:
	Import::ScanInfo m_data;
};

class ImportMeshObjectMessage : public IMessage
{
public:
	ImportMeshObjectMessage(const FileInputData& data);
	~ImportMeshObjectMessage();
	MessageType getType() const;
	IMessage* copy() const;

public:
	const FileInputData m_data;
};

class StepSimplificationMessage : public IMessage
{
public:
	StepSimplificationMessage(const FileInputData& data, StepClassification& classification, const double& keepPercent, const std::filesystem::path& outputPath, bool importAfter);
	~StepSimplificationMessage();
	MessageType getType() const;
	IMessage* copy() const;

public:
	StepClassification m_classification;
	double m_keepPercent;
	FileInputData m_data;

	std::filesystem::path m_outputPath;
	bool m_importAfter;
};

#endif //! CLICKMESSAGE_H_