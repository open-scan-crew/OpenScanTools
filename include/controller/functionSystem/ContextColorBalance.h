#ifndef CONTEXT_COLOR_BALANCE_H
#define CONTEXT_COLOR_BALANCE_H

#include "controller/functionSystem/AContext.h"
#include "io/FileUtils.h"
#include "pointCloudEngine/ColorBalanceSettings.h"

#include "crossguid/guid.hpp"

#include <filesystem>

class Controller;

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

    ColorBalanceSettings m_settings;
    FileType m_outputFileType = FileType::TLS;
    std::filesystem::path m_outputFolder;
    bool m_openFolderAfterExport = false;
    bool m_warningModal = false;
    xg::Guid m_panoramic;
};

#endif
