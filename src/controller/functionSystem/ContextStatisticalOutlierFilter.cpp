#include "controller/functionSystem/ContextStatisticalOutlierFilter.h"

#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/messages/ModalMessage.h"
#include "controller/messages/StatisticalOutlierFilterMessage.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
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

#include <cmath>
#include <filesystem>

// Note (Aurélien) QT::StandardButtons enum values in qmessagebox.h
#define Yes 0x00004000
#define No 0x00010000

namespace
{
    struct RunningStats
    {
        uint64_t count = 0;
        double mean = 0.0;
        double m2 = 0.0;

        void addStats(const OutlierStats& stats)
        {
            if (stats.count == 0)
                return;

            double variance = stats.stddev * stats.stddev;
            double statsM2 = variance * static_cast<double>(stats.count > 1 ? stats.count - 1 : 0);

            if (count == 0)
            {
                count = stats.count;
                mean = stats.mean;
                m2 = statsM2;
                return;
            }

            double delta = stats.mean - mean;
            uint64_t newCount = count + stats.count;
            mean += delta * static_cast<double>(stats.count) / static_cast<double>(newCount);
            m2 += statsM2 + delta * delta * static_cast<double>(count) * static_cast<double>(stats.count) / static_cast<double>(newCount);
            count = newCount;
        }

        OutlierStats toStats() const
        {
            OutlierStats stats;
            stats.count = count;
            stats.mean = mean;
            if (count < 2)
            {
                stats.stddev = 0.0;
            }
            else
            {
                double variance = m2 / static_cast<double>(count - 1);
                stats.stddev = sqrt(variance);
            }
            return stats;
        }
    };
}

ContextStatisticalOutlierFilter::ContextStatisticalOutlierFilter(const ContextId& id)
    : AContext(id)
    , m_panoramic(xg::Guid())
{
    m_state = ContextState::waiting_for_input;
}

ContextStatisticalOutlierFilter::~ContextStatisticalOutlierFilter()
{}

ContextState ContextStatisticalOutlierFilter::start(Controller& controller)
{
    GraphManager& graphManager = controller.getGraphManager();

    if (graphManager.getVisibleScans(m_panoramic).empty())
    {
        FUNCLOG << "No Scans visibles to clean" << LOGENDL;
        controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_NO_SCAN_SELECTED));
        return (m_state = ContextState::abort);
    }

    controller.updateInfo(new GuiDataStatisticalOutlierFilterDialogDisplay());

    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextStatisticalOutlierFilter::feedMessage(IMessage* message, Controller& controller)
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
    case IMessage::MessageType::STAT_OUTLIER_FILTER_PARAMETERS:
    {
        auto decodedMsg = static_cast<StatisticalOutlierFilterMessage*>(message);
        m_kNeighbors = decodedMsg->kNeighbors;
        m_nSigma = decodedMsg->nSigma;
        m_globalFiltering = decodedMsg->mode == OutlierFilterMode::Global;
        m_outputFolder = decodedMsg->outputFolder;

        m_warningModal = true;
        controller.updateInfo(new GuiDataModal(Yes | No, TEXT_STAT_OUTLIER_FILTER_QUESTION));
        break;
    }
    break;
    default:
        break;
    }

    return (m_state);
}

ContextState ContextStatisticalOutlierFilter::launch(Controller& controller)
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
    controller.updateInfo(new GuiDataProcessingSplashScreenStart(scans.size(), TEXT_EXPORT_STAT_OUTLIER_TITLE_PROGESS, TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(0).arg(scans.size())));

    ClippingAssembly clippingAssembly;
    graphManager.getClippingAssembly(clippingAssembly, true, false);

    OutlierStats globalStats;
    if (m_globalFiltering)
    {
        RunningStats runningStats;
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

            OutlierStats stats;
            TlScanOverseer::getInstance().computeOutlierStats(wScan->getScanGuid(), (TransformationModule)*&wScan, *clippingToUse, m_kNeighbors, stats);
            runningStats.addStats(stats);
        }
        globalStats = runningStats.toStats();
    }

    uint64_t scan_count = 0;
    uint64_t total_deleted_points = 0;
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

        size_t initial_point_count = wScan->getNbPoint();
        uint64_t deleted_point_count = 0;
        std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
        tls::ScanGuid old_guid = wScan->getScanGuid();

        TlsFileWriter* tls_writer = nullptr;
        std::wstring log;
        std::wstring outputName = wScan->getName() + L"_SOF";
        TlsFileWriter::getWriter(m_outputFolder, outputName, log, (IScanFileWriter**)&tls_writer);
        if (tls_writer == nullptr)
            continue;
        tls::ScanHeader header;
        TlScanOverseer::getInstance().getScanHeader(old_guid, header);
        header.guid = xg::newGuid();
        tls_writer->appendPointCloud(header, wScan->getTransformation());

        OutlierStats statsToUse = globalStats;
        if (!m_globalFiltering)
            TlScanOverseer::getInstance().computeOutlierStats(old_guid, (TransformationModule)*&wScan, *clippingToUse, m_kNeighbors, statsToUse);

        bool res = TlScanOverseer::getInstance().filterOutliersAndWrite(old_guid, (TransformationModule)*&wScan, *clippingToUse, m_kNeighbors, statsToUse, m_nSigma, tls_writer, deleted_point_count);
        res &= tls_writer->finalizePointCloud();
        delete tls_writer;

        total_deleted_points += deleted_point_count;

        scan_count++;
        float seconds = std::chrono::duration<float, std::ratio<1>>(std::chrono::steady_clock::now() - startTime).count();
        QString qScanName = QString::fromStdWString(wScan->getName());
        controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(scan_count).arg(scans.size()), scan_count));

        if (deleted_point_count > 0)
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("%1 points deleted in scan %2 in %3 seconds.").arg(deleted_point_count).arg(qScanName).arg(seconds)));
        else
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Scan %1 not affected by outlier filter.").arg(qScanName)));
    }

    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Total points deleted: %1").arg(total_deleted_points)));
    controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));

    m_state = ContextState::done;
    return (m_state);
}

bool ContextStatisticalOutlierFilter::canAutoRelaunch() const
{
    return false;
}

ContextType ContextStatisticalOutlierFilter::getType() const
{
    return ContextType::statisticalOutlierFilter;
}

bool ContextStatisticalOutlierFilter::prepareOutputDirectory(Controller& controller, const std::filesystem::path& folderPath)
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
