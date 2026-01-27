#ifndef CONTEXT_COLOR_DENOISE_FILTER_H
#define CONTEXT_COLOR_DENOISE_FILTER_H

#include "controller/functionSystem/AContext.h"

#include <filesystem>

class Controller;
class IMessage;

class ContextColorDenoiseFilter : public AContext
{
public:
    ContextColorDenoiseFilter(const ContextId& id);
    ~ContextColorDenoiseFilter();

    ContextState start(Controller& controller) override;
    ContextState launch(Controller& controller) override;
    ContextState feedMessage(IMessage* message, Controller& controller) override;

    bool canAutoRelaunch() const override;
    ContextType getType() const override;

private:
    bool prepareOutputDirectory(Controller& controller, const std::filesystem::path& folderPath);

private:
    xg::Guid m_panoramic;
    int m_kNeighbors = 32;
    int m_strength = 60;
    double m_radiusFactor = 4.0;
    int m_iterations = 1;
    bool m_preserveLuminance = true;
    std::filesystem::path m_outputFolder;
    bool m_openFolderAfterExport = false;
};

#endif
