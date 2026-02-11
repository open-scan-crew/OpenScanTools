#include "controller/functionSystem/ContextColorimetricFilterExport.h"

#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/messages/CameraMessage.h"
#include "controller/messages/ClippingExportParametersMessage.h"
#include "controller/messages/ModalMessage.h"

#include "gui/GuiData/GuiDataContextRequest.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataIO.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ExportTexts.hpp"
#include "gui/texts/SplashScreenTexts.hpp"

#include "io/exports/CSVWriter.h"
#include "io/exports/IScanFileWriter.h"
#include "models/graph/AClippingNode.h"
#include "models/graph/CameraNode.h"
#include "models/graph/GraphManager.h"
#include "models/graph/PointCloudNode.h"
#include "pointCloudEngine/PCE_core.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "utils/ColorimetricFilterUtils.h"
#include "utils/Logger.h"

#include <algorithm>
#include <chrono>
#include <ctime>

#define Yes 0x00004000
#define No 0x00010000

ContextColorimetricFilterExport::ContextColorimetricFilterExport(const ContextId& id)
    : AContext(id)
{
    m_state = ContextState::waiting_for_input;
}

ContextColorimetricFilterExport::~ContextColorimetricFilterExport()
{}

ContextState ContextColorimetricFilterExport::start(Controller& controller)
{
    controller.updateInfo(new GuiDataContextRequestActiveCamera(m_id));
    if (!controller.getContext().getIsCurrentProjectSaved())
        controller.updateInfo(new GuiDataModal(Yes | No, TEXT_EXPORT_SAVE_QUESTION));

    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextColorimetricFilterExport::feedMessage(IMessage* message, Controller& controller)
{
    GraphManager& graphManager = controller.getGraphManager();

    switch (message->getType())
    {
    case IMessage::MessageType::MODAL:
    {
        ModalMessage* modal = static_cast<ModalMessage*>(message);
        if (modal->m_returnedValue == Yes)
            controller.getFunctionManager().launchBackgroundFunction(controller, ContextType::saveProject, 0);
        break;
    }
    case IMessage::MessageType::CAMERA:
    {
        CameraMessage* cameraMsg = static_cast<CameraMessage*>(message);
        ReadPtr<CameraNode> rCamera = cameraMsg->m_camera.cget();
        if (!rCamera)
        {
            controller.updateInfo(new GuiDataWarning(TEXT_COLORIMETRIC_FILTER_ACTIVATE_FIRST));
            return (m_state = ContextState::abort);
        }

        const DisplayParameters& display = rCamera->getDisplayParameters();
        m_filterSettings = display.m_colorimetricFilter;
        m_renderMode = display.m_mode;
        m_filterReady = hasActiveColorimetricFilter();

        if (!m_filterReady)
        {
            controller.updateInfo(new GuiDataWarning(TEXT_COLORIMETRIC_FILTER_ACTIVATE_FIRST));
            return (m_state = ContextState::abort);
        }

        std::vector<tls::PointCloudInstance> pointClouds = graphManager.getPointCloudInstances(xg::Guid(), true, false, ObjectStatusFilter::VISIBLE);
        if (pointClouds.empty())
        {
            controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_NO_SCAN_SELECTED));
            return (m_state = ContextState::abort);
        }

        GuiDataExportParametersDisplay* guiData = new GuiDataExportParametersDisplay();
        guiData->pc_status_filter_ = ObjectStatusFilter::VISIBLE;
        guiData->pc_source_ = ExportPointCloudSource::SCAN;
        guiData->use_clippings_ = false;
        guiData->use_grids_ = false;
        guiData->show_merge_option_ = pointClouds.size() > 1;
        controller.updateInfo(guiData);

        m_neededMessageCount--;
        break;
    }
    case IMessage::MessageType::CLIPPING_EXPORT_PARAMETERS:
    {
        auto decodedMsg = static_cast<ClippingExportParametersMessage*>(message);
        m_parameters = decodedMsg->m_parameters;

        if (m_parameters.exportWithScanImportTranslation)
            m_scanTranslationToAdd = -controller.getContext().cgetProjectInfo().m_importScanTranslation;

        if (!prepareOutputDirectory(controller, m_parameters.outFolder))
            return (m_state = ContextState::abort);

        if (m_parameters.outFileType != FileType::E57 && m_parameters.outFileType != FileType::PTS && m_parameters.outFileType != FileType::RCP)
        {
            controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_ERROR));
            return (m_state = ContextState::abort);
        }

        if (m_parameters.fileName.empty())
            m_parameters.fileName = buildDefaultFileName(controller.getContext().cgetProjectInfo().m_projectName);

        m_neededMessageCount--;
        break;
    }
    default:
        break;
    }

    if (m_neededMessageCount > 0)
        m_state = ContextState::waiting_for_input;
    else
        m_state = ContextState::ready_for_using;

    return m_state;
}

