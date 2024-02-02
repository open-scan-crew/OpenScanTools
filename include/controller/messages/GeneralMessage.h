#ifndef GENERAL_MESSAGE_H_
#define GENERAL_MESSAGE_H_

#include "controller/messages/IMessage.h"

enum class GeneralInfo
{
	NONE,
	ANIMATIONEND,
	IMAGEEND,
	MAXENUM
};

class GeneralMessage : public IMessage
{
public:
	GeneralMessage(GeneralInfo info);
	~GeneralMessage();
	MessageType getType() const;
	IMessage* copy() const;

public:
	GeneralInfo m_info;
};

#endif //! CAMERA_MESSAGE_H_