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
#include "utils/Logger.h"

#include <algorithm>
#include <cmath>
#include <filesystem>

// Note (Aurélien) QT::StandardButtons enum values in qmessagebox.h
#define Yes 0x00004000
#define No 0x00010000

namespace
{
    struct SofPreAnalysisRunResult
    {
        SofPreAnalysisStats stats;
        float durationSeconds = 0.0f;
    };

    struct SofSubdivisionShadowConfig
    {
        bool enabled = false;
        uint32_t factor = 1;
        uint32_t subBoxCount = 1;
        double haloMeters = 0.0;
        uint32_t fallbackSubBoxCount = 0;
    };

    struct SofSubdivisionSpatialDiagnostics
    {
        double haloMediumMeters = 0.0;
        double haloStrongMeters = 0.0;
        double estimatedPointsPerSubBox = 0.0;
        double estimatedDenseSubBoxRatio = 0.0;
    };

    SofSubdivisionShadowConfig buildSubdivisionShadowConfig(const SofPreAnalysisStats& preAnalysis)
    {
        // Passe 2A: décision conservative en "shadow mode" uniquement.
        SofSubdivisionShadowConfig config;
        config.enabled = preAnalysis.riskClass == SofPreAnalysisRiskClass::High;
        if (!config.enabled)
            return config;

        // Facteur borné pour éviter les explosions de coûts dès la passe d'ossature.
        const uint32_t maxFactor = 4;
        uint32_t suggestedFactor = preAnalysis.testedPointsEstimate > 50000000 ? 4 : 2;
        config.factor = std::clamp(suggestedFactor, 2u, maxFactor);
        config.subBoxCount = config.factor * config.factor * config.factor;

        // Halo "proxy" basé sur les métriques existantes. La valeur n'est pas encore appliquée au filtre.
        config.haloMeters = std::clamp(preAnalysis.spacingMean * 2.0, 0.01, 1.0);

        // Fallback planifié (non exécuté en 2A) pour sous-zones peu peuplées.
        uint32_t minPointsPerSubBox = 5000;
        double avgPointsPerSubBox = static_cast<double>(std::max<uint64_t>(preAnalysis.testedPointsEstimate, 1)) / static_cast<double>(config.subBoxCount);
        if (avgPointsPerSubBox < static_cast<double>(minPointsPerSubBox))
            config.fallbackSubBoxCount = config.subBoxCount;
        return config;
    }

    SofSubdivisionSpatialDiagnostics buildSubdivisionSpatialDiagnostics(const SofPreAnalysisStats& preAnalysis, const SofSubdivisionShadowConfig& shadowConfig)
    {
        SofSubdivisionSpatialDiagnostics diag;
        const uint32_t effectiveSubBoxCount = std::max(1u, shadowConfig.subBoxCount);

        // Passe 2B.A: halo adaptatif moyen/fort pour le diagnostic spatial.
        // Ces valeurs sont loggées pour calibration et n'impactent pas le filtre en sortie.
        diag.haloMediumMeters = std::clamp(preAnalysis.spacingMean * 2.5, 0.01, 2.0);
        diag.haloStrongMeters = std::clamp(preAnalysis.spacingMean * 3.5, 0.01, 2.0);

        diag.estimatedPointsPerSubBox =
            static_cast<double>(std::max<uint64_t>(preAnalysis.testedPointsEstimate, 1)) /
            static_cast<double>(effectiveSubBoxCount);

        const double heterogeneity = std::clamp(0.5 * preAnalysis.spacingCv + 0.5 * preAnalysis.meanDistanceCv, 0.0, 4.0);
        diag.estimatedDenseSubBoxRatio = std::clamp(1.0 - (heterogeneity / 4.0), 0.0, 1.0);
        return diag;
    }

    SofPreAnalysisRunResult runSofPreAnalysis(
        const tls::ScanGuid& scanGuid,
        const WritePtr<PointCloudNode>& wScan,
        const ClippingAssembly& clipping,
        uint16_t kNeighbors,
        uint8_t samplingPercent,
        double beta)
    {
        SofPreAnalysisRunResult result;
        const auto preAnalysisStart = std::chrono::steady_clock::now();

        TlScanOverseer::getInstance().computeOutlierPreAnalysis(
            scanGuid,
            (TransformationModule)*&wScan,
            clipping,
            kNeighbors,
            samplingPercent,
            beta,
            result.stats);

        result.durationSeconds = std::chrono::duration<float, std::ratio<1>>(
            std::chrono::steady_clock::now() - preAnalysisStart)
                                     .count();
        return result;
    }