ContextState ContextColorimetricFilterExport::launch(Controller& controller)
{
    GraphManager& graphManager = controller.getGraphManager();
    TlStreamLock streamLock;

    std::unordered_set<SafePtr<PointCloudNode>> scans = graphManager.getVisibleScans(xg::Guid());
    if (scans.empty())
    {
        controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_NO_SCAN_SELECTED));
        return (m_state = ContextState::abort);
    }

    std::unordered_set<SafePtr<AClippingNode>> clippings = graphManager.getActivatedOrSelectedClippingObjects();
    ClippingAssembly clippingAssembly;
    if (!clippings.empty())
        graphManager.getClippingAssembly(clippingAssembly, clippings);

    const uint64_t totalScans = scans.size();
    const uint64_t totalProgressSteps = totalScans * 100;
    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString()));
    controller.updateInfo(new GuiDataProcessingSplashScreenStart(totalProgressSteps, TEXT_EXPORT_COLORIMETRIC_FILTER_TITLE_PROGRESS, TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(0).arg(totalScans)));

    CSVWriter csvWriter(m_parameters.outFolder / "summary.csv");
    csvWriter << "Name;Point_Count;X;Y;Z;SizeX;SizeY;SizeZ;RotationX;RotationY;RotationZ" << CSVWriter::endl;

    std::unique_ptr<IScanFileWriter> mergedWriter;
    if (m_parameters.method == ExportPointCloudMerging::CLIPPING_AND_SCAN_MERGED)
    {
        std::wstring writerLog;
        IScanFileWriter* writerRaw = nullptr;
        if (!getScanFileWriter(m_parameters.outFolder, m_parameters.fileName, m_parameters.outFileType, writerLog, &writerRaw, true) || writerRaw == nullptr)
        {
            controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_ERROR));
            controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));
            return (m_state = ContextState::abort);
        }
        mergedWriter.reset(writerRaw);
    }

    uint64_t scanCount = 0;
    bool success = true;
    bool wasAborted = false;

    auto updateProgress = [&](uint64_t scansDone, int percent)
    {
        QString state = QString("%1 - %2%")
                            .arg(TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(scansDone).arg(totalScans))
                            .arg(percent);
        uint64_t progressValue = scansDone * 100;
        if (percent < 100)
            progressValue += static_cast<uint64_t>(percent);
        controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(state, progressValue));
    };

    for (const SafePtr<PointCloudNode>& scan : scans)
    {
        if (m_state != ContextState::running)
        {
            wasAborted = true;
            break;
        }

        WritePtr<PointCloudNode> wScan = scan.get();
        if (!wScan)
            continue;

        const ClippingAssembly* clippingToUse = &clippingAssembly;
        ClippingAssembly resolvedAssembly;
        ClippingAssembly emptyAssembly;
        if (clippingAssembly.empty())
            clippingToUse = &emptyAssembly;
        else if (clippingAssembly.hasPhaseClipping())
        {
            resolvedAssembly = clippingAssembly.resolveByPhase(wScan->getPhase());
            clippingToUse = &resolvedAssembly;
        }

        std::unique_ptr<IScanFileWriter> singleWriter;
        IScanFileWriter* writer = nullptr;
        if (m_parameters.method == ExportPointCloudMerging::SCAN_SEPARATED)
        {
            std::wstring writerLog;
            std::wstring outputName = wScan->getName() + L"_CF";
            IScanFileWriter* writerRaw = nullptr;
            if (!getScanFileWriter(m_parameters.outFolder, outputName, m_parameters.outFileType, writerLog, &writerRaw, true) || writerRaw == nullptr)
            {
                success = false;
                break;
            }
            singleWriter.reset(writerRaw);
            writer = singleWriter.get();
        }
        else
        {
            writer = mergedWriter.get();
        }

        tls::ScanGuid scanGuid = wScan->getScanGuid();
        tls::ScanHeader header;
        TlScanOverseer::getInstance().getScanHeader(scanGuid, header);
        header.guid = xg::newGuid();
        if (m_parameters.method == ExportPointCloudMerging::SCAN_SEPARATED)
            header.name = wScan->getName() + L"_CF";

        writer->appendPointCloud(header, wScan->getTransformation());
        writer->setPostTranslation(m_scanTranslationToAdd);

        size_t scanIndex = scanCount;
        auto progressCallback = [&, scanIndex](size_t processed, size_t total)
        {
            if (total == 0)
                return;
            int percent = static_cast<int>((processed * 100) / total);
            percent = std::clamp(percent, 0, 99);
            updateProgress(scanIndex, percent);
        };

        uint64_t keptPoints = 0;
        success &= TlScanOverseer::getInstance().filterColorimetricAndWrite(scanGuid, (TransformationModule)*&wScan, *clippingToUse, m_filterSettings, m_renderMode, writer, keptPoints, progressCallback);
        success &= writer->finalizePointCloud();

        tls::ScanHeader outHeader = writer->getLastScanHeader();
        csvWriter << outHeader.name << outHeader.pointCount
                  << outHeader.transfo.translation[0] << outHeader.transfo.translation[1] << outHeader.transfo.translation[2]
                  << (outHeader.limits.xMax - outHeader.limits.xMin)
                  << (outHeader.limits.yMax - outHeader.limits.yMin)
                  << (outHeader.limits.zMax - outHeader.limits.zMin)
                  << 0 << 0 << 0 << CSVWriter::endl;

        scanCount++;
        updateProgress(scanCount, 100);

        if (!success)
            break;
    }

    controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));

    if (success && m_parameters.openFolderAfterExport && !wasAborted)
        controller.updateInfo(new GuiDataOpenInExplorer(m_parameters.outFolder));

    return (m_state = (success && !wasAborted) ? ContextState::done : ContextState::abort);
}

