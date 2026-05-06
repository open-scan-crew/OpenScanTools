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
#include "io/exports/TlsFileWriter.h"
#include "models/graph/GraphManager.h"
#include "models/graph/PointCloudNode.h"
#include "pointCloudEngine/PCE_core.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "utils/Logger.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <thread>

// Note (Aurélien) QT::StandardButtons enum values in qmessagebox.h
#define Yes 0x00004000
#define No 0x00010000

namespace
{
    std::shared_ptr<IClippingGeometry> cloneGeometrySwappingMode(const std::shared_ptr<IClippingGeometry>& geom)
    {
        if (!geom)
            return nullptr;

        ClippingMode swappedMode = geom->mode;
        if (geom->mode == ClippingMode::showInterior)
            swappedMode = ClippingMode::showExterior;
        else if (geom->mode == ClippingMode::showExterior)
            swappedMode = ClippingMode::showInterior;

        std::shared_ptr<IClippingGeometry> cloned;
        switch (geom->getShape())
        {
        case ClippingShape::box:
            cloned = std::make_shared<BoxClippingGeometry>(swappedMode, geom->matRT_inv, geom->params, geom->rampSteps);
            break;
        case ClippingShape::cylinder:
            cloned = std::make_shared<CylinderClippingGeometry>(swappedMode, geom->matRT_inv, geom->params, geom->rampSteps);
            break;
        case ClippingShape::sphere:
            cloned = std::make_shared<SphereClippingGeometry>(swappedMode, geom->matRT_inv, geom->params, geom->rampSteps);
            break;
        case ClippingShape::torus:
            cloned = std::make_shared<TorusClippingGeometry>(swappedMode, geom->matRT_inv, geom->params, geom->rampSteps);
            break;
        default:
            return nullptr;
        }
        cloned->color = geom->color;
        cloned->gpuDrawId = geom->gpuDrawId;
        cloned->isSelected = geom->isSelected;
        cloned->clipperPhase = geom->clipperPhase;
        return cloned;
    }

    ClippingAssembly buildComplementaryAssembly(const ClippingAssembly& clipping)
    {
        ClippingAssembly out;
        for (const std::shared_ptr<IClippingGeometry>& geom : clipping.clippingUnion)
        {
            std::shared_ptr<IClippingGeometry> cloned = cloneGeometrySwappingMode(geom);
            if (!cloned)
                continue;
            if (cloned->mode == ClippingMode::showInterior)
                out.clippingUnion.push_back(cloned);
            else
                out.clippingIntersection.push_back(cloned);
        }
        for (const std::shared_ptr<IClippingGeometry>& geom : clipping.clippingIntersection)
        {
            std::shared_ptr<IClippingGeometry> cloned = cloneGeometrySwappingMode(geom);
            if (!cloned)
                continue;
            if (cloned->mode == ClippingMode::showInterior)
                out.clippingUnion.push_back(cloned);
            else
                out.clippingIntersection.push_back(cloned);
        }
        return out;
    }

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
    // Reuse robust scan replacement strategy already used in point deletion:
    // load temp tls, then copy it over the original scan file path.
    bool commitTempScanToProject(Controller& controller, WritePtr<PointCloudNode>& wScan, const std::filesystem::path& tempPath, const QString& scanName)
    {
        if (!wScan || tempPath.empty())
            return false;

        tls::ScanGuid newGuid;
        if (!TlScanOverseer::getInstance().getScanGuid(tempPath, newGuid))
        {
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Failed to load temporary scan for %1.").arg(scanName)));
            return false;
        }

        std::filesystem::path absolutePath = wScan->getTlsFilePath();
        tls::ScanGuid oldGuid = wScan->getScanGuid();

        TlScanOverseer::getInstance().freeScan_async(oldGuid, false);
        wScan->setTlsFilePath(tempPath, false, tls::ScanGuid(), false);
        TlScanOverseer::getInstance().copyScanFile_async(newGuid, absolutePath, false, true, true);
        return true;
    }
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
        m_executionMode = decodedMsg->executionMode;
        m_outputFileType = decodedMsg->outputFileType;
        m_outputFolder = decodedMsg->outputFolder;
        m_openFolderAfterExport = decodedMsg->openFolderAfterExport;

