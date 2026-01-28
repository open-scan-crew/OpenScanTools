#include "controller/functionSystem/ContextColorBalance.h"

#include "controller/Controller.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/messages/ModalMessage.h"
#include "controller/messages/ColorBalanceMessage.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataIO.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"
#include "gui/texts/ExportTexts.hpp"
#include "gui/texts/SplashScreenTexts.hpp"
#include "io/exports/IScanFileWriter.h"
#include "models/graph/GraphManager.h"
#include "models/graph/PointCloudNode.h"
#include "pointCloudEngine/ColorBalanceTypes.h"
#include "pointCloudEngine/PCE_core.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "utils/Logger.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <unordered_map>
#include <utility>
#include <vector>

// Note (Aurélien) QT::StandardButtons enum values in qmessagebox.h
#define Yes 0x00004000
#define No 0x00010000

namespace
{
    struct ChannelStats
    {
        double mean = 0.0;
        double sigma = 0.0;
        uint64_t count = 0;
    };

    struct CellStats
    {
        ChannelStats color;
        ChannelStats intensity;
        uint64_t count = 0;
        bool validColor = true;
        double confidence = 0.0;
    };

    struct ReferenceSamples
    {
        std::vector<std::pair<double, double>> colorMeans;
        std::vector<std::pair<double, double>> colorSigmas;
        std::vector<std::pair<double, double>> intensityMeans;
        std::vector<std::pair<double, double>> intensitySigmas;
        int contributingScans = 0;
    };

    struct ReferenceValues
    {
        double colorMean = 0.0;
        double colorSigma = 0.0;
        double intensityMean = 0.0;
        double intensitySigma = 0.0;
        bool hasColor = false;
        bool hasIntensity = false;
    };

    struct ScanChannelUsage
    {
        bool useColor = false;
        bool useIntensity = false;
    };

    struct RangeStats
    {
        bool hasValue = false;
        double minValue = 0.0;
        double maxValue = 0.0;

        void update(double value)
        {
            if (!hasValue)
            {
                minValue = value;
                maxValue = value;
                hasValue = true;
                return;
            }
            minValue = std::min(minValue, value);
            maxValue = std::max(maxValue, value);
        }
    };

    struct CorrectionTotals
    {
        double gainL = 0.0;
        double offsetL = 0.0;
        double weightL = 0.0;
        double gainI = 0.0;
        double offsetI = 0.0;
        double weightI = 0.0;
    };

    double lerp(double a, double b, double t)
    {
        return a + (b - a) * t;
    }

    void smoothCorrectionsAcrossScans(std::vector<std::vector<ColorBalanceCorrectionMap>>& corrections, int level, double smoothingStrength)
    {
        if (smoothingStrength <= 0.0 || corrections.empty())
            return;

        std::unordered_map<ColorBalanceCellKey, std::vector<ColorBalanceCellCorrection*>, ColorBalanceCellKeyHasher> cellCorrections;
        for (auto& scanLevels : corrections)
        {
            if (level < 0 || static_cast<size_t>(level) >= scanLevels.size())
                continue;
            for (auto& entry : scanLevels[level])
                cellCorrections[entry.first].push_back(&entry.second);
        }

        std::unordered_map<ColorBalanceCellKey, CorrectionTotals, ColorBalanceCellKeyHasher> totals;
        totals.reserve(cellCorrections.size());
        for (const auto& entry : cellCorrections)
        {
            CorrectionTotals sums;
            for (const ColorBalanceCellCorrection* correction : entry.second)
            {
                if (correction->hasColor && correction->confidenceL > 0.0f)
                {
                    double weight = static_cast<double>(correction->confidenceL);
                    sums.weightL += weight;
                    sums.gainL += correction->gainL * weight;
                    sums.offsetL += correction->offsetL * weight;
                }
                if (correction->hasIntensity && correction->confidenceI > 0.0f)
                {
                    double weight = static_cast<double>(correction->confidenceI);
                    sums.weightI += weight;
                    sums.gainI += correction->gainI * weight;
                    sums.offsetI += correction->offsetI * weight;
                }
            }
            totals[entry.first] = sums;
        }

        for (auto& entry : cellCorrections)
        {
            const auto totalsIt = totals.find(entry.first);
            if (totalsIt == totals.end())
                continue;
            const CorrectionTotals& sums = totalsIt->second;
            double avgGainL = sums.weightL > 0.0 ? (sums.gainL / sums.weightL) : 1.0;
            double avgOffsetL = sums.weightL > 0.0 ? (sums.offsetL / sums.weightL) : 0.0;
            double avgGainI = sums.weightI > 0.0 ? (sums.gainI / sums.weightI) : 1.0;
            double avgOffsetI = sums.weightI > 0.0 ? (sums.offsetI / sums.weightI) : 0.0;

            for (ColorBalanceCellCorrection* correction : entry.second)
            {
                if (correction->hasColor && correction->confidenceL > 0.0f)
                {
                    correction->gainL = lerp(correction->gainL, avgGainL, smoothingStrength);
                    correction->offsetL = lerp(correction->offsetL, avgOffsetL, smoothingStrength);
                }
                if (correction->hasIntensity && correction->confidenceI > 0.0f)
                {
                    correction->gainI = lerp(correction->gainI, avgGainI, smoothingStrength);
                    correction->offsetI = lerp(correction->offsetI, avgOffsetI, smoothingStrength);
                }
            }
        }
    }

