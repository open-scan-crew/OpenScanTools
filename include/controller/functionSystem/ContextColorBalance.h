#ifndef CONTEXT_COLOR_BALANCE_H
#define CONTEXT_COLOR_BALANCE_H

#include "controller/functionSystem/AContext.h"

#include "crossguid/guid.hpp"

#include <filesystem>

class ContextColorBalance : public AContext
{
public:
    ContextColorBalance(const ContextId& id);
    ~ContextColorBalance();

    ContextState start(Controller& controller) override;
    ContextState launch(Controller& controller) override;
    ContextState feedMessage(IMessage* message, Controller& controller) override;
    bool canAutoRelaunch() const override;

    ContextType getType() const override;

private:
    bool prepareOutputDirectory(Controller& controller, const std::filesystem::path& folderPath);

    xg::Guid m_panoramic;

    int m_photometricDepth = 7;
    int m_nMin = 150;
    int m_nGood = 1000;
    double m_beta = 0.5;
    int m_smoothingLevels = 4;
    bool m_applyIntensityWhenAvailable = true;
    std::filesystem::path m_outputFolder;
    bool m_openFolderAfterExport = false;
};

#endif
