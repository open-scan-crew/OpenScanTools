#include "controller/functionSystem/ContextColorBalance.h"

#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/functionSystem/FunctionManager.h"
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

#include <glm/glm.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <limits>

// Note (Aurélien) QT::StandardButtons enum values in qmessagebox.h
#define Yes 0x00004000
#define No 0x00010000

namespace
{
    bool scanHasRGB(tls::PointFormat format)
    {
        return format == tls::PointFormat::TL_POINT_XYZ_RGB ||
            format == tls::PointFormat::TL_POINT_XYZ_I_RGB;
    }

    bool scanHasIntensity(tls::PointFormat format)
    {
        return format == tls::PointFormat::TL_POINT_XYZ_I ||
            format == tls::PointFormat::TL_POINT_XYZ_I_RGB;
    }

    double estimateScanSpacing(const tls::ScanHeader& header)
    {
        double dx = static_cast<double>(header.limits.xMax - header.limits.xMin);
        double dy = static_cast<double>(header.limits.yMax - header.limits.yMin);
        double dz = static_cast<double>(header.limits.zMax - header.limits.zMin);
        double volume = std::max(0.0, dx * dy * dz);
        if (volume <= 0.0 || header.pointCount == 0)
            return 0.0;
        return std::cbrt(volume / static_cast<double>(header.pointCount));
    }

