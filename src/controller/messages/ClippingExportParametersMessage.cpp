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

ExportInitMessage::ExportInitMessage(bool useClippings, bool useGrids, bool exportScans, bool exportPCOs)
    : m_useClippings(useClippings)
    , m_useGrids(useGrids)
    , m_exportScans(exportScans)
    , m_exportPCOs(exportPCOs)
    , m_exportPcs({})
{}

ExportInitMessage::ExportInitMessage(const std::unordered_set<SafePtr<APointCloudNode>>& exportPcs)
    : m_useClippings(false)
    , m_useGrids(false)
    , m_exportScans(false)
    , m_exportPCOs(false)
    , m_exportPcs(exportPcs)
{}

IMessage::MessageType ExportInitMessage::getType() const
{
    return (MessageType::EXPORT_INIT);
}

IMessage* ExportInitMessage::copy() const
{
    return new ExportInitMessage(*this);
}