#include "controller/functionSystem/ContextSmoothPoints.h"

#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/functionSystem/FunctionManager.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "io/exports/TlsFileWriter.h"
#include "models/graph/GraphManager.h"
#include "models/graph/PointCloudNode.h"
#include "pointCloudEngine/PCE_core.h"
#include "pointCloudEngine/TlScanOverseer.h"

#include "gui/texts/ExportTexts.hpp"
#include "gui/texts/SplashScreenTexts.hpp"

#include "utils/Logger.h"

ContextSmoothPoints::ContextSmoothPoints(const ContextId& id)
    : AContext(id)
    , m_panoramic(xg::Guid())
{
    m_state = ContextState::waiting_for_input;
}

ContextSmoothPoints::~ContextSmoothPoints()
{}

ContextState ContextSmoothPoints::start(Controller& controller)
{
    GraphManager& graphManager = controller.getGraphManager();
    if (graphManager.getVisibleScans(m_panoramic).empty())
    {
        FUNCLOG << "No scans visible to smooth" << LOGENDL;
        controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_NO_SCAN_SELECTED));
        return (m_state = ContextState::abort);
    }

    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextSmoothPoints::feedMessage(IMessage* message, Controller& controller)
{
    if (message->getType() != IMessage::MessageType::SMOOTH_POINTS_PARAMETERS)
    {
        FUNCLOG << "Context smooth points: wrong message type" << LOGENDL;
        return (m_state = ContextState::abort);
    }

    auto* decodedMsg = static_cast<SmoothPointsMessage*>(message);
    m_params = decodedMsg->params;
    m_hasParameters = true;
    return (m_state = ContextState::ready_for_using);
}

ContextState ContextSmoothPoints::launch(Controller& controller)
{
    if (!m_hasParameters)
        return (m_state = ContextState::abort);

    GraphManager& graphManager = controller.getGraphManager();

    std::filesystem::path temp_folder = controller.getContext().cgetProjectInternalInfo().getPointCloudFolderPath(false) / "temp";
    prepareOutputDirectory(controller, temp_folder);

    TlStreamLock streamLock;

    std::unordered_set<SafePtr<PointCloudNode>> scans = graphManager.getVisibleScans(m_panoramic);

    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString()));
    controller.updateInfo(new GuiDataProcessingSplashScreenStart(scans.size(), TEXT_SPLASH_SCREEN_SMOOTHING_TITLE, TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(0).arg(scans.size())));

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
        uint64_t moved_points = 0;

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

        bool res = TlScanOverseer::getInstance().smoothScan(old_guid, (TransformationModule)*&wScan, m_params, tls_writer, moved_points);
        res &= tls_writer->finalizePointCloud();
        delete tls_writer;

        res &= TlScanOverseer::getInstance().getScanGuid(temp_path, new_guid);

        scan_count++;
        float seconds = std::chrono::duration<float, std::ratio<1>>(std::chrono::steady_clock::now() - startTime).count();
        QString qScanName = QString::fromStdWString(wScan->getName());
        controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(scan_count).arg(scans.size()), scan_count));

        if (!res || new_guid == xg::Guid())
        {
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Failed to smooth scan %1.").arg(qScanName)));
            continue;
        }

        std::filesystem::path absolutePath = wScan->getTlsFilePath();
        TlScanOverseer::getInstance().freeScan_async(old_guid, false);
        wScan->setTlsFilePath(temp_path, false);
        TlScanOverseer::getInstance().copyScanFile_async(new_guid, absolutePath, false, true, true);

        controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("%1 points smoothed in scan %2 in %3 seconds.").arg(moved_points).arg(qScanName).arg(seconds)));
    }

    controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));

    m_state = ContextState::done;
    return (m_state);
}

bool ContextSmoothPoints::canAutoRelaunch() const
{
    return (false);
}

ContextType ContextSmoothPoints::getType() const
{
    return (ContextType::smoothPoints);
}

bool ContextSmoothPoints::prepareOutputDirectory(Controller& controller, const std::filesystem::path& folderPath)
{
    if (std::filesystem::is_directory(folderPath) == false)
    {
        try
        {
            if (std::filesystem::create_directory(folderPath) == false)
            {
                controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_INVALID_DIRECTORY));
                return false;
            }
        }
        catch (std::exception&)
        {
            controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_INVALID_DIRECTORY));
            return false;
        }
    }
    return true;
}
