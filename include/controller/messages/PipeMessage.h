#ifndef PIPE_MESSAGE_H
#define PIPE_MESSAGE_H

#include "controller/messages/IMessage.h"

class PipeConnetionMessage : public IMessage
{
public:
	PipeConnetionMessage(const double& RonDext, const bool& applyAngleConstraints, const bool& keepDiameter);
	~PipeConnetionMessage();
	const double& getRonDext() const;
	const bool& getAngleConstraints() const;
	const bool& getKeepDiameter() const;

	MessageType getType() const;
	virtual IMessage* copy() const;
private:
	const double& m_RonDext;
	const bool& m_angleConstraints;
	const bool& m_keepDiameter;
};

#endif //! PIPEMESSAGE_H_
