#include "controller/messages/PointCloudObjectCreationParametersMessage.h"

PointCloudObjectCreationParametersMessage::PointCloudObjectCreationParametersMessage(const PointCloudObjectParameters& parameters)
    : m_parameters(parameters)
{}

PointCloudObjectCreationParametersMessage::~PointCloudObjectCreationParametersMessage()
{}

IMessage::MessageType PointCloudObjectCreationParametersMessage::getType() const
{
    return (MessageType::PCO_CREATION_PARAMETERS);
}
IMessage* PointCloudObjectCreationParametersMessage::copy() const
{
    return new PointCloudObjectCreationParametersMessage(*this);
}