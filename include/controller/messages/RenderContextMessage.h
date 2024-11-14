#ifndef RENDERCONTEXT_MESSAGE_H_
#define RENDERCONTEXT_MESSAGE_H_

#include "controller/messages/IMessage.h"
#include "models/3d/DisplayParameters.h"

class RenderContextMessage : public IMessage
{
public:
	RenderContextMessage(const DisplayParameters& parameters);
	~RenderContextMessage() {};
	IMessage::MessageType getType() const;	
	IMessage* copy() const;

public:
	DisplayParameters m_parameters;
};
#endif // !RENDERCONTEXT_MESSAGE_H_