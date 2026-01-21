#ifndef CONTEXT_SMOOTH_POINT_CLOUD_H
#define CONTEXT_SMOOTH_POINT_CLOUD_H

#include "controller/functionSystem/AContext.h"
#include "pointCloudEngine/PointCloudSmoothingEngine.h"

#include "tls_def.h"

#include <filesystem>

class Controller;

class ContextSmoothPointCloud : public AContext
{
public:
    ContextSmoothPointCloud(const ContextId& id);
    ~ContextSmoothPointCloud();

    ContextState start(Controller& controller) override;
    ContextState launch(Controller& controller) override;
    ContextState feedMessage(IMessage* message, Controller& controller) override;
    bool canAutoRelaunch() const override;
    ContextType getType() const override;

private:
    bool prepareOutputDirectory(Controller& controller, const std::filesystem::path& folderPath);

private:
    SmoothPointCloudParameters m_parameters;
    bool m_warningModal = false;
    ContextId m_saveContext;
    tls::ScanGuid m_panoramic;
};

#endif
