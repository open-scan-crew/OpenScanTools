#include "controller/messages/PlaneMessage.h"

PlaneMessage::PlaneMessage(const PlaneDetectionOptions& options)
	: m_options(options)
{}

PlaneMessage::~PlaneMessage()
{}

IMessage::MessageType PlaneMessage::getType() const
{
	return IMessage::MessageType::PLANEMESSAGE;
}

IMessage* PlaneMessage::copy() const
{
	return new PlaneMessage(m_options);
}

PlaneDetectionOptions PlaneMessage::getOptions() const
{
	return m_options;
}

BeamBendingMessage::BeamBendingMessage(const BeamBendingOptions& options)
	: m_options(options)
{}

BeamBendingOptions BeamBendingMessage::getOptions() const
{
	return m_options;
}

BeamBendingMessage::~BeamBendingMessage()
{}

IMessage::MessageType BeamBendingMessage::getType() const
{
	return IMessage::MessageType::BEAMBENDINGMESSAGE;
}

IMessage* BeamBendingMessage::copy() const
{
	return new BeamBendingMessage(m_options);
}

SetOfPointsMessage::SetOfPointsMessage(const SetOfPointsOptions& options, const bool& userAxes, const bool& createMeasures, const bool& fromTop, const double& step, const double& threshold, const bool& horizontal)
	: m_options(options), m_userAxes(userAxes), m_createMeasures(createMeasures), m_fromTop(fromTop), m_step(step),m_threshold(threshold),m_horizontal(horizontal)
{}

SetOfPointsOptions SetOfPointsMessage::getOptions() const
{
	return m_options;
}

bool SetOfPointsMessage::getCreateMeasures() const
{
	return m_createMeasures;
}

bool SetOfPointsMessage::getUserAxes() const
{
	return m_userAxes;
}

bool SetOfPointsMessage::getFromTop() const
{
	return m_fromTop;
}

bool SetOfPointsMessage::getHorizontal() const
{
	return m_horizontal;
}

double SetOfPointsMessage::getStep() const
{
	return m_step;
}

double SetOfPointsMessage::getThreshold() const
{
	return m_threshold;
}

SetOfPointsMessage::~SetOfPointsMessage()
{}

IMessage::MessageType SetOfPointsMessage::getType() const
{
	return IMessage::MessageType::SETOFPOINTSMESSAGE;
}

IMessage* SetOfPointsMessage::copy() const
{
	return new SetOfPointsMessage(m_options, m_userAxes, m_createMeasures, m_fromTop,m_step,m_threshold,m_horizontal);
}
