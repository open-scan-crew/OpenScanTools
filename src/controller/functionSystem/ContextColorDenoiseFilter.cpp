#include "controller/functionSystem/ContextColorDenoiseFilter.h"

#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/messages/ColorDenoiseFilterMessage.h"
#include "controller/messages/ModalMessage.h"
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
#include <chrono>
#include <filesystem>

// Note (Aurélien) QT::StandardButtons enum values in qmessagebox.h
#define Yes 0x00004000
#define No 0x00010000

ContextColorDenoiseFilter::ContextColorDenoiseFilter(const ContextId& id)
    : AContext(id)
    , m_panoramic(xg::Guid())
{
    m_state = ContextState::waiting_for_input;
}

ContextColorDenoiseFilter::~ContextColorDenoiseFilter()
{}

ContextState ContextColorDenoiseFilter::start(Controller& controller)
{
    GraphManager& graphManager = controller.getGraphManager();

    if (graphManager.getVisibleScans(m_panoramic).empty())
    {
        FUNCLOG << "No Scans visibles to denoise" << LOGENDL;
        controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_NO_SCAN_SELECTED));
        return (m_state = ContextState::abort);
    }

    controller.updateInfo(new GuiDataColorDenoiseFilterDialogDisplay());

    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextColorDenoiseFilter::feedMessage(IMessage* message, Controller& controller)
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
    case IMessage::MessageType::COLOR_DENOISE_FILTER_PARAMETERS:
    {
        auto decodedMsg = static_cast<ColorDenoiseFilterMessage*>(message);
        m_kNeighbors = decodedMsg->kNeighbors;
        m_strength = decodedMsg->strength;
        m_radiusFactor = decodedMsg->radiusFactor;
        m_iterations = decodedMsg->iterations;
        m_preserveLuminance = decodedMsg->preserveLuminance;
        m_outputFolder = decodedMsg->outputFolder;
        m_openFolderAfterExport = decodedMsg->openFolderAfterExport;

        m_warningModal = true;
        controller.updateInfo(new GuiDataModal(Yes | No, TEXT_COLOR_DENOISE_FILTER_QUESTION));
        break;
    }
    break;
    default:
        break;
    }

    return (m_state);
}

ContextState ContextColorDenoiseFilter::launch(Controller& controller)
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
    controller.updateInfo(new GuiDataProcessingSplashScreenStart(totalProgressSteps, TEXT_EXPORT_COLOR_DENOISE_TITLE_PROGESS, TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(0).arg(totalScans)));

    ClippingAssembly clippingAssembly;
    graphManager.getClippingAssembly(clippingAssembly, true, false);

    uint64_t scanCount = 0;
    uint64_t totalProcessedPoints = 0;
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

        if (!wScan->getRGBAvailable() && !wScan->getIntensityAvailable())
        {
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Scan %1 has no RGB or intensity data. Skipped.").arg(QString::fromStdWString(wScan->getName()))));
            scanCount++;
            updateProgress(scanCount, 100, scanCount * 100);
            continue;
        }

        const ClippingAssembly* clippingToUse = &clippingAssembly;
        ClippingAssembly resolvedAssembly;
        if (clippingAssembly.hasPhaseClipping())
        {
            resolvedAssembly = clippingAssembly.resolveByPhase(wScan->getPhase());
            clippingToUse = &resolvedAssembly;
        }

        std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
        tls::ScanGuid oldGuid = wScan->getScanGuid();

        TlsFileWriter* tlsWriter = nullptr;
        std::wstring log;
        std::wstring outputName = wScan->getName() + L"_DENOISE";
        TlsFileWriter::getWriter(m_outputFolder, outputName, log, (IScanFileWriter**)&tlsWriter);
        if (tlsWriter == nullptr)
            continue;
        tls::ScanHeader header;
        TlScanOverseer::getInstance().getScanHeader(oldGuid, header);
        header.guid = xg::newGuid();
        tlsWriter->appendPointCloud(header, wScan->getTransformation());

        auto filterProgress = makeProgressCallback(scanCount, 0, 100);
        uint64_t processedPoints = 0;
        bool res = TlScanOverseer::getInstance().denoiseColorsAndWrite(oldGuid, (TransformationModule)*&wScan, *clippingToUse, m_kNeighbors, m_strength, m_radiusFactor, m_iterations, m_preserveLuminance, tlsWriter, processedPoints, filterProgress);
        res &= tlsWriter->finalizePointCloud();
        delete tlsWriter;

        totalProcessedPoints += processedPoints;

        scanCount++;
        float seconds = std::chrono::duration<float, std::ratio<1>>(std::chrono::steady_clock::now() - startTime).count();
        QString qScanName = QString::fromStdWString(wScan->getName());
        updateProgress(scanCount, 100, scanCount * 100);

        if (res)
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Scan %1 denoised (%2 points) in %3 seconds.").arg(qScanName).arg(processedPoints).arg(seconds)));
        else
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Scan %1 denoise failed after %2 seconds.").arg(qScanName).arg(seconds)));
    }

    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Total points processed: %1").arg(totalProcessedPoints)));
    controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));

    if (m_openFolderAfterExport)
        controller.updateInfo(new GuiDataOpenInExplorer(m_outputFolder));

    m_state = ContextState::done;
    return (m_state);
}

bool ContextColorDenoiseFilter::canAutoRelaunch() const
{
    return false;
}

ContextType ContextColorDenoiseFilter::getType() const
{
    return ContextType::colorDenoiseFilter;
}

bool ContextColorDenoiseFilter::prepareOutputDirectory(Controller& controller, const std::filesystem::path& folderPath)
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