    struct Sof2CCalibration
    {
        // 2C-1: HIGH-only calibration constants (BORDERLINE remains unchanged/off).
        uint32_t maxSubBoxes = 64;
        double minEstimatedPointsPerSubBox = 3000.0;
        double minDenseRatio = 0.08;
        float preAnalysisTimeBudgetSeconds = 20.0f;

        double heterogeneityActivationThreshold = 0.68;
        double heterogeneityNormalizationSpan = 1.75;
        double localDeltaMin = 0.12;
        double localDeltaMax = 0.22;
        double nSigmaFloor = 0.22;

        // 2C-3: BORDERLINE remains OFF by default; optional experimental gate.
        bool enableBorderlineExperimental = false;
        double borderlineRiskScoreThreshold = 0.55;
        double borderlineDeltaScale = 0.60;

        // Short pass 2: additional conservative guards against scan-level over-removal.
        double highRiskMaxTargetRemovalRatio = 0.08;
        double highRiskSparseSeverityConservativeThreshold = 0.85;
        double highRiskConservativeDeltaCap = 0.05;
        double highRiskAdaptiveScaleBypassThreshold = 0.70;
    };

    struct SofHighRiskExecutionGuard
    {
        bool fallbackToParentScan = false;
        bool fallbackDueToSparsePlanning = false;
        bool fallbackDueToTimeBudget = false;
        bool fallbackDueToRepeatedInstability = false;
        double sparseSeverity = 0.0;
    };

    SofHighRiskExecutionGuard buildHighRiskExecutionGuard(const SofPreAnalysisStats& preAnalysis, float preAnalysisSeconds, const Sof2CCalibration& calibration)
    {
        SofHighRiskExecutionGuard guard;

        // 2B.B-2: scan-level guardrails before full sub-box execution is enabled.
        if (preAnalysis.subdivisionShadowSubBoxCount > calibration.maxSubBoxes)
            guard.fallbackDueToSparsePlanning = true;

        const bool lowEstimatedPoints = preAnalysis.subdivisionShadowEstimatedPointsPerSubBox < calibration.minEstimatedPointsPerSubBox;
        const bool lowDenseRatio = preAnalysis.subdivisionShadowEstimatedDenseSubBoxRatio < calibration.minDenseRatio;
        const double fallbackSubBoxRatio = preAnalysis.subdivisionShadowSubBoxCount > 0
            ? static_cast<double>(preAnalysis.subdivisionShadowFallbackSubBoxCount) / static_cast<double>(preAnalysis.subdivisionShadowSubBoxCount)
            : 0.0;

        // 2C-2: avoid binary OR-only behavior; fallback only if sparse signals are materially combined.
        guard.sparseSeverity = 0.0;
        if (lowEstimatedPoints)
            guard.sparseSeverity += 0.45;
        if (lowDenseRatio)
            guard.sparseSeverity += 0.35;
        if (fallbackSubBoxRatio > 0.0)
            guard.sparseSeverity += std::clamp(fallbackSubBoxRatio, 0.0, 1.0) * 0.40;

        if ((lowEstimatedPoints && lowDenseRatio) || fallbackSubBoxRatio >= 0.50 || guard.sparseSeverity >= 0.75)
            guard.fallbackDueToSparsePlanning = true;

        if (preAnalysisSeconds > calibration.preAnalysisTimeBudgetSeconds)
            guard.fallbackDueToTimeBudget = true;

        guard.fallbackToParentScan =
            guard.fallbackDueToSparsePlanning ||
            guard.fallbackDueToTimeBudget ||
            guard.fallbackDueToRepeatedInstability;

        return guard;
    }

    struct SofHighRiskAggressiveness
    {
        bool active = false;
        double delta = 0.0;
        double heterogeneityScore = 0.0;
    };

