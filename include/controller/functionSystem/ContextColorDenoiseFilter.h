#ifndef CONTEXT_COLOR_DENOISE_FILTER_H
#define CONTEXT_COLOR_DENOISE_FILTER_H

#include "controller/functionSystem/AContext.h"

#include "crossguid/guid.hpp"

#include <filesystem>
#include <unordered_map>
#include <vector>

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
    bool m_warningModal = false;
    int m_kNeighbors = 32;
    int m_strength = 60;
    int m_luminanceStrength = 30;
    double m_radiusFactor = 4.0;
    int m_iterations = 1;
    bool m_preserveLuminance = true;
    bool m_globalDenoise = false;
    std::filesystem::path m_outputFolder;
    bool m_openFolderAfterExport = false;
};

#endif
