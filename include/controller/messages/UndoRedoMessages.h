#ifndef UNDOREDOMESSAGE_H_
#define UNDOREDOMESSAGE_H_

#include "IMessage.h"

class UndoMessage : public IMessage
{
public:
	UndoMessage();
	~UndoMessage();
	MessageType getType() const;
	IMessage* copy() const;
};

class RedoMessage : public IMessage
{
public:
	RedoMessage();
	~RedoMessage();
	MessageType getType() const;
	IMessage* copy() const;
};

class ClearMessage : public IMessage
{
public:
	ClearMessage();
	~ClearMessage();
	MessageType getType() const;
	IMessage* copy() const;
};
#endif