    SofHighRiskAggressiveness buildHighRiskAggressiveness(const SofPreAnalysisStats& preAnalysis, bool enableHighRiskExecution, bool fallbackToParentScan, const Sof2CCalibration& calibration)
    {
        SofHighRiskAggressiveness result;
        if (!enableHighRiskExecution || fallbackToParentScan)
            return result;

        // 2B.B-3: activate local aggressiveness only for heterogeneous HIGH scans.
        const double heterogeneityScore = std::clamp(0.5 * preAnalysis.spacingCv + 0.5 * preAnalysis.meanDistanceCv, 0.0, 4.0);
        const bool heterogeneous = heterogeneityScore >= calibration.heterogeneityActivationThreshold;
        if (!heterogeneous)
            return result;

        result.active = true;
        result.heterogeneityScore = heterogeneityScore;

        // Bounded dynamic delta to avoid global over-hardening.
        const double normalized = std::clamp((heterogeneityScore - calibration.heterogeneityActivationThreshold) / calibration.heterogeneityNormalizationSpan, 0.0, 1.0);
        result.delta = std::clamp(calibration.localDeltaMin + normalized * (calibration.localDeltaMax - calibration.localDeltaMin), calibration.localDeltaMin, calibration.localDeltaMax);
        return result;
    }



    bool isExperimentalBorderlineEnabled(const SofPreAnalysisStats& preAnalysis, const Sof2CCalibration& calibration)
    {
        // 2C-3 experimental policy: keep BORDERLINE off by default.
        if (!calibration.enableBorderlineExperimental)
            return false;

        if (preAnalysis.riskClass != SofPreAnalysisRiskClass::Borderline)
            return false;

        return preAnalysis.riskScore >= calibration.borderlineRiskScoreThreshold;
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

    const Sof2CCalibration sof2CCalibration;

    uint64_t scan_count = 0;
    uint64_t total_deleted_points = 0;
    uint64_t preAnalysisLowCount = 0;
    uint64_t preAnalysisBorderlineCount = 0;
    uint64_t preAnalysisHighCount = 0;
    uint64_t preAnalysisBorderlineExperimentalEligibleCount = 0;
    uint64_t subdivisionShadowEnabledCount = 0;
    uint64_t subdivisionShadowFallbackCount = 0;
    double preAnalysisRiskScoreSum = 0.0;
    uint64_t highRiskInstabilityFallbackCount = 0;
    uint64_t highRiskAggressivenessActiveCount = 0;
    double highRiskAggressivenessDeltaSum = 0.0;
    double highRiskAdaptiveDeltaScale = 1.0;
    uint64_t highRiskAdaptiveBackoffTriggerCount = 0;
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
        QString qScanName = QString::fromStdWString(wScan->getName());

        // Pré-analyse centralisée dans un helper dédié pour éviter les duplications de déclarations
        // lors de merges/cherry-picks sur ce bloc de code.
        const SofPreAnalysisRunResult preAnalysisRun = runSofPreAnalysis(
            old_guid, wScan, *clippingToUse, m_kNeighbors, m_samplingPercent, m_beta);
        SofPreAnalysisStats preAnalysis = preAnalysisRun.stats;
        SofSubdivisionShadowConfig subdivisionShadow = buildSubdivisionShadowConfig(preAnalysis);
        preAnalysis.subdivisionShadowEnabled = subdivisionShadow.enabled;
        preAnalysis.subdivisionShadowFactor = subdivisionShadow.factor;
        preAnalysis.subdivisionShadowSubBoxCount = subdivisionShadow.subBoxCount;
        preAnalysis.subdivisionShadowHalo = subdivisionShadow.haloMeters;
        preAnalysis.subdivisionShadowFallbackSubBoxCount = subdivisionShadow.fallbackSubBoxCount;
        const SofSubdivisionSpatialDiagnostics spatialDiag = buildSubdivisionSpatialDiagnostics(preAnalysis, subdivisionShadow);
        preAnalysis.subdivisionShadowHaloMediumMeters = spatialDiag.haloMediumMeters;
        preAnalysis.subdivisionShadowHaloStrongMeters = spatialDiag.haloStrongMeters;
        preAnalysis.subdivisionShadowEstimatedPointsPerSubBox = spatialDiag.estimatedPointsPerSubBox;
        preAnalysis.subdivisionShadowEstimatedDenseSubBoxRatio = spatialDiag.estimatedDenseSubBoxRatio;
        const float preAnalysisSeconds = preAnalysisRun.durationSeconds;
        SofHighRiskExecutionGuard highRiskGuard = buildHighRiskExecutionGuard(preAnalysis, preAnalysisSeconds, sof2CCalibration);
        SofHighRiskAggressiveness highRiskAggressiveness = buildHighRiskAggressiveness(
            preAnalysis,
            preAnalysis.riskClass == SofPreAnalysisRiskClass::High,
            highRiskGuard.fallbackToParentScan,
            sof2CCalibration);
        const bool borderlineExperimentalEnabled = isExperimentalBorderlineEnabled(preAnalysis, sof2CCalibration);
        if (borderlineExperimentalEnabled)
            ++preAnalysisBorderlineExperimentalEligibleCount;

        preAnalysisRiskScoreSum += preAnalysis.riskScore;
        if (preAnalysis.subdivisionShadowEnabled)
        {
            ++subdivisionShadowEnabledCount;
            if (preAnalysis.subdivisionShadowFallbackSubBoxCount > 0)
                ++subdivisionShadowFallbackCount;
        }
        switch (preAnalysis.riskClass)
        {
        case SofPreAnalysisRiskClass::Low:
            ++preAnalysisLowCount;
            break;
        case SofPreAnalysisRiskClass::Borderline:
            ++preAnalysisBorderlineCount;
            break;
        case SofPreAnalysisRiskClass::High:
            ++preAnalysisHighCount;
            break;
        default:
            break;
        }

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
        const bool forceLocalStatsForHighRisk = preAnalysis.subdivisionShadowEnabled;
        const bool enableHighRiskSubdivisionExecution = preAnalysis.riskClass == SofPreAnalysisRiskClass::High;
        const bool enableBorderlineExperimentalExecution = borderlineExperimentalEnabled;
        if (!m_globalFiltering || forceLocalStatsForHighRisk)
        {
            auto statsProgress = makeProgressCallback(scan_count, 0, 50);
            TlScanOverseer::getInstance().computeOutlierStats(old_guid, (TransformationModule)*&wScan, *clippingToUse, m_kNeighbors, m_samplingPercent, m_beta, statsToUse, statsProgress);

            // Passe 2B: fallback de sécurité vers les stats globales si l'échantillon local est insuffisant.
            const uint64_t minLocalSampleCount = 500;
            if (forceLocalStatsForHighRisk && statsToUse.count < minLocalSampleCount)
            {
                uint64_t localSampleCount = statsToUse.count;
                statsToUse = globalStats;
                controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(
                    QString("SOF fallback to global stats for scan %1 (local samples=%2 < %3)")
                        .arg(qScanName)
                        .arg(localSampleCount)
                        .arg(minLocalSampleCount)));
            }
        }

