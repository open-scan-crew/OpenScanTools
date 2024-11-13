#include "controller/messages/ClippingExportParametersMessage.h"

ClippingExportParametersMessage::ClippingExportParametersMessage(const ClippingExportParameters& parameters)
    : m_parameters(parameters)
{}

ClippingExportParametersMessage::~ClippingExportParametersMessage()
{}

IMessage::MessageType ClippingExportParametersMessage::getType() const
{
    return (MessageType::CLIPPING_EXPORT_PARAMETERS);
}
IMessage* ClippingExportParametersMessage::copy() const
{
    return new ClippingExportParametersMessage(*this);
}

ExportInitMessage::ExportInitMessage(bool use_clippings, bool use_grids, bool export_scans, bool export_PCOs, ObjectStatusFilter point_cloud_filter)
    : use_clippings_(use_clippings)
    , use_grids_(use_grids)
    , export_scans_(export_scans)
    , export_PCOs_(export_PCOs)
    , point_cloud_filter_(point_cloud_filter)
{}

IMessage::MessageType ExportInitMessage::getType() const
{
    return (MessageType::EXPORT_INIT);
}

IMessage* ExportInitMessage::copy() const
{
    return new ExportInitMessage(*this);
}