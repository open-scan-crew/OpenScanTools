#ifndef CONTEXT_COLOR_BALANCE_H
#define CONTEXT_COLOR_BALANCE_H

#include "controller/functionSystem/AContext.h"
#include "controller/messages/ColorBalanceMessage.h"
#include "pointCloudEngine/ColorBalanceSettings.h"

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
    ColorBalanceSettings buildSettings() const;

    ColorBalanceMode m_mode = ColorBalanceMode::Separate;
    ColorBalanceStrength m_strength = ColorBalanceStrength::Mid;
    bool m_applyOnRGBAndIntensity = false;
    std::filesystem::path m_outputFolder;
    bool m_openFolderAfterExport = true;
    bool m_warningModal = false;
    xg::Guid m_panoramic;
};

#endif
