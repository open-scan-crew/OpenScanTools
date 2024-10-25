#ifndef CONTEXT_DELETE_POINTS_H
#define CONTEXT_DELETE_POINTS_H

#include "controller/functionSystem/AContext.h"
#include "io/exports/ExportParameters.hpp"

class ContextDeletePoints : public AContext
{
public:
    ContextDeletePoints(const ContextId& id);
    ~ContextDeletePoints();
    ContextState start(Controller& controller) override;
    ContextState launch(Controller& controller) override;
    ContextState feedMessage(IMessage* message, Controller& controller) override;
    bool canAutoRelaunch() const;

    ContextType getType() const override;

private:
    bool prepareOutputDirectory(Controller& controller, const std::filesystem::path& folderPath);

private:
    ExportClippingFilter m_clippingFilter;
    tls::ScanGuid m_panoramic;
    ContextId m_saveContext;
    xg::Guid m_viewportId;

    bool m_warningModal;
};

#endif