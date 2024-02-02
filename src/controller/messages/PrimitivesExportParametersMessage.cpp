#include "controller/messages/PrimitivesExportParametersMessage.h"

PrimitivesExportParametersMessage::PrimitivesExportParametersMessage(const PrimitivesExportParameters& parameters)
    : m_parameters(parameters)
{}

PrimitivesExportParametersMessage::~PrimitivesExportParametersMessage()
{}

IMessage::MessageType PrimitivesExportParametersMessage::getType() const
{
    return (MessageType::PRIMITIVES_EXPORT_PARAMETERS);
}
IMessage* PrimitivesExportParametersMessage::copy() const
{
    return new PrimitivesExportParametersMessage(*this);
}