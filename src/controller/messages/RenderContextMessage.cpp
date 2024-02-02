#include "controller/messages/RenderContextMessage.h"

RenderContextMessage::RenderContextMessage(const DisplayParameters& parameters)
	: m_parameters(parameters)
{}

IMessage::MessageType RenderContextMessage::getType() const
{
	return MessageType::RENDER_CONTEXT;
}

IMessage* RenderContextMessage::copy() const
{
	return new RenderContextMessage(*this);
}