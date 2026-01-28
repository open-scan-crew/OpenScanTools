#include "controller/functionSystem/ContextColorBalanceFilter.h"

#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/messages/ModalMessage.h"
#include "controller/messages/ColorBalanceFilterMessage.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataIO.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"
#include "gui/texts/ExportTexts.hpp"
#include "gui/texts/SplashScreenTexts.hpp"
#include "io/exports/TlsFileWriter.h"
#include "models/graph/GraphManager.h"
#include "models/graph/PointCloudNode.h"
#include "pointCloudEngine/PCE_core.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "utils/Logger.h"

#include <algorithm>
#include <filesystem>
#include <functional>

// Note (Aurélien) QT::StandardButtons enum values in qmessagebox.h
#define Yes 0x00004000
#define No 0x00010000

ContextColorBalanceFilter::ContextColorBalanceFilter(const ContextId& id)
    : AContext(id)
    , m_panoramic(xg::Guid())
{
    m_state = ContextState::waiting_for_input;
}

ContextColorBalanceFilter::~ContextColorBalanceFilter()
{}

ContextState ContextColorBalanceFilter::start(Controller& controller)
{
    GraphManager& graphManager = controller.getGraphManager();
    std::unordered_set<SafePtr<PointCloudNode>> scans = graphManager.getVisibleScans(m_panoramic);
    if (scans.empty())
    {
        FUNCLOG << "No Scans visibles to balance" << LOGENDL;
        controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_NO_SCAN_SELECTED));
        return (m_state = ContextState::abort);
    }

    bool rgbAvailable = false;
    bool intensityAvailable = false;
    bool rgbAndIntensityAvailable = false;
    for (const SafePtr<PointCloudNode>& scan : scans)
    {
        ReadPtr<PointCloudNode> rScan = scan.cget();
        if (!rScan)
            continue;

        bool scanRgb = rScan->getRGBAvailable();
        bool scanIntensity = rScan->getIntensityAvailable();
        rgbAvailable |= scanRgb;
        intensityAvailable |= scanIntensity;
        if (scanRgb && scanIntensity)
            rgbAndIntensityAvailable = true;
    }

    controller.updateInfo(new GuiDataColorBalanceFilterDialogDisplay(rgbAvailable, intensityAvailable, rgbAndIntensityAvailable));
    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextColorBalanceFilter::feedMessage(IMessage* message, Controller& controller)
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
    case IMessage::MessageType::COLOR_BALANCE_FILTER_PARAMETERS:
    {
        auto decodedMsg = static_cast<ColorBalanceFilterMessage*>(message);
        m_kMin = decodedMsg->kMin;
        m_kMax = decodedMsg->kMax;
        m_trimPercent = decodedMsg->trimPercent;
        m_globalBalancing = decodedMsg->mode == ColorBalanceMode::Global;
        m_applyOnIntensityAndRgb = decodedMsg->applyOnIntensityAndRgb;
        m_outputFolder = decodedMsg->outputFolder;
        m_openFolderAfterExport = decodedMsg->openFolderAfterExport;

        m_warningModal = true;
        controller.updateInfo(new GuiDataModal(Yes | No, TEXT_COLOR_BALANCE_FILTER_QUESTION));
    }
    break;
    default:
        break;
    }

    return (m_state);
}

ContextState ContextColorBalanceFilter::launch(Controller& controller)
{
    GraphManager& graphManager = controller.getGraphManager();

    if (!prepareOutputDirectory(controller, m_outputFolder))
    {
        m_state = ContextState::abort;
        return m_state;
    }

    TlStreamLock streamLock;
    std::unordered_set<SafePtr<PointCloudNode>> scans = graphManager.getVisibleScans(m_panoramic);

    if (m_globalBalancing && scans.size() < 2)
    {
        controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_COLOR_BALANCE_NEEDS_MULTIPLE_SCANS));
        m_state = ContextState::abort;
        return m_state;
    }

    if (m_globalBalancing)
    {
        TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(m_panoramic, true, true));
    }

    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString()));
    const uint64_t totalScans = scans.size();
    const uint64_t totalProgressSteps = totalScans * 100;
    controller.updateInfo(new GuiDataProcessingSplashScreenStart(totalProgressSteps, TEXT_EXPORT_COLOR_BALANCE_TITLE_PROGRESS, TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(0).arg(totalScans)));

    ClippingAssembly clippingAssembly;
    graphManager.getClippingAssembly(clippingAssembly, true, false);

    uint64_t scanCount = 0;
    uint64_t totalModifiedPoints = 0;

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

        uint64_t modifiedPointCount = 0;
        std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
        tls::ScanGuid old_guid = wScan->getScanGuid();

        TlsFileWriter* tls_writer = nullptr;
        std::wstring log;
        std::wstring outputName = wScan->getName() + L"_CB";
        TlsFileWriter::getWriter(m_outputFolder, outputName, log, (IScanFileWriter**)&tls_writer);
        if (tls_writer == nullptr)
            continue;

        tls::ScanHeader header;
        TlScanOverseer::getInstance().getScanHeader(old_guid, header);
        header.guid = xg::newGuid();
        tls_writer->appendPointCloud(header, wScan->getTransformation());

        bool hasRgb = wScan->getRGBAvailable();
        bool hasIntensity = wScan->getIntensityAvailable();
        bool applyOnRgb = hasRgb;
        bool applyOnIntensity = false;
        if (hasIntensity && hasRgb)
            applyOnIntensity = m_applyOnIntensityAndRgb;
        else if (hasIntensity)
            applyOnIntensity = true;

        std::function<void(const GeometricBox&, std::vector<PointXYZIRGB>&)> externalProvider;
        if (m_globalBalancing)
        {
            externalProvider = [clippingToUse, old_guid](const GeometricBox& box, std::vector<PointXYZIRGB>& points)
            {
                TlScanOverseer::getInstance().collectPointsInGeometricBox(box, *clippingToUse, old_guid, points);
            };
        }

        auto progressCallback = makeProgressCallback(scanCount, 0, 100);
        bool res = TlScanOverseer::getInstance().balanceColorsAndWrite(old_guid, (TransformationModule)*&wScan, *clippingToUse, m_kMin, m_kMax, m_trimPercent, applyOnIntensity, applyOnRgb, externalProvider, tls_writer, modifiedPointCount, progressCallback);
        res &= tls_writer->finalizePointCloud();
        delete tls_writer;

        totalModifiedPoints += modifiedPointCount;

        scanCount++;
        float seconds = std::chrono::duration<float, std::ratio<1>>(std::chrono::steady_clock::now() - startTime).count();
        QString qScanName = QString::fromStdWString(wScan->getName());
        updateProgress(scanCount, 100, scanCount * 100);

        if (modifiedPointCount > 0)
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("%1 points updated in scan %2 in %3 seconds.").arg(modifiedPointCount).arg(qScanName).arg(seconds)));
        else
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Scan %1 not affected by color balance.").arg(qScanName)));
    }

    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Total points updated: %1").arg(totalModifiedPoints)));
    controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));

    if (m_openFolderAfterExport)
        controller.updateInfo(new GuiDataOpenInExplorer(m_outputFolder));

    m_state = ContextState::done;
    return (m_state);
}

bool ContextColorBalanceFilter::canAutoRelaunch() const
{
    return false;
}

ContextType ContextColorBalanceFilter::getType() const
{
    return ContextType::colorBalanceFilter;
}

bool ContextColorBalanceFilter::prepareOutputDirectory(Controller& controller, const std::filesystem::path& folderPath)
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
