#ifndef CLIPPING_EXPORT_PARAMETERS_MESSAGE_H
#define CLIPPING_EXPORT_PARAMETERS_MESSAGE_H

#include "io/exports/ExportParameters.hpp"
#include "controller/messages/IMessage.h"
#include "models/OpenScanToolsModelEssentials.h"

#include <unordered_set>

class APointCloudNode;

class ClippingExportParametersMessage : public IMessage
{
public:
    ClippingExportParametersMessage(const ClippingExportParameters& parameters);
    ~ClippingExportParametersMessage();
    MessageType	getType() const override;
    IMessage* copy() const override;

public:
    const ClippingExportParameters m_parameters;
};

class ExportInitMessage : public IMessage
{
public:
    ExportInitMessage(bool useClippings, bool m_useGrids, bool exportScans, bool exportPCOs);
    ExportInitMessage(const std::unordered_set<SafePtr<APointCloudNode>>& exportPcs);
    ~ExportInitMessage() {};
    MessageType	getType() const override;
    IMessage* copy() const override;
public:
    const bool m_useClippings;
    const bool m_useGrids;
    const bool m_exportScans;
    const bool m_exportPCOs;
    const std::unordered_set<SafePtr<APointCloudNode>> m_exportPcs;

};

#endif //! #pragma once
