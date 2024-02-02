#ifndef MODALMESSAGE_H_
#define MODALMESSAGE_H_

#include "IMessage.h"
#include <stdint.h>

class ModalMessage : public IMessage
{
public:
	ModalMessage(const uint32_t& value);
	~ModalMessage();
	MessageType getType() const;	
	IMessage* copy() const;

public:
	uint32_t m_returnedValue;
};

#endif
