#include "controller/messages/SmoothPointsMessage.h"

SmoothPointsMessage::SmoothPointsMessage(const SmoothPointsParameters& parameters)
    : params(parameters)
{
}

IMessage::MessageType SmoothPointsMessage::getType() const
{
    return IMessage::MessageType::SMOOTH_POINTS_PARAMETERS;
}

IMessage* SmoothPointsMessage::copy() const
{
    return new SmoothPointsMessage(*this);
}