        m_warningModal = true;
        if (m_executionMode == FilterExecutionMode::ApplyOnProject)
            controller.updateInfo(new GuiDataModal(Yes | No, TEXT_DELETE_POINTS_QUESTION));
        else
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

    const bool applyOnProject = m_executionMode == FilterExecutionMode::ApplyOnProject;
    const std::filesystem::path scanFolder = controller.getContext().cgetProjectInternalInfo().getPointCloudFolderPath(false);
    const std::filesystem::path tempFolder = scanFolder / "temp_sof";
    const std::filesystem::path outputFolder = applyOnProject ? tempFolder : m_outputFolder;
    if (!prepareOutputDirectory(controller, outputFolder))
    {
        m_state = ContextState::abort;
        return m_state;
    }

    TlStreamLock streamLock;

    std::unordered_set<SafePtr<PointCloudNode>> scans = graphManager.getVisibleScans(m_panoramic);

    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString()));
    const uint64_t totalScans = scans.size();
    const uint64_t totalProgressSteps = totalScans * 100;
    controller.updateInfo(new GuiDataProcessingSplashScreenStart(totalProgressSteps, TEXT_EXPORT_STAT_OUTLIER_TITLE_PROGESS, TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(0).arg(totalScans)));

    ClippingAssembly clippingAssembly;
    graphManager.getClippingAssembly(clippingAssembly, true, false);
    OutlierStats globalStats;
    bool wasAborted = false;
    if (m_globalFiltering)
    {
        RunningStats runningStats;
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

    uint64_t scan_count = 0;
    uint64_t total_deleted_points = 0;
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
        std::filesystem::path tempPath;
        if (applyOnProject)
        {
            TlsFileWriter* tlsWriter = nullptr;
            TlsFileWriter::getWriter(outputFolder, wScan->getName(), log, (IScanFileWriter**)&tlsWriter);
            scan_writer = tlsWriter;
            if (tlsWriter != nullptr)
                tempPath = tlsWriter->getFilePath();
        }
        else
        {
            std::wstring outputName = wScan->getName() + L"_SOF";
            getScanFileWriter(outputFolder, outputName, m_outputFileType, log, &scan_writer, true);
        }
        if (scan_writer == nullptr)
            continue;
        tls::ScanHeader header;
        TlScanOverseer::getInstance().getScanHeader(old_guid, header);
        header.guid = xg::newGuid();
        scan_writer->appendPointCloud(header, wScan->getTransformation());

        OutlierStats statsToUse = globalStats;
        if (!m_globalFiltering)
        {
            auto statsProgress = makeProgressCallback(scan_count, 0, 50);
            TlScanOverseer::getInstance().computeOutlierStats(old_guid, (TransformationModule)*&wScan, *clippingToUse, m_kNeighbors, m_samplingPercent, m_beta, statsToUse, statsProgress);
        }

        auto filterProgress = makeProgressCallback(scan_count, m_globalFiltering ? 0 : 50, m_globalFiltering ? 100 : 50);
        bool res = TlScanOverseer::getInstance().filterOutliersAndWrite(old_guid, (TransformationModule)*&wScan, *clippingToUse, m_kNeighbors, statsToUse, m_nSigma, m_beta, scan_writer, deleted_point_count, filterProgress);
        res &= scan_writer->finalizePointCloud();
        delete scan_writer;

        total_deleted_points += deleted_point_count;

        scan_count++;
        float seconds = std::chrono::duration<float, std::ratio<1>>(std::chrono::steady_clock::now() - startTime).count();
        QString qScanName = QString::fromStdWString(wScan->getName());
        updateProgress(scan_count, 100, scan_count * 100);

        if (applyOnProject && !clippingToUse->empty())
        {
            // 3A strategy:
            // A = filtered clipped area, B = untouched outside-clipping area, final = A+B merged.
            std::wstring logTmp;
            TlsFileWriter* outsideWriter = nullptr;
            TlsFileWriter::getWriter(outputFolder, wScan->getName() + L"_outside", logTmp, (IScanFileWriter**)&outsideWriter);
            if (outsideWriter != nullptr)
            {
                tls::ScanHeader outsideHeader;
                TlScanOverseer::getInstance().getScanHeader(old_guid, outsideHeader);
                outsideHeader.guid = xg::newGuid();
                outsideWriter->appendPointCloud(outsideHeader, wScan->getTransformation());
                ClippingAssembly complement = buildComplementaryAssembly(*clippingToUse);
                bool outsideOk = TlScanOverseer::getInstance().clipScan(old_guid, (TransformationModule)*&wScan, complement, outsideWriter);
                outsideOk &= outsideWriter->finalizePointCloud();
                std::filesystem::path outsidePath = outsideWriter->getFilePath();
                delete outsideWriter;

                tls::ScanGuid filteredGuid;
                tls::ScanGuid outsideGuid;
                if (res
                    && TlScanOverseer::getInstance().getScanGuid(tempPath, filteredGuid)
                    && outsideOk
                    && TlScanOverseer::getInstance().getScanGuid(outsidePath, outsideGuid))
                {
                    TlsFileWriter* mergedWriter = nullptr;
                    TlsFileWriter::getWriter(outputFolder, wScan->getName(), logTmp, (IScanFileWriter**)&mergedWriter);
                    if (mergedWriter != nullptr)
                    {
                        std::filesystem::path mergedPath = mergedWriter->getFilePath();
                        tls::ScanHeader mergedHeader;
                        TlScanOverseer::getInstance().getScanHeader(old_guid, mergedHeader);
                        mergedHeader.guid = xg::newGuid();
                        mergedWriter->appendPointCloud(mergedHeader, wScan->getTransformation());
                        ClippingAssembly emptyClipping;
                        bool mergeOk = TlScanOverseer::getInstance().clipScan(filteredGuid, (TransformationModule)*&wScan, emptyClipping, mergedWriter);
                        mergeOk &= TlScanOverseer::getInstance().clipScan(outsideGuid, (TransformationModule)*&wScan, emptyClipping, mergedWriter);
                        mergeOk &= mergedWriter->finalizePointCloud();
                        delete mergedWriter;
                        TlScanOverseer::getInstance().freeScan_async(filteredGuid, false);
                        TlScanOverseer::getInstance().freeScan_async(outsideGuid, false);
                        if (mergeOk)
                            tempPath = mergedPath;
                    }
                }
            }
        }

        if (deleted_point_count > 0)
        {
            if (applyOnProject && res)
            {
                if (!commitTempScanToProject(controller, wScan, tempPath, qScanName))
                    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Failed to replace scan %1 after filtering.").arg(qScanName)));
            }
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("%1 points deleted in scan %2 in %3 seconds.").arg(deleted_point_count).arg(qScanName).arg(seconds)));
        }
        else
        {
            if (applyOnProject && !tempPath.empty())
            {
                std::error_code ec;
                std::filesystem::remove(tempPath, ec);
            }
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Scan %1 not affected by outlier filter.").arg(qScanName)));
        }

        if (m_state != ContextState::running)
        {
            wasAborted = true;
            break;
        }
    }

    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Total points deleted: %1").arg(total_deleted_points)));
    controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));

    if (!applyOnProject && m_openFolderAfterExport && !wasAborted)
        controller.updateInfo(new GuiDataOpenInExplorer(m_outputFolder));

    m_state = wasAborted ? ContextState::abort : ContextState::done;
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
