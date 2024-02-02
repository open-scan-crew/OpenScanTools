#include "controller/messages/FullClickMessage.h"

FullClickMessage::FullClickMessage(const ClickInfo& clickInfo)
    : m_clickInfo(clickInfo)
{}

FullClickMessage::~FullClickMessage()
{}

IMessage::MessageType FullClickMessage::getType() const
{
	return IMessage::MessageType::FULL_CLICK;
}

IMessage* FullClickMessage::copy() const
{
	return new FullClickMessage(*this);
}

const glm::dvec3& FullClickMessage::getRay() const
{
	return m_clickInfo.ray;
}

const glm::dvec3& FullClickMessage::getRayOrigin() const
{
	return m_clickInfo.rayOrigin;
}

const uint32_t& FullClickMessage::getHeight() const
{
	return m_clickInfo.height;
}

const double& FullClickMessage::getHeightAt1m() const
{
	return m_clickInfo.heightAt1m;
}

const double& FullClickMessage::getFov() const
{
	return m_clickInfo.fov;
}

SafePtr<CameraNode> FullClickMessage::getPanel() const
{
	return m_clickInfo.viewport;
}

const tls::ScanGuid& FullClickMessage::getPanoramic() const
{
	return m_clickInfo.panoramic;
}