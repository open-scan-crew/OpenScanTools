#include "controller/functionSystem/ContextColorBalanceFilter.h"

#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/messages/ModalMessage.h"
#include "controller/messages/ColorBalanceFilterMessage.h"
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
#include "utils/Config.h"
#include "utils/Logger.h"
#include "utils/ScanJobRunner.h"
#include "utils/Utils.h"

#include <algorithm>
#include <filesystem>
#include <functional>
#include <mutex>
#include <system_error>
#include <thread>

namespace
{
    void cleanupTempColorBalanceFiles(const std::filesystem::path& tempFolder)
    {
        if (!std::filesystem::is_directory(tempFolder))
            return;

        std::error_code ec;
        const std::wstring suffix = L"_CB_temp";
        for (const auto& entry : std::filesystem::directory_iterator(tempFolder, ec))
        {
            if (ec)
                return;

            if (!entry.is_regular_file())
                continue;

            const std::filesystem::path& path = entry.path();
            const std::wstring stem = path.stem().wstring();
            if (path.extension() == ".tls" && stem.size() >= suffix.size() && stem.compare(stem.size() - suffix.size(), suffix.size(), suffix) == 0)
            {
                std::filesystem::remove(path, ec);
            }
        }
    }

    void deleteFileWithRetry(const std::filesystem::path& filePath)
    {
        if (filePath.empty())
            return;

        std::error_code ec;
        for (int attempt = 0; attempt < 5; ++attempt)
        {
            std::filesystem::remove(filePath, ec);
            if (!ec || !std::filesystem::exists(filePath, ec))
                return;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

// Note (Aurélien) QT::StandardButtons enum values in qmessagebox.h
#define Yes 0x00004000
#define No 0x00010000

ContextColorBalanceFilter::ContextColorBalanceFilter(const ContextId& id)
    : AContext(id)
    , m_panoramic(xg::Guid())
{
    m_state = ContextState::waiting_for_input;
}

ContextColorBalanceFilter::~ContextColorBalanceFilter()
{}

ContextState ContextColorBalanceFilter::start(Controller& controller)
{
    GraphManager& graphManager = controller.getGraphManager();
    std::unordered_set<SafePtr<PointCloudNode>> scans = graphManager.getVisibleScans(m_panoramic);
    if (scans.empty())
    {
        FUNCLOG << "No Scans visibles to balance" << LOGENDL;
        controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_NO_SCAN_SELECTED));
        return (m_state = ContextState::abort);
    }

    bool rgbAvailable = false;
    bool intensityAvailable = false;
    bool rgbAndIntensityAvailable = false;
    for (const SafePtr<PointCloudNode>& scan : scans)
    {
        ReadPtr<PointCloudNode> rScan = scan.cget();
        if (!rScan)
            continue;

        bool scanRgb = rScan->getRGBAvailable();
        bool scanIntensity = rScan->getIntensityAvailable();
        rgbAvailable |= scanRgb;
        intensityAvailable |= scanIntensity;
        if (scanRgb && scanIntensity)
            rgbAndIntensityAvailable = true;
    }

    controller.updateInfo(new GuiDataColorBalanceFilterDialogDisplay(rgbAvailable, intensityAvailable, rgbAndIntensityAvailable));
    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextColorBalanceFilter::feedMessage(IMessage* message, Controller& controller)
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
        auto decodedMsg = static_cast<ColorBalanceFilterMessage*>(message);
        m_kMin = decodedMsg->kMin;
        m_kMax = decodedMsg->kMax;
        m_trimPercent = decodedMsg->trimPercent;
        m_sharpnessBlend = decodedMsg->sharpnessBlend;
        m_globalBalancing = decodedMsg->mode == ColorBalanceMode::Global;
        m_applyOnIntensityAndRgb = decodedMsg->applyOnIntensityAndRgb;
        m_outputFileType = decodedMsg->outputFileType;
        m_outputFolder = decodedMsg->outputFolder;
        m_openFolderAfterExport = decodedMsg->openFolderAfterExport;

        m_warningModal = true;
        controller.updateInfo(new GuiDataModal(Yes | No, TEXT_COLOR_BALANCE_FILTER_QUESTION));
    }
    break;
    default:
        break;
    }

    return (m_state);
}

