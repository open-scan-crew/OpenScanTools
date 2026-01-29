#include "controller/functionSystem/ContextColorBalance.h"

#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/messages/ColorBalanceMessage.h"
#include "controller/messages/ModalMessage.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataIO.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"
#include "gui/texts/ExportTexts.hpp"
#include "gui/texts/SplashScreenTexts.hpp"
#include "io/exports/IScanFileWriter.h"
#include "models/graph/GraphManager.h"
#include "models/graph/PointCloudNode.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "utils/Logger.h"

#include <algorithm>
#include <filesystem>

// Note (Aurélien) QT::StandardButtons enum values in qmessagebox.h
#define Yes 0x00004000
#define No 0x00010000

ContextColorBalance::ContextColorBalance(const ContextId& id)
    : AContext(id)
    , m_panoramic(xg::Guid())
{
    m_state = ContextState::waiting_for_input;
}

ContextColorBalance::~ContextColorBalance()
{}

ContextState ContextColorBalance::start(Controller& controller)
{
    GraphManager& graphManager = controller.getGraphManager();
    std::unordered_set<SafePtr<PointCloudNode>> scans = graphManager.getVisibleScans(m_panoramic);

    if (scans.size() < 2)
    {
        FUNCLOG << "Not enough scans for color balance" << LOGENDL;
        controller.updateInfo(new GuiDataWarning(TEXT_COLOR_BALANCE_MIN_SCANS));
        return (m_state = ContextState::abort);
    }

    bool enableRgbIntensity = true;
    for (const SafePtr<PointCloudNode>& scan : scans)
    {
        ReadPtr<PointCloudNode> rScan = scan.cget();
        if (!rScan)
            continue;
        if (!rScan->getRGBAvailable() || !rScan->getIntensityAvailable())
        {
            enableRgbIntensity = false;
            break;
        }
    }

    controller.updateInfo(new GuiDataColorBalanceDialogDisplay(enableRgbIntensity));
    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextColorBalance::feedMessage(IMessage* message, Controller& controller)
{
    switch (message->getType())
    {
    case IMessage::MessageType::MODAL:
    {
        ModalMessage* modal = static_cast<ModalMessage*>(message);
        if (modal->m_returnedValue == Yes)
            m_state = ContextState::ready_for_using;
        else
            m_state = ContextState::abort;
    }
    break;
    case IMessage::MessageType::COLOR_BALANCE_PARAMETERS:
    {
        auto decodedMsg = static_cast<ColorBalanceMessage*>(message);
        m_settings = decodedMsg->settings;
        m_outputFileType = decodedMsg->outputFileType;
        m_outputFolder = decodedMsg->outputFolder;
        m_openFolderAfterExport = decodedMsg->openFolderAfterExport;

        m_warningModal = true;
        controller.updateInfo(new GuiDataModal(Yes | No, TEXT_COLOR_BALANCE_QUESTION));
        break;
    }
    break;
    default:
        break;
    }

    return (m_state);
}

ContextState ContextColorBalance::launch(Controller& controller)
{
    GraphManager& graphManager = controller.getGraphManager();

    if (!prepareOutputDirectory(controller, m_outputFolder))
    {
        m_state = ContextState::abort;
        return m_state;
    }

    TlStreamLock streamLock;
    std::unordered_set<SafePtr<PointCloudNode>> scans = graphManager.getVisibleScans(m_panoramic);

    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString()));
    const uint64_t totalScans = scans.size();
    const uint64_t totalProgressSteps = totalScans * 100;
    controller.updateInfo(new GuiDataProcessingSplashScreenStart(totalProgressSteps, TEXT_EXPORT_COLOR_BALANCE_TITLE_PROGESS, TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(0).arg(totalScans)));

    ClippingAssembly clippingAssembly;
    graphManager.getClippingAssembly(clippingAssembly, true, false);

    std::vector<tls::ScanGuid> scanGuids;
    scanGuids.reserve(scans.size());
    for (const SafePtr<PointCloudNode>& scan : scans)
    {
        ReadPtr<PointCloudNode> rScan = scan.cget();
        if (!rScan)
            continue;
        scanGuids.push_back(rScan->getScanGuid());
    }

    uint64_t scan_count = 0;
    uint64_t total_adjusted_points = 0;
    auto updateProgress = [&](uint64_t scansDone, int percent, uint64_t progressValue)
    {
        QString state = QString("%1 - %2%")
                            .arg(TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(scansDone).arg(totalScans))
                            .arg(percent);
        controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(state, progressValue));
    };
    auto makeProgressCallback = [&](uint64_t scansDone, int basePercent, int spanPercent)
    {
        return [scansDone, basePercent, spanPercent, &updateProgress](size_t processed, size_t total)
        {
            if (total == 0)
                return;
            int percent = basePercent + static_cast<int>((processed * spanPercent) / total);
            percent = std::clamp(percent, basePercent, basePercent + spanPercent - 1);
            if (percent >= 100)
                percent = 99;
            uint64_t progressValue = scansDone * 100 + static_cast<uint64_t>(percent);
            updateProgress(scansDone, percent, progressValue);
        };
    };

    for (const SafePtr<PointCloudNode>& scan : scans)
    {
        WritePtr<PointCloudNode> wScan = scan.get();
        if (!wScan)
            continue;

        const ClippingAssembly* clippingToUse = &clippingAssembly;
        ClippingAssembly resolvedAssembly;
        if (clippingAssembly.hasPhaseClipping())
        {
            resolvedAssembly = clippingAssembly.resolveByPhase(wScan->getPhase());
            clippingToUse = &resolvedAssembly;
        }

        uint64_t adjusted_points = 0;
        std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
        tls::ScanGuid old_guid = wScan->getScanGuid();

        IScanFileWriter* scan_writer = nullptr;
        std::wstring log;
        std::wstring outputName = wScan->getName() + L"_CB";
        if (!getScanFileWriter(m_outputFolder, outputName, m_outputFileType, log, &scan_writer) || scan_writer == nullptr)
            continue;

        tls::ScanHeader header;
        TlScanOverseer::getInstance().getScanHeader(old_guid, header);
        header.guid = xg::newGuid();
        scan_writer->appendPointCloud(header, wScan->getTransformation());

        ColorBalanceSettings scanSettings = m_settings;
        if (wScan->getRGBAvailable())
            scanSettings.applyOnRgb = true;
        else
            scanSettings.applyOnRgb = false;

        if (wScan->getIntensityAvailable())
        {
            if (wScan->getRGBAvailable())
                scanSettings.applyOnIntensity = m_settings.applyOnIntensity;
            else
                scanSettings.applyOnIntensity = true;
        }
        else
        {
            scanSettings.applyOnIntensity = false;
        }

        std::vector<tls::ScanGuid> otherGuids;
        otherGuids.reserve(scanGuids.size());
        for (const tls::ScanGuid& guid : scanGuids)
        {
            if (guid != old_guid)
                otherGuids.push_back(guid);
        }

        auto progressCallback = makeProgressCallback(scan_count, 0, 100);
        bool res = TlScanOverseer::getInstance().colorBalanceAndWrite(old_guid, otherGuids, (TransformationModule)*&wScan, *clippingToUse, scanSettings, scan_writer, adjusted_points, progressCallback);
        res &= scan_writer->finalizePointCloud();
        delete scan_writer;
        (void)res;

        total_adjusted_points += adjusted_points;

        scan_count++;
        float seconds = std::chrono::duration<float, std::ratio<1>>(std::chrono::steady_clock::now() - startTime).count();
        QString qScanName = QString::fromStdWString(wScan->getName());
        updateProgress(scan_count, 100, scan_count * 100);

        if (adjusted_points > 0)
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("%1 points adjusted in scan %2 in %3 seconds.").arg(adjusted_points).arg(qScanName).arg(seconds)));
        else
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Scan %1 not affected by color balance.").arg(qScanName)));
    }

    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Total points adjusted: %1").arg(total_adjusted_points)));
    controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));

    if (m_openFolderAfterExport)
        controller.updateInfo(new GuiDataOpenInExplorer(m_outputFolder));

    m_state = ContextState::done;
    return (m_state);
}

bool ContextColorBalance::canAutoRelaunch() const
{
    return false;
}

ContextType ContextColorBalance::getType() const
{
    return ContextType::colorBalance;
}

bool ContextColorBalance::prepareOutputDirectory(Controller& controller, const std::filesystem::path& folderPath)
{
    if (std::filesystem::is_directory(folderPath) == false)
    {
        try
        {
            if (std::filesystem::create_directory(folderPath) == false)
            {
                Logger::log(LoggerMode::IOLog) << "Error: the path '" << folderPath << "' is not a valid path for a folder." << Logger::endl;
                controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_INVALID_DIRECTORY));
                return false;
            }
        }
        catch (std::exception e)
        {
            Logger::log(LoggerMode::IOLog) << "Error: the path '" << folderPath << "' is not a valid path for a folder." << Logger::endl;
            controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_INVALID_DIRECTORY));
            return false;
        }
    }

    return true;
}
