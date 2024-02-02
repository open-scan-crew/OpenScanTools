#ifndef PLANE_MESSAGE_H
#define PLANE_MESSAGE_H

#include "controller/messages/IMessage.h"
#include "controller/functionSystem/PlaneDetectionOptions.h"

class PlaneMessage : public IMessage
{
public:
	PlaneMessage(const PlaneDetectionOptions& options);
	~PlaneMessage();
	PlaneDetectionOptions getOptions() const;

	MessageType getType() const;
	virtual IMessage* copy() const;

private:
	PlaneDetectionOptions m_options;
};

class BeamBendingMessage : public IMessage
{
public:
	BeamBendingMessage(const BeamBendingOptions& options);
	~BeamBendingMessage();
	BeamBendingOptions getOptions() const;

	MessageType getType() const;
	virtual IMessage* copy() const;

private:
	BeamBendingOptions m_options;
};

class SetOfPointsMessage : public IMessage
{
public:
	SetOfPointsMessage(const SetOfPointsOptions& options, const bool& userAxes, const bool& createMeasures, const bool& fromTop, const double& step, const double& threshold, const bool& horizontal);
	~SetOfPointsMessage();
	SetOfPointsOptions getOptions() const;
	bool getUserAxes() const;
	bool getCreateMeasures() const;
	bool getFromTop() const;
	bool getHorizontal() const;
	double getStep() const;
	double getThreshold() const;
	MessageType getType() const;
	virtual IMessage* copy() const;
	
private:
	SetOfPointsOptions m_options;
	bool m_userAxes, m_createMeasures, m_fromTop, m_horizontal;
	double m_step, m_threshold;
};
#endif //! PLANE_MESSAGE_H

