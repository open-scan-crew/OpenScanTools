#include "controller/messages/UndoRedoMessages.h"

UndoMessage::UndoMessage()
{}

UndoMessage::~UndoMessage()
{}

IMessage::MessageType UndoMessage::getType() const
{
	return MessageType::UNDO;
}

IMessage* UndoMessage::copy() const
{
	return new UndoMessage(*this);
}

RedoMessage::RedoMessage()
{}

RedoMessage::~RedoMessage()
{}

IMessage::MessageType RedoMessage::getType() const
{
	return MessageType::REDO;
}

IMessage* RedoMessage::copy() const
{
	return new RedoMessage(*this);
}

ClearMessage::ClearMessage()
{}

ClearMessage::~ClearMessage()
{}

IMessage::MessageType ClearMessage::getType() const
{
	return MessageType::CLEAR;
}

IMessage* ClearMessage::copy() const
{
	return new ClearMessage(*this);
}