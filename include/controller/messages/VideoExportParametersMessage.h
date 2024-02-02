#ifndef VIDEO_EXPORT_PARAMETERS_MESSAGE_H
#define VIDEO_EXPORT_PARAMETERS_MESSAGE_H

#include "io/exports/ExportParameters.hpp"
#include "controller/messages/IMessage.h"


class VideoExportParametersMessage : public IMessage
{
public:
    VideoExportParametersMessage(const VideoExportParameters& parameters);
    ~VideoExportParametersMessage();
    MessageType	getType() const override;
    IMessage* copy() const override;

public:
    VideoExportParameters m_parameters;
};

#endif //! #pragma once
