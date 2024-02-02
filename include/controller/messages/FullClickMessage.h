#ifndef FULL_CLICK_MESSAGE_H
#define FULL_CLICK_MESSAGE_H

#include "controller/messages/IMessage.h"
#include "models/3d/ClickInfo.h"

class FullClickMessage : public IMessage
{
public:
    FullClickMessage(const ClickInfo&);
	~FullClickMessage();
	MessageType getType() const;
	IMessage* copy() const;
	const glm::dvec3& getRay() const;
	const glm::dvec3& getRayOrigin() const;
	const uint32_t& getHeight() const;
	const double& getHeightAt1m() const;
	const double& getFov() const;
	SafePtr<CameraNode> getPanel() const;
	const tls::ScanGuid& getPanoramic() const;

public:
    const ClickInfo m_clickInfo;
};

#endif