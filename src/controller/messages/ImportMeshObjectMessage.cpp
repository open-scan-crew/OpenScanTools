#include "controller/messages/ImportMeshObjectMessage.h"

/*
* ImportMeshObjectMessage
*/

ImportMeshObjectMessage::ImportMeshObjectMessage(const FileInputData& data)
	: m_data(data)
{}

ImportMeshObjectMessage::~ImportMeshObjectMessage()
{}

IMessage::MessageType ImportMeshObjectMessage::getType() const
{
	return MessageType::IMPORT_MESHOBJECT;
}

IMessage* ImportMeshObjectMessage::copy() const
{
	return new ImportMeshObjectMessage(*this);
}

/*
* StepSimplificationMessage
*/

StepSimplificationMessage::StepSimplificationMessage(const FileInputData& data, StepClassification & classification, const double & keepPercent, const std::filesystem::path& outputFile, bool importAfter)
	: m_data(data), m_classification(classification), m_keepPercent(keepPercent)
	, m_outputPath(outputFile), m_importAfter(importAfter)
{}

StepSimplificationMessage::~StepSimplificationMessage()
{}

IMessage::MessageType StepSimplificationMessage::getType() const
{
	return MessageType::STEP_SIMPLIFICATION;
}

IMessage * StepSimplificationMessage::copy() const
{
	return new StepSimplificationMessage(*this);
}