        // 2B.B-1 execution gate (HIGH only): keep the classic pipeline for LOW/BORDERLINE.
        // NOTE: full spatial subdivision (core/work halo ownership) is introduced incrementally in next passes.
        double effectiveNSigma = m_nSigma;
        if (enableHighRiskSubdivisionExecution && !highRiskGuard.fallbackToParentScan)
        {
            // 2B.B-3: apply bounded local aggressiveness only on heterogeneous HIGH-risk scans.
            if (highRiskAggressiveness.active)
            {
                double adaptedDelta = highRiskAggressiveness.delta * highRiskAdaptiveDeltaScale;

                // Short pass 2: bypass hardening when adaptive scale is already in deep backoff.
                if (highRiskAdaptiveDeltaScale < sof2CCalibration.highRiskAdaptiveScaleBypassThreshold)
                    adaptedDelta = 0.0;

                // Short pass 2: conservative cap for very sparse/unstable planning contexts.
                if (highRiskGuard.sparseSeverity >= sof2CCalibration.highRiskSparseSeverityConservativeThreshold)
                    adaptedDelta = std::min(adaptedDelta, sof2CCalibration.highRiskConservativeDeltaCap);

                effectiveNSigma = std::max(sof2CCalibration.nSigmaFloor, m_nSigma - adaptedDelta);
                ++highRiskAggressivenessActiveCount;
                highRiskAggressivenessDeltaSum += adaptedDelta;
            }
        }
        else if (enableHighRiskSubdivisionExecution && highRiskGuard.fallbackToParentScan)
        {
            ++highRiskInstabilityFallbackCount;
        }

        if (enableBorderlineExperimentalExecution)
        {
            // 2C-3 experimental branch: lighter-than-HIGH local hardening for near-HIGH borderline scans.
            const double highLikeDelta = std::clamp(highRiskAggressiveness.delta, sof2CCalibration.localDeltaMin, sof2CCalibration.localDeltaMax);
            const double borderlineDelta = highLikeDelta * sof2CCalibration.borderlineDeltaScale;
            effectiveNSigma = std::max(sof2CCalibration.nSigmaFloor, m_nSigma - borderlineDelta);
        }