ContextState ContextColorBalanceFilter::launch(Controller& controller)
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

    if (m_globalBalancing && scans.size() < 2)
    {
        controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_COLOR_BALANCE_NEEDS_MULTIPLE_SCANS));
        m_state = ContextState::abort;
        return m_state;
    }

    if (m_globalBalancing)
    {
        TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(m_panoramic, true, true));
    }

    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString()));
    const uint64_t totalScans = scanList.size();
    const uint64_t totalProgressSteps = totalScans * 100;
    controller.updateInfo(new GuiDataProcessingSplashScreenStart(totalProgressSteps, TEXT_EXPORT_COLOR_BALANCE_TITLE_PROGRESS, TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(0).arg(totalScans)));

    ClippingAssembly clippingAssembly;
    std::unordered_set<SafePtr<AClippingNode>> clippings = graphManager.getActivatedOrSelectedClippingObjects();
    if (!clippings.empty())
        graphManager.getClippingAssembly(clippingAssembly, clippings);
    const bool useTempClippedScans = m_globalBalancing && !clippingAssembly.empty();
    std::filesystem::path tempFolder;
    if (useTempClippedScans)
    {
        tempFolder = controller.getContext().cgetProjectInternalInfo().getPointCloudFolderPath(false) / "temp_color_balance";
        if (!prepareOutputDirectory(controller, tempFolder))
        {
            m_state = ContextState::abort;
            return m_state;
        }
        cleanupTempColorBalanceFiles(tempFolder);
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
    std::atomic<uint64_t> totalModifiedPoints{0};

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
    if (m_globalBalancing)
    {
        auto workingScans = graphManager.getVisiblePointCloudInstances(m_panoramic, true, true);
        options.threadInit = [workingScans]()
        {
            TlScanOverseer::setWorkingScansTransfo(workingScans);
        };
    }

    ScanJobRunner::run(scanList, options, [&](size_t scanIndex, const SafePtr<PointCloudNode>& scan)
    {
        WritePtr<PointCloudNode> wScan = scan.get();
        if (!wScan)
            return;

        ClippingAssembly emptyAssembly;
        const ClippingAssembly* clippingToUse = clippingAssembly.empty() ? &emptyAssembly : &clippingAssembly;
        ClippingAssembly resolvedAssembly;
        if (clippingAssembly.hasPhaseClipping())
        {
            resolvedAssembly = clippingAssembly.resolveByPhase(wScan->getPhase());
            clippingToUse = &resolvedAssembly;
        }

        uint64_t modifiedPointCount = 0;
        std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
        tls::ScanGuid old_guid = wScan->getScanGuid();

        IScanFileWriter* scan_writer = nullptr;
        std::wstring log;
        std::wstring outputName = wScan->getName() + L"_CB";
        if (!getScanFileWriter(m_outputFolder, outputName, m_outputFileType, log, &scan_writer, true) || scan_writer == nullptr)
            return;

        tls::ScanHeader header;
        TlScanOverseer::getInstance().getScanHeader(old_guid, header);
        header.guid = xg::newGuid();
        scan_writer->appendPointCloud(header, wScan->getTransformation());

        bool hasRgb = wScan->getRGBAvailable();
        bool hasIntensity = wScan->getIntensityAvailable();
        bool applyOnRgb = hasRgb;
        bool applyOnIntensity = false;
        if (hasIntensity && hasRgb)
            applyOnIntensity = m_applyOnIntensityAndRgb;
        else if (hasIntensity)
            applyOnIntensity = true;

        if (!clippingAssembly.empty() && !TlScanOverseer::getInstance().testClippingEffect(old_guid, (TransformationModule)*&wScan, *clippingToUse))
            clippingToUse = &emptyAssembly;

        const ClippingAssembly* clippingForExternal = clippingToUse;
        tls::ScanGuid balanceGuid = old_guid;
        const TransformationModule balanceTransform = (TransformationModule)*&wScan;
        std::filesystem::path tempPath;
        if (useTempClippedScans && clippingToUse != &emptyAssembly)
        {
            TlsFileWriter* clip_writer = nullptr;
            std::wstring tempLog;
            std::wstring guidSuffix = Utils::from_utf8(xg::newGuid().str());
            std::wstring tempName = wScan->getName() + L"_CB_temp_" + guidSuffix;
            TlsFileWriter::getWriter(tempFolder, tempName, tempLog, (IScanFileWriter**)&clip_writer);
            if (clip_writer != nullptr)
            {
                tls::ScanHeader clipHeader;
                TlScanOverseer::getInstance().getScanHeader(old_guid, clipHeader);
                clipHeader.guid = xg::newGuid();
                clip_writer->appendPointCloud(clipHeader, wScan->getTransformation());
                bool clipRes = TlScanOverseer::getInstance().clipScan(old_guid, (TransformationModule)*&wScan, *clippingToUse, clip_writer);
                clipRes &= clip_writer->finalizePointCloud();
                tempPath = clip_writer->getFilePath();
                delete clip_writer;

                tls::ScanGuid tempGuid;
                if (clipRes && TlScanOverseer::getInstance().getScanGuid(tempPath, tempGuid))
                {
                    balanceGuid = tempGuid;
                    clippingToUse = &emptyAssembly;
                }
            }
        }

        std::function<void(const GeometricBox&, std::vector<PointXYZIRGB>&)> externalProvider;
        if (m_globalBalancing)
        {
            externalProvider = [clippingForExternal, old_guid](const GeometricBox& box, std::vector<PointXYZIRGB>& points)
            {
                TlScanOverseer::getInstance().collectPointsInGeometricBox(box, *clippingForExternal, old_guid, points);
            };
        }

        auto progressCallback = makeProgressCallback(scanIndex, 0, 100);
        bool res = TlScanOverseer::getInstance().balanceColorsAndWrite(balanceGuid, balanceTransform, *clippingToUse, m_kMin, m_kMax, m_trimPercent, m_sharpnessBlend, applyOnIntensity, applyOnRgb, externalProvider, scan_writer, modifiedPointCount, progressCallback);
        res &= scan_writer->finalizePointCloud();
        delete scan_writer;
        if (balanceGuid != old_guid)
        {
            TlScanOverseer::getInstance().freeScan_async(balanceGuid, false);
            deleteFileWithRetry(tempPath);
        }

        totalModifiedPoints.fetch_add(modifiedPointCount);
        completedScans.fetch_add(1);
        updateProgressForScan(scanIndex, 100);
        float seconds = std::chrono::duration<float, std::ratio<1>>(std::chrono::steady_clock::now() - startTime).count();
        QString qScanName = QString::fromStdWString(wScan->getName());

        if (modifiedPointCount > 0)
            safeUpdateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("%1 points updated in scan %2 in %3 seconds.").arg(modifiedPointCount).arg(qScanName).arg(seconds)));
        else
            safeUpdateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Scan %1 not affected by color balance.").arg(qScanName)));
    });

    updateProgress(100, totalProgressSteps);
    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Total points updated: %1").arg(totalModifiedPoints.load())));
    controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));

    if (m_openFolderAfterExport)
        controller.updateInfo(new GuiDataOpenInExplorer(m_outputFolder));

    m_state = ContextState::done;
    return (m_state);
}

bool ContextColorBalanceFilter::canAutoRelaunch() const
{
    return false;
}

ContextType ContextColorBalanceFilter::getType() const
{
    return ContextType::colorBalanceFilter;
}

bool ContextColorBalanceFilter::prepareOutputDirectory(Controller& controller, const std::filesystem::path& folderPath)
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
