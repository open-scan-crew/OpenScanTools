#ifndef CONTEXT_SMOOTH_POINTS_H
#define CONTEXT_SMOOTH_POINTS_H

#include "controller/functionSystem/AContext.h"
#include "controller/messages/SmoothPointsMessage.h"
#include "tls_def.h"

#include <filesystem>

class ContextSmoothPoints : public AContext
{
public:
    ContextSmoothPoints(const ContextId& id);
    ~ContextSmoothPoints();

    ContextState start(Controller& controller) override;
    ContextState launch(Controller& controller) override;
    ContextState feedMessage(IMessage* message, Controller& controller) override;

    bool canAutoRelaunch() const override;
    ContextType getType() const override;

private:
    bool prepareOutputDirectory(Controller& controller, const std::filesystem::path& folderPath);

private:
    tls::ScanGuid m_panoramic;
    SmoothPointsParameters m_params;
    bool m_hasParameters = false;
};

#endif
