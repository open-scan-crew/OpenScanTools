#include "controller/messages/CameraMessage.h"

CameraMessage::CameraMessage(SafePtr<CameraNode> cameraNode)
	: m_cameraNode(cameraNode)
{}

CameraMessage::~CameraMessage()
{}

IMessage::MessageType CameraMessage::getType() const
{
	return  IMessage::MessageType::CAMERA;
}

IMessage* CameraMessage::copy() const
{
	return new CameraMessage(*this);
}