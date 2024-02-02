#ifndef POINT_CLOUD_OBJECT_CREATION_PARAMETERS_MESSAGE_H_
#define POINT_CLOUD_OBJECT_CREATION_PARAMETERS_MESSAGE_H_

#include <filesystem>
#include "io/exports/ExportParameters.hpp"
#include "controller/messages/IMessage.h"

class PointCloudObjectCreationParametersMessage : public IMessage
{
public:
    PointCloudObjectCreationParametersMessage(const PointCloudObjectParameters& parameters);
    ~PointCloudObjectCreationParametersMessage();
    MessageType	getType() const;
    IMessage* copy() const;

public:
    const PointCloudObjectParameters m_parameters;
};

#endif //! #pragma once
