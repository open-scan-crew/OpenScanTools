#ifndef MANIPULATE_MESSAGE_H_
#define MANIPULATE_MESSAGE_H_

#include "controller/messages/IMessage.h"

enum ZMovement
{
	Default,
	Lock,
	Along
};

class ManipulateMessage : public IMessage
{
public:
	ManipulateMessage(bool m_rotate, ZMovement m_zmove);
	~ManipulateMessage();
	MessageType getType() const;
	IMessage* copy() const;

public:
	bool m_rotate;
	ZMovement m_zmove = ZMovement::Default;
};

#endif //! CAMERA_MESSAGE_H_