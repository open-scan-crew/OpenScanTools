#ifndef CONTEXT_COLOR_BALANCE_FILTER_H
#define CONTEXT_COLOR_BALANCE_FILTER_H

#include "controller/functionSystem/AContext.h"
#include "controller/messages/ColorBalanceFilterMessage.h"
#include "tls_def.h"

#include <filesystem>

class Controller;

class ContextColorBalanceFilter : public AContext
{
public:
    ContextColorBalanceFilter(const ContextId& id);
    ~ContextColorBalanceFilter();

    ContextState start(Controller& controller) override;
    ContextState launch(Controller& controller) override;
    ContextState feedMessage(IMessage* message, Controller& controller) override;
    bool canAutoRelaunch() const override;
    ContextType getType() const override;

private:
    bool prepareOutputDirectory(Controller& controller, const std::filesystem::path& folderPath);

    tls::ScanGuid m_panoramic;
    int m_kMin = 24;
    int m_kMax = 40;
    double m_trimPercent = 18.0;
    double m_sharpnessBlend = 0.2;
    bool m_globalBalancing = false;
    bool m_applyOnIntensityAndRgb = true;
    std::filesystem::path m_outputFolder;
    bool m_openFolderAfterExport = false;
    bool m_warningModal = false;
};

#endif
