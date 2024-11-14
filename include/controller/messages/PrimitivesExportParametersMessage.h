#ifndef PRIMITIVES_EXPORT_PARAMETERS_MESSAGE_H
#define PRIMITIVES_EXPORT_PARAMETERS_MESSAGE_H

#include "io/exports/ExportParameters.hpp"
#include "controller/messages/IMessage.h"


class PrimitivesExportParametersMessage : public IMessage
{
public:
	PrimitivesExportParametersMessage(const PrimitivesExportParameters& parameters);
    ~PrimitivesExportParametersMessage();
    MessageType	getType() const override;
    IMessage* copy() const override;

public:
    const PrimitivesExportParameters m_parameters;
};

#endif //! #pragma once