bool ContextColorimetricFilterExport::canAutoRelaunch() const
{
    return false;
}

ContextType ContextColorimetricFilterExport::getType() const
{
    return ContextType::colorimetricFilterExport;
}

bool ContextColorimetricFilterExport::prepareOutputDirectory(Controller& controller, const std::filesystem::path& folderPath)
{
    if (std::filesystem::is_directory(folderPath))
        return true;

    try
    {
        if (!std::filesystem::create_directory(folderPath))
        {
            controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_INVALID_DIRECTORY));
            return false;
        }
    }
    catch (std::exception)
    {
        controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_INVALID_DIRECTORY));
        return false;
    }

    return true;
}

bool ContextColorimetricFilterExport::hasActiveColorimetricFilter() const
{
    if (!m_filterSettings.enabled)
        return false;

    auto ordered = ColorimetricFilterUtils::normalizeSettings(m_filterSettings, m_renderMode);
    for (const auto& entry : ordered)
    {
        if (entry.enabled)
            return true;
    }
    return false;
}

std::wstring ContextColorimetricFilterExport::buildDefaultFileName(const std::wstring& projectName) const
{
    std::wstring fileName = L"export_" + projectName;
    std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string str = std::ctime(&time);
    for (char& c : str)
    {
        if (c == ' ' || c == '\r' || c == '\n' || c == ':')
            c = '_';
    }

    fileName += L"_";
    fileName += std::wstring(str.begin(), str.end());
    return fileName;
}
