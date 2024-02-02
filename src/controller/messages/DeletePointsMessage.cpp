#include "controller/messages/DeletePointsMessage.h"

DeletePointsMessage::DeletePointsMessage(ExportClippingFilter filter)
    : clippingFilter(filter)
{}

IMessage::MessageType DeletePointsMessage::getType() const
{
    return MessageType::DELETE_POINTS_PARAMETERS;
}

IMessage* DeletePointsMessage::copy() const
{
    return new DeletePointsMessage(clippingFilter);
}