    double medianFromHistogram(const std::array<uint32_t, 256>& hist, uint64_t count)
    {
        if (count == 0)
            return 0.0;
        uint64_t mid = (count - 1) / 2;
        uint64_t cumulative = 0;
        for (size_t i = 0; i < hist.size(); ++i)
        {
            cumulative += hist[i];
            if (cumulative > mid)
                return static_cast<double>(i);
        }
        return 0.0;
    }

    double madFromHistogram(const std::array<uint32_t, 256>& hist, uint64_t count, int median)
    {
        if (count == 0)
            return 0.0;
        uint64_t mid = (count - 1) / 2;
        uint64_t cumulative = 0;
        for (int d = 0; d < 256; ++d)
        {
            uint64_t current = 0;
            int low = median - d;
            int high = median + d;
            if (low >= 0)
                current += hist[static_cast<size_t>(low)];
            if (high < 256 && high != low)
                current += hist[static_cast<size_t>(high)];
            cumulative += current;
            if (cumulative > mid)
                return static_cast<double>(d);
        }
        return 0.0;
    }

    ChannelStats computeChannelStats(const std::array<uint32_t, 256>& hist, uint64_t count)
    {
        ChannelStats stats;
        stats.count = count;
        if (count == 0)
            return stats;
        double median = medianFromHistogram(hist, count);
        double mad = madFromHistogram(hist, count, static_cast<int>(std::lround(median)));
        stats.mean = median;
        stats.sigma = mad * 1.4826;
        return stats;
    }

    double weightedMedianFromSamples(std::vector<std::pair<double, double>> samples)
    {
        if (samples.empty())
            return 0.0;
        samples.erase(std::remove_if(samples.begin(), samples.end(), [](const auto& sample)
        {
            return sample.second <= 0.0;
        }), samples.end());
        if (samples.empty())
            return 0.0;
        std::sort(samples.begin(), samples.end(), [](const auto& lhs, const auto& rhs)
        {
            return lhs.first < rhs.first;
        });
        double totalWeight = 0.0;
        for (const auto& sample : samples)
            totalWeight += sample.second;
        if (totalWeight <= 0.0)
            return 0.0;
        double target = totalWeight * 0.5;
        double cumulative = 0.0;
        for (const auto& sample : samples)
        {
            cumulative += sample.second;
            if (cumulative >= target)
                return sample.first;
        }
        return samples.back().first;
    }

    double computeConfidence(uint64_t count, int nMin, int nGood)
    {
        if (count < static_cast<uint64_t>(nMin))
            return 0.0;
        if (nGood <= nMin)
            return 1.0;
        double weight = (static_cast<double>(count) - static_cast<double>(nMin)) / (static_cast<double>(nGood) - static_cast<double>(nMin));
        return std::clamp(weight, 0.0, 1.0);
    }