        auto filterProgress = makeProgressCallback(scan_count, (m_globalFiltering && !forceLocalStatsForHighRisk) ? 0 : 50, (m_globalFiltering && !forceLocalStatsForHighRisk) ? 100 : 50);
        bool res = TlScanOverseer::getInstance().filterOutliersAndWrite(old_guid, (TransformationModule)*&wScan, *clippingToUse, m_kNeighbors, statsToUse, effectiveNSigma, m_beta, scan_writer, deleted_point_count, filterProgress);
        res &= scan_writer->finalizePointCloud();
        delete scan_writer;

        total_deleted_points += deleted_point_count;

        double currentRemovalRatio = initial_point_count > 0
            ? static_cast<double>(deleted_point_count) / static_cast<double>(initial_point_count)
            : 0.0;
        if (enableHighRiskSubdivisionExecution && currentRemovalRatio > sof2CCalibration.highRiskMaxTargetRemovalRatio)
        {
            // Short pass 2: aggressively increase backoff pressure after an over-removal event.
            highRiskAdaptiveDeltaScale = std::clamp(highRiskAdaptiveDeltaScale - 0.20, 0.50, 1.00);
            ++highRiskAdaptiveBackoffTriggerCount;
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(
                QString("SOF high-risk over-removal guard triggered for scan %1 (removalRatio=%2 > target=%3)")
                    .arg(qScanName)
                    .arg(currentRemovalRatio, 0, 'f', 4)
                    .arg(sof2CCalibration.highRiskMaxTargetRemovalRatio, 0, 'f', 4)));
        }

        scan_count++;
        float seconds = std::chrono::duration<float, std::ratio<1>>(std::chrono::steady_clock::now() - startTime).count();
        updateProgress(scan_count, 100, scan_count * 100);

        if (deleted_point_count > 0)
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("%1 points deleted in scan %2 in %3 seconds.").arg(deleted_point_count).arg(qScanName).arg(seconds)));
        else
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Scan %1 not affected by outlier filter.").arg(qScanName)));

        QString preAnalysisAction = preAnalysis.recommendedAction == SofPreAnalysisAction::SubdivisionRecommended
            ? "SUBDIVISION_RECOMMENDED"
            : "NO_SUBDIVISION";
        controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(
            QString("SOF pre-analysis %1: risk=%2, action=%3, occupiedCells=%4, testedPoints=%5, time=%6s, subdivisionShadow=%7 factor=%8 subBoxes=%9 halo=%10 fallbackSubBoxes=%11 haloMedium=%12 haloStrong=%13 estPtsPerSubBox=%14 estDenseSubBoxRatio=%15")
                .arg(qScanName)
                .arg(preAnalysis.riskScore, 0, 'f', 3)
                .arg(preAnalysisAction)
                .arg(preAnalysis.occupiedCells)
                .arg(preAnalysis.testedPointsEstimate)
                .arg(preAnalysisSeconds, 0, 'f', 3)
                .arg(preAnalysis.subdivisionShadowEnabled ? "ON" : "OFF")
                .arg(preAnalysis.subdivisionShadowFactor)
                .arg(preAnalysis.subdivisionShadowSubBoxCount)
                .arg(preAnalysis.subdivisionShadowHalo, 0, 'f', 3)
                .arg(preAnalysis.subdivisionShadowFallbackSubBoxCount)
                .arg(preAnalysis.subdivisionShadowHaloMediumMeters, 0, 'f', 3)
                .arg(preAnalysis.subdivisionShadowHaloStrongMeters, 0, 'f', 3)
                .arg(preAnalysis.subdivisionShadowEstimatedPointsPerSubBox, 0, 'f', 1)
                .arg(preAnalysis.subdivisionShadowEstimatedDenseSubBoxRatio, 0, 'f', 3)));

        if (enableHighRiskSubdivisionExecution)
        {
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(
                QString("SOF high-risk execution gate for scan %1 (effectiveNSigma=%2, fallbackParent=%3, sparsePlan=%4, timeBudget=%5, localAggressive=%6, delta=%7, heterogeneity=%8)")
                    .arg(qScanName)
                    .arg(effectiveNSigma, 0, 'f', 3)
                    .arg(highRiskGuard.fallbackToParentScan ? "YES" : "NO")
                    .arg(highRiskGuard.fallbackDueToSparsePlanning ? "YES" : "NO")
                    .arg(highRiskGuard.fallbackDueToTimeBudget ? "YES" : "NO")
                    .arg(highRiskAggressiveness.active ? "YES" : "NO")
                    .arg(highRiskAggressiveness.delta, 0, 'f', 3)
                    .arg(highRiskAggressiveness.heterogeneityScore, 0, 'f', 3)));
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(
                QString("SOF high-risk diagnostics for scan %1 (adaptiveDeltaScale=%2, sparseSeverity=%3)")
                    .arg(qScanName)
                    .arg(highRiskAdaptiveDeltaScale, 0, 'f', 3)
                    .arg(highRiskGuard.sparseSeverity, 0, 'f', 3)));
        }


        if (enableHighRiskSubdivisionExecution)
        {
            // 2C-2: adaptive safe-backoff if fallback happened and removal is still weak.
            const double removalRatio = currentRemovalRatio;
            if (highRiskGuard.fallbackToParentScan && removalRatio < 0.004)
            {
                // 2C short pass 1: faster backoff to protect sparse/remote zones from over-removal.
                highRiskAdaptiveDeltaScale = std::clamp(highRiskAdaptiveDeltaScale - 0.15, 0.55, 1.00);
                ++highRiskAdaptiveBackoffTriggerCount;
            }
            else if (!highRiskGuard.fallbackToParentScan && removalRatio > 0.015)
            {
                // Recovery is intentionally slower than backoff to keep conservative behavior sticky.
                highRiskAdaptiveDeltaScale = std::clamp(highRiskAdaptiveDeltaScale + 0.03, 0.55, 1.00);
            }
        }

        if (m_state != ContextState::running)
        {
            wasAborted = true;
            break;
        }
    }

    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Total points deleted: %1").arg(total_deleted_points)));
    double averageRisk = scan_count > 0 ? preAnalysisRiskScoreSum / static_cast<double>(scan_count) : 0.0;
    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("SOF pre-analysis summary: LOW=%1 BORDERLINE=%2 HIGH=%3 avgRisk=%4 subdivisionShadowEnabled=%5 fallbackPlanned=%6")
        .arg(preAnalysisLowCount)
        .arg(preAnalysisBorderlineCount)
        .arg(preAnalysisHighCount)
        .arg(averageRisk, 0, 'f', 3)
        .arg(subdivisionShadowEnabledCount)
        .arg(subdivisionShadowFallbackCount)));
    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("SOF high-risk parent fallback count: %1").arg(highRiskInstabilityFallbackCount)));
    const double avgHighRiskDelta = highRiskAggressivenessActiveCount > 0 ? highRiskAggressivenessDeltaSum / static_cast<double>(highRiskAggressivenessActiveCount) : 0.0;
    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("SOF 2C calibration: minPtsPerSubBox=%1 minDenseRatio=%2 heterogeneityThreshold=%3 deltaRange=[%4,%5] overRemovalTarget=%6 sparseConservativeThreshold=%7")
        .arg(sof2CCalibration.minEstimatedPointsPerSubBox, 0, 'f', 0)
        .arg(sof2CCalibration.minDenseRatio, 0, 'f', 3)
        .arg(sof2CCalibration.heterogeneityActivationThreshold, 0, 'f', 3)
        .arg(sof2CCalibration.localDeltaMin, 0, 'f', 3)
        .arg(sof2CCalibration.localDeltaMax, 0, 'f', 3)
        .arg(sof2CCalibration.highRiskMaxTargetRemovalRatio, 0, 'f', 3)
        .arg(sof2CCalibration.highRiskSparseSeverityConservativeThreshold, 0, 'f', 3)));
    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("SOF borderline experimental mode: enabled=%1 eligibleScans=%2 threshold=%3 deltaScale=%4")
        .arg(sof2CCalibration.enableBorderlineExperimental ? "YES" : "NO")
        .arg(preAnalysisBorderlineExperimentalEligibleCount)
        .arg(sof2CCalibration.borderlineRiskScoreThreshold, 0, 'f', 3)
        .arg(sof2CCalibration.borderlineDeltaScale, 0, 'f', 3)));
    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("SOF high-risk adaptive backoff: scale=%1 triggers=%2")
        .arg(highRiskAdaptiveDeltaScale, 0, 'f', 3)
        .arg(highRiskAdaptiveBackoffTriggerCount)));
    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("SOF high-risk local aggressiveness: active=%1 avgDelta=%2")
        .arg(highRiskAggressivenessActiveCount)
        .arg(avgHighRiskDelta, 0, 'f', 3)));
    controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));

    if (m_openFolderAfterExport && !wasAborted)
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
