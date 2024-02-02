#ifndef SIMPLENUMBERMESSAGE_H_
#define SIMPLENUMBERMESSAGE_H_

#include "IMessage.h"
#include <stdint.h>
#include <vector>

class SimpleNumberMessage : public IMessage
{
public:
	SimpleNumberMessage(const uint64_t& value);
	~SimpleNumberMessage();
	MessageType getType() const;	
	IMessage* copy() const;

public:
	uint64_t m_returnedValue;
};


class DoubleVectorMessage : public IMessage
{
public:
	DoubleVectorMessage(const std::vector<double>& value);
	~DoubleVectorMessage();
	MessageType getType() const;
	IMessage* copy() const;

public:
	std::vector<double> m_returnedValue;
};

#endif
