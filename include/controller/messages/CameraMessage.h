#ifndef CAMERA_MESSAGE_H_
#define CAMERA_MESSAGE_H_

#include "controller/messages/IMessage.h"
#include "models/OpenScanToolsModelEssentials.h"

class CameraNode;

class CameraMessage : public IMessage
{
public:
	CameraMessage(SafePtr<CameraNode> cameraNode);
	~CameraMessage();
	MessageType getType() const;
	IMessage* copy() const;

public:
	SafePtr<CameraNode> m_cameraNode;
};

#endif //! CAMERA_MESSAGE_H_