    void mergeHistogram(ColorBalanceCellHistogram& dst, const ColorBalanceCellHistogram& src)
    {
        dst.count += src.count;
        for (size_t i = 0; i < dst.lumaHist.size(); ++i)
        {
            dst.lumaHist[i] += src.lumaHist[i];
            dst.intensityHist[i] += src.intensityHist[i];
        }
    }

    ColorBalanceHistogramMap buildParentHistogram(const ColorBalanceHistogramMap& child)
    {
        ColorBalanceHistogramMap parent;
        parent.reserve(child.size());
        for (const auto& entry : child)
        {
            ColorBalanceCellKey parentKey = colorBalanceParentCellKey(entry.first, 1);
            ColorBalanceCellHistogram& parentHist = parent[parentKey];
            mergeHistogram(parentHist, entry.second);
        }
        return parent;
    }
}

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

    const auto visibleScans = graphManager.getVisibleScans(m_panoramic);
    if (visibleScans.size() < 2)
    {
        FUNCLOG << "Not enough scans to run color balance" << LOGENDL;
        controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_COLOR_BALANCE_MINIMUM_SCANS));
        return (m_state = ContextState::abort);
    }

    controller.updateInfo(new GuiDataColorBalanceDialogDisplay());

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
        m_photometricDepth = decodedMsg->photometricDepth;
        m_nMin = decodedMsg->nMin;
        m_nGood = decodedMsg->nGood;
        m_beta = decodedMsg->beta;
        m_smoothingLevels = decodedMsg->smoothingLevels;
        m_applyIntensityWhenAvailable = decodedMsg->applyIntensityWhenAvailable;
        m_outputFolder = decodedMsg->outputFolder;
        m_openFolderAfterExport = decodedMsg->openFolderAfterExport;

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
    if (scans.size() < 2)
    {
        controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_COLOR_BALANCE_MINIMUM_SCANS));
        m_state = ContextState::abort;
        return m_state;
    }

    std::vector<SafePtr<PointCloudNode>> scanList;
    scanList.reserve(scans.size());
    for (const SafePtr<PointCloudNode>& scan : scans)
    {
        WritePtr<PointCloudNode> wScan = scan.get();
        if (!wScan)
            continue;
        scanList.push_back(scan);
    }
    std::sort(scanList.begin(), scanList.end(), [](const SafePtr<PointCloudNode>& lhs, const SafePtr<PointCloudNode>& rhs)
    {
        WritePtr<PointCloudNode> wLeft = lhs.get();
        WritePtr<PointCloudNode> wRight = rhs.get();
        return wLeft->getName() < wRight->getName();
    });
    if (scanList.size() < 2)
    {
        controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_COLOR_BALANCE_MINIMUM_SCANS));
        m_state = ContextState::abort;
        return m_state;
    }

    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString()));
    const uint64_t totalScans = scanList.size();
    const uint64_t totalProgressSteps = totalScans * 100;
    controller.updateInfo(new GuiDataProcessingSplashScreenStart(totalProgressSteps, TEXT_EXPORT_COLOR_BALANCE_TITLE_PROGRESS, TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(0).arg(totalScans)));

    ClippingAssembly clippingAssembly;
    graphManager.getClippingAssembly(clippingAssembly, true, false);

    int depth = std::clamp(m_photometricDepth, 1, 15);
    int nMin = std::max(1, m_nMin);
    int nGood = std::max(nMin, m_nGood);
    int smoothingLevels = std::clamp(m_smoothingLevels, 1, 6);
    double beta = std::clamp(m_beta, 0.1, 1.0);

    double rootSize = 0.0;
    std::vector<ScanChannelUsage> scanUsage;
    std::vector<std::wstring> scanNames;
    scanUsage.reserve(scanList.size());
    scanNames.reserve(scanList.size());

    for (const SafePtr<PointCloudNode>& scan : scanList)
    {
        WritePtr<PointCloudNode> wScan = scan.get();
        if (!wScan)
            continue;
        scanNames.push_back(wScan->getName());
        tls::ScanHeader header;
        if (TlScanOverseer::getInstance().getScanHeader(wScan->getScanGuid(), header))
        {
            double sx = static_cast<double>(header.limits.xMax - header.limits.xMin);
            double sy = static_cast<double>(header.limits.yMax - header.limits.yMin);
            double sz = static_cast<double>(header.limits.zMax - header.limits.zMin);
            rootSize = std::max(rootSize, std::max({ sx, sy, sz }));
        }

        bool hasColor = (wScan->getPointFormat() == tls::PointFormat::TL_POINT_XYZ_RGB || wScan->getPointFormat() == tls::PointFormat::TL_POINT_XYZ_I_RGB);
        bool hasIntensity = (wScan->getPointFormat() == tls::PointFormat::TL_POINT_XYZ_I || wScan->getPointFormat() == tls::PointFormat::TL_POINT_XYZ_I_RGB);
        ScanChannelUsage usage;
        usage.useColor = hasColor;
        usage.useIntensity = hasIntensity && (m_applyIntensityWhenAvailable || !hasColor);
        scanUsage.push_back(usage);
    }

    double baseCellSize = rootSize > 0.0 ? (rootSize / std::pow(2.0, depth)) : 1.0;

    std::vector<ColorBalanceHistogramMap> baseHistograms(scanList.size());
    uint64_t scanIndex = 0;
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

    scanIndex = 0;
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

        auto progressCallback = makeProgressCallback(scanIndex, 0, 50);
        TlScanOverseer::getInstance().computeColorBalanceHistogram(wScan->getScanGuid(), (TransformationModule)*&wScan, *clippingToUse, baseCellSize, scanUsage[scanIndex].useColor, scanUsage[scanIndex].useIntensity, baseHistograms[scanIndex], progressCallback);
        scanIndex++;
    }

    std::vector<std::vector<ColorBalanceHistogramMap>> histogramLevels(scanList.size());
    for (size_t i = 0; i < scanList.size(); ++i)
    {
        histogramLevels[i].reserve(smoothingLevels);
        histogramLevels[i].push_back(baseHistograms[i]);
        for (int level = 1; level < smoothingLevels; ++level)
            histogramLevels[i].push_back(buildParentHistogram(histogramLevels[i][level - 1]));
    }

    std::vector<std::vector<ColorBalanceCorrectionMap>> corrections(scanList.size(), std::vector<ColorBalanceCorrectionMap>(smoothingLevels));

    for (int level = 0; level < smoothingLevels; ++level)
    {
        std::vector<std::unordered_map<ColorBalanceCellKey, CellStats, ColorBalanceCellKeyHasher>> statsByScan(scanList.size());
        std::unordered_map<ColorBalanceCellKey, ReferenceSamples, ColorBalanceCellKeyHasher> referenceSamples;
        std::unordered_map<ColorBalanceCellKey, int, ColorBalanceCellKeyHasher> overlapCount;
        std::unordered_map<ColorBalanceCellKey, int, ColorBalanceCellKeyHasher> overlapCountAll;
        std::unordered_map<ColorBalanceCellKey, int, ColorBalanceCellKeyHasher> validColorCount;
        constexpr double kLumaSaturationHigh = 245.0;
        constexpr double kLumaSigmaMin = 2.0;

        for (size_t s = 0; s < scanList.size(); ++s)
        {
            const ColorBalanceHistogramMap& histMap = histogramLevels[s][level];
            statsByScan[s].reserve(histMap.size());
            for (const auto& entry : histMap)
            {
                CellStats cellStats;
                cellStats.count = entry.second.count;
                if (scanUsage[s].useColor)
                    cellStats.color = computeChannelStats(entry.second.lumaHist, entry.second.count);
                if (scanUsage[s].useIntensity)
                    cellStats.intensity = computeChannelStats(entry.second.intensityHist, entry.second.count);

                cellStats.confidence = computeConfidence(entry.second.count, nMin, nGood);
                if (scanUsage[s].useColor)
                    cellStats.validColor = !(cellStats.color.mean > kLumaSaturationHigh && cellStats.color.sigma < kLumaSigmaMin);

                statsByScan[s][entry.first] = cellStats;
                overlapCountAll[entry.first] += 1;

                double weightRef = cellStats.confidence * (cellStats.validColor ? 1.0 : 0.0);
                if (weightRef > 0.0)
                {
                    ReferenceSamples& samples = referenceSamples[entry.first];
                    if (scanUsage[s].useColor)
                    {
                        samples.colorMeans.emplace_back(cellStats.color.mean, weightRef);
                        samples.colorSigmas.emplace_back(cellStats.color.sigma, weightRef);
                    }
                    if (scanUsage[s].useIntensity)
                    {
                        samples.intensityMeans.emplace_back(cellStats.intensity.mean, weightRef);
                        samples.intensitySigmas.emplace_back(cellStats.intensity.sigma, weightRef);
                    }
                    samples.contributingScans += 1;
                    overlapCount[entry.first] += 1;
                    if (scanUsage[s].useColor)
                        validColorCount[entry.first] += 1;
                }
            }
        }

        std::unordered_map<ColorBalanceCellKey, ReferenceValues, ColorBalanceCellKeyHasher> referenceValues;
        referenceValues.reserve(referenceSamples.size());
        for (auto& entry : referenceSamples)
        {
            ReferenceValues values;
            if (!entry.second.colorMeans.empty() && entry.second.contributingScans >= 2)
            {
                values.colorMean = weightedMedianFromSamples(entry.second.colorMeans);
                values.colorSigma = weightedMedianFromSamples(entry.second.colorSigmas);
                values.hasColor = true;
            }
            if (!entry.second.intensityMeans.empty() && entry.second.contributingScans >= 2)
            {
                values.intensityMean = weightedMedianFromSamples(entry.second.intensityMeans);
                values.intensitySigma = weightedMedianFromSamples(entry.second.intensitySigmas);
                values.hasIntensity = true;
            }
            referenceValues[entry.first] = values;
        }

        for (size_t s = 0; s < scanList.size(); ++s)
        {
            ColorBalanceCorrectionMap& correctionMap = corrections[s][level];
            correctionMap.reserve(statsByScan[s].size());
            for (const auto& entry : statsByScan[s])
            {
                const CellStats& stats = entry.second;
                auto refIt = referenceValues.find(entry.first);
                if (refIt == referenceValues.end())
                    continue;
                const ReferenceValues& ref = refIt->second;
                if (overlapCount[entry.first] < 2)
                    continue;
                double confidence = stats.confidence * (stats.validColor ? 1.0 : 0.0);
                if (confidence <= 0.0)
                    continue;
                ColorBalanceCellCorrection correction;
                if (scanUsage[s].useColor && ref.hasColor)
                {
                    double gain = (stats.color.sigma > 0.0 && ref.colorSigma > 0.0) ? (ref.colorSigma / stats.color.sigma) : 1.0;
                    double offset = ref.colorMean - gain * stats.color.mean;
                    correction.gainL = gain;
                    correction.offsetL = offset;
                    correction.confidenceL = static_cast<float>(confidence);
                    correction.hasColor = true;
                }
                if (scanUsage[s].useIntensity && ref.hasIntensity)
                {
                    double gain = (stats.intensity.sigma > 0.0 && ref.intensitySigma > 0.0) ? (ref.intensitySigma / stats.intensity.sigma) : 1.0;
                    double offset = ref.intensityMean - gain * stats.intensity.mean;
                    correction.gainI = gain;
                    correction.offsetI = offset;
                    correction.confidenceI = static_cast<float>(confidence);
                    correction.hasIntensity = true;
                }
                if (correction.hasColor || correction.hasIntensity)
                    correctionMap[entry.first] = correction;
            }
        }

        size_t overlapCells = 0;
        size_t validCells = 0;
        for (const auto& entry : overlapCountAll)
        {
            if (entry.second < 2)
                continue;
            overlapCells += 1;
            auto validIt = validColorCount.find(entry.first);
            if (validIt != validColorCount.end() && validIt->second >= 2)
                validCells += 1;
        }
        double validRatio = overlapCells > 0 ? static_cast<double>(validCells) / static_cast<double>(overlapCells) : 0.0;
        double smoothingStrength = std::clamp(1.0 - validRatio, 0.0, 0.75);
        if (smoothingStrength > 0.0)
        {
            Logger::log(LoggerMode::FunctionLog)
                << "Color balance graph smoothing (level " << level << "): strength=" << smoothingStrength << Logger::endl;
            smoothCorrectionsAcrossScans(corrections, level, smoothingStrength);
        }
        Logger::log(LoggerMode::FunctionLog)
            << "Color balance diagnostics (level " << level << "): overlap cells=" << overlapCells
            << ", valid cells=" << validCells << ", ratio=" << validRatio << Logger::endl;

        for (size_t s = 0; s < scanList.size(); ++s)
        {
            RangeStats gainL;
            RangeStats offsetL;
            RangeStats gainI;
            RangeStats offsetI;
            for (const auto& entry : corrections[s][level])
            {
                const ColorBalanceCellCorrection& correction = entry.second;
                if (correction.hasColor)
                {
                    gainL.update(correction.gainL);
                    offsetL.update(correction.offsetL);
                }
                if (correction.hasIntensity)
                {
                    gainI.update(correction.gainI);
                    offsetI.update(correction.offsetI);
                }
            }

            SubLogger& logger = Logger::log(LoggerMode::FunctionLog);
            logger << "Color balance diagnostics (level " << level << ") scan " << scanNames[s] << ": ";
            if (gainL.hasValue)
            {
                logger << "color gain[" << gainL.minValue << "," << gainL.maxValue << "]"
                       << " offset[" << offsetL.minValue << "," << offsetL.maxValue << "]";
            }
            else
            {
                logger << "color gain/offset N/A";
            }
            logger << "; ";
            if (gainI.hasValue)
            {
                logger << "intensity gain[" << gainI.minValue << "," << gainI.maxValue << "]"
                       << " offset[" << offsetI.minValue << "," << offsetI.maxValue << "]";
            }
            else
            {
                logger << "intensity gain/offset N/A";
            }
            logger << Logger::endl;
        }
    }

    scanIndex = 0;
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

        std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

        IScanFileWriter* scanWriter = nullptr;
        std::wstring log;
        std::wstring outputName = wScan->getName() + L"_CB";
        if (!getScanFileWriter(m_outputFolder, outputName, FileType::TLS, log, &scanWriter) || scanWriter == nullptr)
            continue;

        tls::ScanHeader header;
        TlScanOverseer::getInstance().getScanHeader(wScan->getScanGuid(), header);
        header.guid = xg::newGuid();
        scanWriter->appendPointCloud(header, wScan->getTransformation());

        auto progressCallback = makeProgressCallback(scanIndex, 50, 50);
        TlScanOverseer::getInstance().applyColorBalanceAndWrite(wScan->getScanGuid(), (TransformationModule)*&wScan, *clippingToUse, corrections[scanIndex], baseCellSize, beta, scanUsage[scanIndex].useColor, scanUsage[scanIndex].useIntensity, scanWriter, progressCallback);
        scanWriter->finalizePointCloud();
        delete scanWriter;

        scanIndex++;
        float seconds = std::chrono::duration<float, std::ratio<1>>(std::chrono::steady_clock::now() - startTime).count();
        QString qScanName = QString::fromStdWString(wScan->getName());
        updateProgress(scanIndex, 99, scanIndex * 100 - 1);
        controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Color balance applied to scan %1 in %2 seconds.").arg(qScanName).arg(seconds)));
    }

    controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));

    if (m_openFolderAfterExport)
        controller.updateInfo(new GuiDataOpenInExplorer(m_outputFolder));

    m_state = ContextState::done;
    return m_state;
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
