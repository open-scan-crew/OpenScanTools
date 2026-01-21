#ifndef SMOOTH_POINTS_MESSAGE_H
#define SMOOTH_POINTS_MESSAGE_H

#include "controller/messages/IMessage.h"
#include "pointCloudEngine/SmoothPointsParameters.h"

class SmoothPointsMessage : public IMessage
{
public:
    explicit SmoothPointsMessage(const SmoothPointsParameters& parameters);
    ~SmoothPointsMessage() = default;
    MessageType getType() const override;
    IMessage* copy() const override;

public:
    const SmoothPointsParameters params;
};

#endif