    std::array<glm::dvec3, 8> makeBoxCorners(const tls::Limits& limits)
    {
        return {
            glm::dvec3(limits.xMin, limits.yMin, limits.zMin),
            glm::dvec3(limits.xMin, limits.yMin, limits.zMax),
            glm::dvec3(limits.xMin, limits.yMax, limits.zMin),
            glm::dvec3(limits.xMin, limits.yMax, limits.zMax),
            glm::dvec3(limits.xMax, limits.yMin, limits.zMin),
            glm::dvec3(limits.xMax, limits.yMin, limits.zMax),
            glm::dvec3(limits.xMax, limits.yMax, limits.zMin),
            glm::dvec3(limits.xMax, limits.yMax, limits.zMax)
        };
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

    std::unordered_set<SafePtr<PointCloudNode>> scans = graphManager.getVisibleScans(m_panoramic);
    if (scans.empty())
    {
        FUNCLOG << "No Scans visibles to balance" << LOGENDL;
        controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_NO_SCAN_SELECTED));
        return (m_state = ContextState::abort);
    }

    bool hasRGB = false;
    bool hasIntensity = false;
    bool hasBoth = false;

    for (const SafePtr<PointCloudNode>& scan : scans)
    {
        ReadPtr<PointCloudNode> rScan = scan.cget();
        if (!rScan)
            continue;
        tls::ScanHeader header;
        if (!TlScanOverseer::getInstance().getScanHeader(rScan->getScanGuid(), header))
            continue;

        bool scanRgb = scanHasRGB(header.format);
        bool scanIntensity = scanHasIntensity(header.format);
        hasRGB |= scanRgb;
        hasIntensity |= scanIntensity;
        hasBoth |= scanRgb && scanIntensity;
    }

    controller.updateInfo(new GuiDataColorBalanceDialogDisplay(hasRGB, hasIntensity, hasBoth));

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
    case IMessage::MessageType::COLOR_BALANCE_FILTER_PARAMETERS:
    {
        auto decodedMsg = static_cast<ColorBalanceMessage*>(message);
        m_strength = decodedMsg->strength;
        m_mode = decodedMsg->mode;
        m_applyOnRGBAndIntensity = decodedMsg->applyOnRGBAndIntensity;
        m_outputFolder = decodedMsg->outputFolder;
        m_openFolderAfterExport = decodedMsg->openFolderAfterExport;

        m_warningModal = true;
        controller.updateInfo(new GuiDataModal(Yes | No, TEXT_COLOR_BALANCE_FILTER_QUESTION));
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
    const uint64_t totalScans = scans.size();
    const bool globalBalance = m_mode == ColorBalanceMode::Global;
    const uint64_t buildSteps = globalBalance ? totalScans * 100 : 0;
    const uint64_t filterSteps = totalScans * 100;
    const uint64_t totalProgressSteps = buildSteps + filterSteps;

    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString()));
    controller.updateInfo(new GuiDataProcessingSplashScreenStart(totalProgressSteps, TEXT_EXPORT_COLOR_BALANCE_TITLE_PROGESS, TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(0).arg(totalScans)));

    ClippingAssembly clippingAssembly;
    graphManager.getClippingAssembly(clippingAssembly, true, false);

    auto updateProgress = [&](uint64_t scansDone, int percent, uint64_t progressValue)
    {
        QString state = QString("%1 - %2%")
                            .arg(TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(scansDone).arg(totalScans))
                            .arg(percent);
        controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(state, progressValue));
    };

    auto makeProgressCallback = [&](uint64_t scanIndex, uint64_t progressOffset, int basePercent, int spanPercent)
    {
        return [scanIndex, progressOffset, basePercent, spanPercent, &updateProgress, totalScans](size_t processed, size_t total)
        {
            if (total == 0)
                return;
            int percent = basePercent + static_cast<int>((processed * spanPercent) / total);
            percent = std::clamp(percent, basePercent, basePercent + spanPercent - 1);
            if (percent >= 100)
                percent = 99;
            uint64_t progressValue = progressOffset + scanIndex * 100 + static_cast<uint64_t>(percent);
            updateProgress(scanIndex + 1, percent, progressValue);
        };
    };

    ColorBalanceSettings baseSettings = buildSettings();
    ColorBalanceGlobalGrid globalGrid;
    double globalSpacing = 0.0;

    if (globalBalance)
    {
        glm::dvec3 minBound(std::numeric_limits<double>::max());
        double spacingSum = 0.0;
        uint64_t spacingCount = 0;

        for (const SafePtr<PointCloudNode>& scan : scans)
        {
            ReadPtr<PointCloudNode> rScan = scan.cget();
            if (!rScan)
                continue;
            tls::ScanHeader header;
            if (!TlScanOverseer::getInstance().getScanHeader(rScan->getScanGuid(), header))
                continue;

            double spacing = estimateScanSpacing(header);
            if (spacing > 0.0)
            {
                spacingSum += spacing;
                spacingCount++;
            }

            glm::dmat4 modelMat = rScan->getTransformation();
            for (const glm::dvec3& corner : makeBoxCorners(header.limits))
            {
                glm::dvec4 globalCorner = modelMat * glm::dvec4(corner, 1.0);
                minBound = glm::min(minBound, glm::dvec3(globalCorner));
            }
        }

        if (spacingCount > 0)
            globalSpacing = spacingSum / static_cast<double>(spacingCount);
        if (globalSpacing <= 0.0)
            globalSpacing = 0.01;

        if (minBound.x == std::numeric_limits<double>::max())
            globalGrid.origin = glm::dvec3(0.0);
        else
            globalGrid.origin = minBound;
        globalGrid.voxelSize = std::max(globalSpacing * 2.0, 0.001);
        globalGrid.maxSamplesPerVoxel = std::max<size_t>(32, baseSettings.neighborCount * 2);

        uint64_t scanIndex = 0;
        for (const SafePtr<PointCloudNode>& scan : scans)
        {
            WritePtr<PointCloudNode> wScan = scan.get();
            if (!wScan)
            {
                scanIndex++;
                continue;
            }

            const ClippingAssembly* clippingToUse = &clippingAssembly;
            ClippingAssembly resolvedAssembly;
            if (clippingAssembly.hasPhaseClipping())
            {
                resolvedAssembly = clippingAssembly.resolveByPhase(wScan->getPhase());
                clippingToUse = &resolvedAssembly;
            }

            auto buildProgress = makeProgressCallback(scanIndex, 0, 0, 100);
            TlScanOverseer::getInstance().collectColorBalanceSamples(wScan->getScanGuid(), (TransformationModule)*&wScan, *clippingToUse, globalGrid.origin, globalGrid.voxelSize, globalGrid.maxSamplesPerVoxel, globalGrid.samples, buildProgress);
            scanIndex++;
        }
    }

    uint64_t scanCount = 0;
    for (const SafePtr<PointCloudNode>& scan : scans)
    {
        WritePtr<PointCloudNode> wScan = scan.get();
        if (!wScan)
        {
            updateProgress(scanCount + 1, 100, buildSteps + (scanCount + 1) * 100);
            scanCount++;
            continue;
        }

        const ClippingAssembly* clippingToUse = &clippingAssembly;
        ClippingAssembly resolvedAssembly;
        if (clippingAssembly.hasPhaseClipping())
        {
            resolvedAssembly = clippingAssembly.resolveByPhase(wScan->getPhase());
            clippingToUse = &resolvedAssembly;
        }

        tls::ScanGuid oldGuid = wScan->getScanGuid();
        tls::ScanHeader header;
        TlScanOverseer::getInstance().getScanHeader(oldGuid, header);

        ColorBalanceSettings scanSettings = baseSettings;
        bool hasRGB = scanHasRGB(header.format);
        bool hasIntensity = scanHasIntensity(header.format);
        scanSettings.applyRGB = hasRGB;
        scanSettings.applyIntensity = hasIntensity;
        if (hasRGB && hasIntensity && !m_applyOnRGBAndIntensity)
            scanSettings.applyIntensity = false;

        TlsFileWriter* tls_writer = nullptr;
        std::wstring log;
        std::wstring outputName = wScan->getName() + L"_CB";
        TlsFileWriter::getWriter(m_outputFolder, outputName, log, (IScanFileWriter**)&tls_writer);
        if (tls_writer == nullptr)
            continue;

        header.guid = xg::newGuid();
        tls_writer->appendPointCloud(header, wScan->getTransformation());

        double scanSpacing = estimateScanSpacing(header);
        double globalRadius = std::max(globalGrid.voxelSize, scanSpacing * scanSettings.beta);
        auto filterProgress = makeProgressCallback(scanCount, buildSteps, 0, 100);

        std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
        bool res = TlScanOverseer::getInstance().balanceColorsAndWrite(oldGuid, (TransformationModule)*&wScan, *clippingToUse, scanSettings, tls_writer, globalBalance ? &globalGrid : nullptr, globalRadius, filterProgress);
        res &= tls_writer->finalizePointCloud();
        delete tls_writer;

        scanCount++;
        float seconds = std::chrono::duration<float, std::ratio<1>>(std::chrono::steady_clock::now() - startTime).count();
        QString qScanName = QString::fromStdWString(wScan->getName());
        updateProgress(scanCount, 100, buildSteps + scanCount * 100);

        if (res)
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Color balance applied to scan %1 in %2 seconds.").arg(qScanName).arg(seconds)));
        else
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Failed to apply color balance to scan %1.").arg(qScanName)));
    }

    controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));

    if (m_openFolderAfterExport)
        controller.updateInfo(new GuiDataOpenInExplorer(m_outputFolder));

    m_state = ContextState::done;
    return (m_state);
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

ColorBalanceSettings ContextColorBalance::buildSettings() const
{
    ColorBalanceSettings settings;
    switch (m_strength)
    {
    case ColorBalanceStrength::Low:
        settings.neighborCount = 16;
        settings.trimRatio = 0.1;
        settings.beta = 2.0;
        break;
    case ColorBalanceStrength::Mid:
        settings.neighborCount = 24;
        settings.trimRatio = 0.2;
        settings.beta = 2.5;
        break;
    case ColorBalanceStrength::High:
        settings.neighborCount = 32;
        settings.trimRatio = 0.3;
        settings.beta = 3.0;
        break;
    default:
        break;
    }
    return settings;
}
