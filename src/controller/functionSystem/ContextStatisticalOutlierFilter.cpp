#include "controller/functionSystem/ContextStatisticalOutlierFilter.h"

#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/messages/ModalMessage.h"
#include "controller/messages/StatisticalOutlierFilterMessage.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataIO.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"
#include "gui/texts/ExportTexts.hpp"
#include "gui/texts/SplashScreenTexts.hpp"
#include "io/exports/IScanFileWriter.h"
#include "models/graph/GraphManager.h"
#include "models/graph/PointCloudNode.h"
#include "pointCloudEngine/PCE_core.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "utils/Config.h"
#include "utils/Logger.h"
#include "utils/ScanJobRunner.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <mutex>

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
        m_samplingPercent = decodedMsg->samplingPercent;
        m_beta = decodedMsg->beta;
        m_globalFiltering = decodedMsg->mode == OutlierFilterMode::Global;
        m_outputFileType = decodedMsg->outputFileType;
        m_outputFolder = decodedMsg->outputFolder;
        m_openFolderAfterExport = decodedMsg->openFolderAfterExport;

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
    std::vector<SafePtr<PointCloudNode>> scanList(scans.begin(), scans.end());

    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString()));
    const uint64_t totalScans = scanList.size();
    const uint64_t totalProgressSteps = totalScans * 100;
    controller.updateInfo(new GuiDataProcessingSplashScreenStart(totalProgressSteps, TEXT_EXPORT_STAT_OUTLIER_TITLE_PROGESS, TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(0).arg(totalScans)));

    ClippingAssembly clippingAssembly;
    graphManager.getClippingAssembly(clippingAssembly, true, false);

    OutlierStats globalStats;
    if (m_globalFiltering)
    {
        RunningStats runningStats;
        for (const SafePtr<PointCloudNode>& scan : scanList)
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
            TlScanOverseer::getInstance().computeOutlierStats(wScan->getScanGuid(), (TransformationModule)*&wScan, *clippingToUse, m_kNeighbors, m_samplingPercent, m_beta, stats);
            runningStats.addStats(stats);
        }
        globalStats = runningStats.toStats();
    }

    std::mutex infoMutex;
    auto safeUpdateInfo = [&](IGuiData* data)
    {
        std::lock_guard<std::mutex> lock(infoMutex);
        controller.updateInfo(data);
    };

    std::vector<std::atomic<int>> scanPercents(totalScans);
    for (auto& percent : scanPercents)
        percent.store(0);
    std::atomic<uint64_t> completedScans{0};
    std::atomic<uint64_t> progressSteps{0};
    std::atomic<uint64_t> totalDeletedPoints{0};

    auto updateProgress = [&](int percent, uint64_t progressValue)
    {
        uint64_t scansDone = completedScans.load();
        QString state = QString("%1 - %2%")
                            .arg(TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(scansDone).arg(totalScans))
                            .arg(percent);
        safeUpdateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(state, progressValue));
    };

    auto updateProgressForScan = [&](size_t scanIndex, int percent)
    {
        percent = std::clamp(percent, 0, 100);
        int prev = scanPercents[scanIndex].load();
        while (percent > prev && !scanPercents[scanIndex].compare_exchange_weak(prev, percent))
        {
        }
        int delta = percent - prev;
        if (delta <= 0)
            return;
        uint64_t newSteps = progressSteps.fetch_add(static_cast<uint64_t>(delta)) + static_cast<uint64_t>(delta);
        uint64_t totalStepsMax = totalProgressSteps;
        bool done = newSteps >= totalStepsMax;
        int displayPercent = done ? 100 : std::min(99, static_cast<int>((newSteps * 100) / totalStepsMax));
        uint64_t progressValue = done ? totalStepsMax : std::min(newSteps, totalStepsMax - 1);
        updateProgress(displayPercent, progressValue);
    };

    auto makeProgressCallback = [&](size_t scanIndex, int basePercent, int spanPercent)
    {
        return [scanIndex, basePercent, spanPercent, &updateProgressForScan](size_t processed, size_t total)
        {
            if (total == 0)
                return;
            int percent = basePercent + static_cast<int>((processed * spanPercent) / total);
            percent = std::clamp(percent, basePercent, basePercent + spanPercent);
            if (percent >= 100)
                percent = 99;
            updateProgressForScan(scanIndex, percent);
        };
    };

    ScanJobRunner::Options options;
    options.multithreaded = Config::isMultithreadedCalculation();
    ScanJobRunner::run(scanList, options, [&](size_t scanIndex, const SafePtr<PointCloudNode>& scan)
    {
        WritePtr<PointCloudNode> wScan = scan.get();
        if (!wScan)
            return;

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

        IScanFileWriter* scan_writer = nullptr;
        std::wstring log;
        std::wstring outputName = wScan->getName() + L"_SOF";
        if (!getScanFileWriter(m_outputFolder, outputName, m_outputFileType, log, &scan_writer, true) || scan_writer == nullptr)
            continue;
        tls::ScanHeader header;
        TlScanOverseer::getInstance().getScanHeader(old_guid, header);
        header.guid = xg::newGuid();
        scan_writer->appendPointCloud(header, wScan->getTransformation());

        OutlierStats statsToUse = globalStats;
        if (!m_globalFiltering)
        {
            auto statsProgress = makeProgressCallback(scanIndex, 0, 50);
            TlScanOverseer::getInstance().computeOutlierStats(old_guid, (TransformationModule)*&wScan, *clippingToUse, m_kNeighbors, m_samplingPercent, m_beta, statsToUse, statsProgress);
        }

        auto filterProgress = makeProgressCallback(scanIndex, m_globalFiltering ? 0 : 50, m_globalFiltering ? 100 : 50);
        bool res = TlScanOverseer::getInstance().filterOutliersAndWrite(old_guid, (TransformationModule)*&wScan, *clippingToUse, m_kNeighbors, statsToUse, m_nSigma, m_beta, scan_writer, deleted_point_count, filterProgress);
        res &= scan_writer->finalizePointCloud();
        delete scan_writer;

        totalDeletedPoints.fetch_add(deleted_point_count);
        completedScans.fetch_add(1);
        updateProgressForScan(scanIndex, 100);
        float seconds = std::chrono::duration<float, std::ratio<1>>(std::chrono::steady_clock::now() - startTime).count();
        QString qScanName = QString::fromStdWString(wScan->getName());

        if (deleted_point_count > 0)
            safeUpdateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("%1 points deleted in scan %2 in %3 seconds.").arg(deleted_point_count).arg(qScanName).arg(seconds)));
        else
            safeUpdateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Scan %1 not affected by outlier filter.").arg(qScanName)));
    });

    updateProgress(100, totalProgressSteps);
    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Total points deleted: %1").arg(totalDeletedPoints.load())));
    controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));

    if (m_openFolderAfterExport)
        controller.updateInfo(new GuiDataOpenInExplorer(m_outputFolder));

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
