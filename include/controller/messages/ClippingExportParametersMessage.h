#ifndef CLIPPING_EXPORT_PARAMETERS_MESSAGE_H
#define CLIPPING_EXPORT_PARAMETERS_MESSAGE_H

#include "io/exports/ExportParameters.hpp"
#include "controller/messages/IMessage.h"

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
    ExportInitMessage(bool use_clippings, bool use_grids, bool export_scans, bool export_pcos, ObjectStatusFilter point_cloud_filter);
    ~ExportInitMessage() {};
    MessageType	getType() const override;
    IMessage* copy() const override;
public:
    const bool use_clippings_;
    const bool use_grids_;
    const bool export_scans_;
    const bool export_PCOs_;
    ObjectStatusFilter point_cloud_filter_;

};

#endif //! #pragma once
