#ifndef DELETE_POINTS_MESSAGE_H
#define DELETE_POINTS_MESSAGE_H

#include "io/exports/ExportParameters.hpp"
#include "controller/messages/IMessage.h"

class DeletePointsMessage : public IMessage
{
public:
    DeletePointsMessage(ExportClippingFilter clippingSource);
    ~DeletePointsMessage() {};
    MessageType	getType() const override;
    IMessage* copy() const override;

public:
    const ExportClippingFilter clippingFilter;
};

#endif
