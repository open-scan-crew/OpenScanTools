#include "controller/messages/VideoExportParametersMessage.h"

VideoExportParametersMessage::VideoExportParametersMessage(const VideoExportParameters& parameters)
    : m_parameters(parameters)
{}

VideoExportParametersMessage::~VideoExportParametersMessage()
{}

IMessage::MessageType VideoExportParametersMessage::getType() const
{
    return (MessageType::VIDEO_EXPORT_PARAMETERS);
}
IMessage* VideoExportParametersMessage::copy() const
{
    return new VideoExportParametersMessage(*this);
}