#include "controller/messages/PipeMessage.h"

PipeConnetionMessage::PipeConnetionMessage(const double& RonDext, const bool& applyAngleConstraints, const bool& keepDiameter) : m_RonDext(RonDext),m_angleConstraints(applyAngleConstraints), m_keepDiameter(keepDiameter)
{}

PipeConnetionMessage::~PipeConnetionMessage()
{}

IMessage::MessageType PipeConnetionMessage::getType() const
{
	return IMessage::MessageType::PIPECONNECTIONMESSAGE;
}

IMessage* PipeConnetionMessage::copy() const
{
	return new PipeConnetionMessage(m_RonDext,m_angleConstraints, m_keepDiameter);
}

const double& PipeConnetionMessage::getRonDext() const
{
	return m_RonDext;
}

const bool& PipeConnetionMessage::getAngleConstraints() const
{
	return m_angleConstraints;
}

const bool& PipeConnetionMessage::getKeepDiameter() const
{
	return m_keepDiameter;
}