#include "controller/functionSystem/ContextSmoothPointCloud.h"

#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/IControlListener.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/messages/ModalMessage.h"
#include "controller/messages/SmoothPointCloudMessage.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/texts/ContextTexts.hpp"
#include "gui/texts/ExportTexts.hpp"
#include "gui/texts/SplashScreenTexts.hpp"
#include "io/exports/TlsFileWriter.h"
#include "pointCloudEngine/PCE_core.h"
#include "pointCloudEngine/TlScanOverseer.h"

#include "models/graph/PointCloudNode.h"
#include "models/graph/GraphManager.h"

#include "utils/Logger.h"

#include <chrono>

// Note (Aurélien) QT::StandardButtons enum values in qmessagebox.h
#define Yes 0x00004000
#define No 0x00010000
#define Cancel 0x00400000

ContextSmoothPointCloud::ContextSmoothPointCloud(const ContextId& id)
    : AContext(id)
    , m_panoramic(xg::Guid())
{
    m_state = ContextState::waiting_for_input;
    m_warningModal = false;
}

ContextSmoothPointCloud::~ContextSmoothPointCloud()
{}

ContextState ContextSmoothPointCloud::start(Controller& controller)
{
    if (controller.getContext().getIsCurrentProjectSaved() == false)
        controller.updateInfo(new GuiDataModal(Yes | No, TEXT_SMOOTH_SAVE_BEFORE_QUESTION));

    GraphManager& graphManager = controller.getGraphManager();

    if (graphManager.getVisibleScans(m_panoramic).empty())
    {
        FUNCLOG << "No Scans visibles to smooth" << LOGENDL;
        controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_NO_SCAN_SELECTED));
        return (m_state = ContextState::abort);
    }

    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextSmoothPointCloud::feedMessage(IMessage* message, Controller& controller)
{
    switch (message->getType())
    {
    case IMessage::MessageType::MODAL:
    {
        if (!m_warningModal)
        {
            ModalMessage* modal = static_cast<ModalMessage*>(message);
            if (modal->m_returnedValue == Yes)
                m_saveContext = controller.getFunctionManager().launchBackgroundFunction(controller, ContextType::saveProject, m_id);
        }
        else
        {
            ModalMessage* modal = static_cast<ModalMessage*>(message);
            if (modal->m_returnedValue == Yes)
                m_state = ContextState::ready_for_using;
            else
                m_state = ContextState::abort;
        }
    }
    break;
    case IMessage::MessageType::SMOOTH_POINT_CLOUD_PARAMETERS:
    {
        auto decodedMsg = static_cast<SmoothPointCloudMessage*>(message);

        m_parameters.maxDisplacementMm = decodedMsg->maxDisplacementMm;
        m_parameters.voxelSizeMm = decodedMsg->voxelSizeMm;
        m_parameters.adaptiveVoxel = decodedMsg->adaptiveVoxel;
        m_parameters.preserveEdges = decodedMsg->preserveEdges;

        m_warningModal = true;
        controller.updateInfo(new GuiDataModal(Yes | Cancel, TEXT_SMOOTH_POINTS_QUESTION));
    }
    break;
    default:
    {
        FUNCLOG << "Context smooth point cloud: wrong message type" << LOGENDL;
    }
    }

    return (m_state);
}

ContextState ContextSmoothPointCloud::launch(Controller& controller)
{
    GraphManager& graphManager = controller.getGraphManager();

    std::filesystem::path temp_folder = controller.getContext().cgetProjectInternalInfo().getPointCloudFolderPath(false) / "temp";
    prepareOutputDirectory(controller, temp_folder);

    TlStreamLock streamLock;

    std::unordered_set<SafePtr<PointCloudNode>> scans = graphManager.getVisibleScans(m_panoramic);

    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString()));
    controller.updateInfo(new GuiDataProcessingSplashScreenStart(scans.size(), TEXT_EXPORT_CLIPPING_TITLE_PROGESS, TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(0).arg(scans.size())));

    PointCloudSmoothingEngine smoothingEngine;

    uint64_t scan_count = 0;
    for (const SafePtr<PointCloudNode>& scan : scans)
    {
        WritePtr<PointCloudNode> wScan = scan.get();
        if (!wScan)
            continue;

        std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
        std::filesystem::path temp_path;
        tls::ScanGuid old_guid = wScan->getScanGuid();
        tls::ScanGuid new_guid = old_guid;

        TlsFileWriter* tls_writer = nullptr;
        std::wstring log;
        TlsFileWriter::getWriter(temp_folder, wScan->getName(), log, (IScanFileWriter**)&tls_writer);
        if (tls_writer == nullptr)
            continue;

        temp_path = tls_writer->getFilePath();
        tls::ScanHeader header;
        TlScanOverseer::getInstance().getScanHeader(old_guid, header);
        header.guid = xg::newGuid();
        tls_writer->appendPointCloud(header, wScan->getTransformation());

        uint64_t processedPoints = 0;
        bool res = smoothingEngine.smoothScan(wScan->getTlsFilePath(), tls_writer, m_parameters, processedPoints, log);
        res &= tls_writer->finalizePointCloud();
        delete tls_writer;

        if (res)
        {
            res &= TlScanOverseer::getInstance().getScanGuid(temp_path, new_guid);
        }

        scan_count++;
        float seconds = std::chrono::duration<float, std::ratio<1>>(std::chrono::steady_clock::now() - startTime).count();
        QString qScanName = QString::fromStdWString(wScan->getName());
        controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(scan_count).arg(scans.size()), scan_count));

        if (res && new_guid != old_guid)
        {
            std::filesystem::path absolutePath = wScan->getTlsFilePath();
            TlScanOverseer::getInstance().freeScan_async(old_guid, false);
            wScan->setTlsFilePath(temp_path, false);
            TlScanOverseer::getInstance().copyScanFile_async(new_guid, absolutePath, false, true, true);
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("%1 points smoothed in scan %2 in %3 seconds.").arg(processedPoints).arg(qScanName).arg(seconds)));
        }
        else if (!res)
        {
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Scan %1 smoothing failed.").arg(qScanName)));
        }
        else
        {
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Scan %1 not affected by smoothing.").arg(qScanName)));
        }
    }

    controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));

    m_state = ContextState::done;
    return (m_state);
}

bool ContextSmoothPointCloud::canAutoRelaunch() const
{
    return (false);
}

ContextType ContextSmoothPointCloud::getType() const
{
    return (ContextType::smoothPointCloud);
}

bool ContextSmoothPointCloud::prepareOutputDirectory(Controller& controller, const std::filesystem::path& folderPath)
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
