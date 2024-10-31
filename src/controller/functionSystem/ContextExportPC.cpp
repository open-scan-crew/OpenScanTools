#include "controller/functionSystem/ContextExportPC.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/functionSystem/FunctionManager.h"

#include "controller/messages/ClippingExportParametersMessage.h"
#include "controller/messages/CameraMessage.h"
#include "controller/messages/ModalMessage.h"
#include "controller/messages/NewProjectMessage.h"


#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataIO.h"
#include "gui/GuiData/GuiDataContextRequest.h"
#include "gui/texts/ExportTexts.hpp"
#include "gui/texts/SplashScreenTexts.hpp"
#include "pointCloudEngine/PCE_core.h"
#include "pointCloudEngine/TlScanOverseer.h"

#include "io/exports/IScanFileWriter.h"
#include "io/exports/RcpFileWriter.h"
#include "io/exports/TlsWriter.h"
#include "io/exports/CSVWriter.hxx"

#include "utils/math/trigo.h"
#include "utils/Utils.h"
#include "utils/system.h"

#include "models/pointCloud/PointXYZIRGB.h"
#include "models/3d/GridCalculation.h"

#include "models/graph/CameraNode.h"
#include "models/graph/ScanNode.h"
#include "models/graph/GraphManager.h"
#include "models/graph/BoxNode.h"
#include "io/SaveLoadSystem.h"
#include "utils/Logger.h"


// Note (Aurélien) QT::StandardButtons enum values in qmessagebox.h
#define Yes 0x00004000
#define No 0x00010000
#define Cancel 0x00400000

#define IOLOG Logger::log(LoggerMode::IOLog)
#define LOGENDL Logger::endl

ContextExportPC::ContextExportPC(const ContextId& id)
    : AContext(id)
    , m_saveContext(0)
    , m_viewportId(xg::Guid())
    , m_currentStep(0)
    , m_neededMessageCount(3)
{
    m_state = ContextState::waiting_for_input;
}

ContextExportPC::~ContextExportPC()
{}

ContextState ContextExportPC::start(Controller& controller)
{
    controller.updateInfo(new GuiDataContextRequestActiveCamera(m_id));
    if (controller.getContext().getIsCurrentProjectSaved() == false) 
        controller.updateInfo(new GuiDataModal(Yes | No , TEXT_EXPORT_SAVE_QUESTION));
    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextExportPC::feedMessage(IMessage* message, Controller& controller)
{
    GraphManager& graphManager = controller.getGraphManager();
    switch (message->getType())
    {
    case IMessage::MessageType::MODAL:
    {
        ModalMessage* modal = static_cast<ModalMessage*>(message);
        if (modal->m_returnedValue == Yes)
            m_saveContext = controller.getFunctionManager().launchBackgroundFunction(controller, ContextType::saveProject, 0);

    }
    break;
    case IMessage::MessageType::CAMERA:
    {
        CameraMessage* cameraInfo = static_cast<CameraMessage*>(message);
        m_cameraNode = cameraInfo->m_cameraNode;
        m_neededMessageCount--;
    }
    break;
    case IMessage::MessageType::EXPORT_INIT:
    {
        auto decodedMsg = static_cast<ExportInitMessage*>(message);
        m_useClips = decodedMsg->m_useClippings;
        m_useGrids = decodedMsg->m_useGrids;
        m_exportScans = decodedMsg->m_exportScans;
        m_exportPCOs = decodedMsg->m_exportPCOs;
        m_selectedPcs = decodedMsg->m_exportPcs;

        m_neededMessageCount--;

        break;
    }
    case IMessage::MessageType::CLIPPING_EXPORT_PARAMETERS:
    {
        auto decodedMsg = static_cast<ClippingExportParametersMessage*>(message);
        m_parameters = decodedMsg->m_parameters;

        if (m_parameters.exportWithScanImportTranslation)
            m_scanTranslationToAdd = -controller.getContext().cgetProjectInfo().m_importScanTranslation;

        // Prerequisites
        if (prepareOutputDirectory(controller, m_parameters.outFolder) == false)
            return (m_state = ContextState::abort);

        if (m_parameters.fileName.empty())
        {
            m_parameters.fileName = "export_";
            m_parameters.fileName += controller.getContext().cgetProjectInfo().m_projectName;
            std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::string str = std::ctime(&time);
            for (int i = 0; i < str.size(); i++)
            {
                if (str[i] == ' ' || str[i] == '\r' || str[i] == '\n' || str[i] == ':')
                    str[i] = '_';
            }
            m_parameters.fileName += "_";
            m_parameters.fileName += str;
        }

        m_neededMessageCount--;

        break;
    }
    break;

    default:
    {
        FUNCLOG << "wrong message type" << LOGENDL;
        break;
    }
    }

    if (m_neededMessageCount == 1)
    {

        std::unordered_set<SafePtr<AClippingNode>> clippings;
        if(m_useClips || m_useGrids)
            clippings = graphManager.getActivatedOrSelectedClippingObjects();

        if (m_useClips)
        {
            // Check that at least one clipping box is selected.
            if (clippings.empty())
            {
                FUNCLOG << "No Clipping object selected or active" << LOGENDL;
                controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_NO_USABLE_CLIPPING_OBJECTS));
                return (m_state = ContextState::abort);
            }
        }

        if (m_useGrids)
        {
            bool noGrid = true;
            for (const SafePtr<AClippingNode>& clip : clippings)
            {
                ElementType type;
                {
                    ReadPtr<AClippingNode> rClip = clip.cget();
                    if (rClip)
                        type = rClip->getType();
                }
                if (type == ElementType::Grid)
                    noGrid = false;
            }
            // Check that at least one clipping box is selected.
            if (noGrid)
            {
                FUNCLOG << "No Grid boxes selected" << LOGENDL;
                controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_GRID_SELECT_FIRST));
                return (m_state = ContextState::abort);
            }
        }

        ReadPtr<CameraNode> rCam = m_cameraNode.cget();
        if (!rCam)
        {
            FUNCLOG << "Cam error" << LOGENDL;
            assert(false);
            return (m_state = ContextState::abort);
        }

        xg::Guid panoGuid;
        {
            ReadPtr<ScanNode> rScan = rCam->getPanoramicScan().cget();
            if (rScan)
                panoGuid = rScan->getScanGuid();
        }

        std::vector<tls::PointCloudInstance> visibleScans = graphManager.getVisiblePointCloudInstances(panoGuid, m_exportScans, m_exportPCOs);


        if (m_selectedPcs.empty() && visibleScans.empty())
        {
            FUNCLOG << "No Scans visibles or selected to export" << LOGENDL;
            controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_NO_SCAN_SELECTED));
            return (m_state = ContextState::abort);
        }

        if (!m_forSubProject)
            controller.updateInfo(new GuiDataExportParametersDisplay(clippings, m_useClips, m_useGrids, (m_selectedPcs.size() > 1 || visibleScans.size() > 1)));
    }

    if (m_neededMessageCount > 0)
        m_state = ContextState::waiting_for_input;
    else
        m_state = ContextState::ready_for_using;

    return (m_state);
}

void writeHeaderInCSV(CSVWriter& csvWriter, const tls::ScanHeader& header)
{
    csvWriter << header.name;
    csvWriter << (header.pointCount);
    csvWriter << (header.transfo.translation[0]);
    csvWriter << (header.transfo.translation[1]);
    csvWriter << (header.transfo.translation[2]);
    csvWriter << (header.bbox.xMax - header.bbox.xMin);
    csvWriter << (header.bbox.yMax - header.bbox.yMin);
    csvWriter << (header.bbox.zMax - header.bbox.zMin);
    glm::dvec3 eulers(tls::math::quat_to_euler_zyx_deg(glm::dquat(header.transfo.quaternion[3], header.transfo.quaternion[0], header.transfo.quaternion[1], header.transfo.quaternion[2])));
    csvWriter << (eulers.x);
    csvWriter << (eulers.y);
    csvWriter << (eulers.z);
    csvWriter.endLine();
}

ContextState ContextExportPC::launch(Controller& controller)
{
    bool result = false;

    if (m_parameters.clippingFilter == ExportClippingFilter::NONE)
    {
        // Scans separated, no clippings
        // Scans merged, no clippings
        result = processScanExport(controller);
    }
    else if (m_parameters.clippingFilter == ExportClippingFilter::SELECTED ||
        m_parameters.clippingFilter == ExportClippingFilter::ACTIVE)
    {
        // Scans separated, clippings active, clipping separated
        // Scans merged, clippings active, clipping separated
        // Scans merged, clippings active, clipping merged
        result = processClippingExport(controller);
    }
    else if (m_parameters.clippingFilter == ExportClippingFilter::GRIDS)
    {
        // Scans merged, grids active, clipping separated
        result = processGridExport(controller);
    }

    m_state = result ? ContextState::done : ContextState::abort;
    if (result && m_parameters.openFolderAfterExport && !m_forSubProject)
        controller.updateInfo(new GuiDataOpenInExplorer(m_parameters.outFolder));

    return (m_state);
}

bool ContextExportPC::canAutoRelaunch() const
{
    return (false);
}

ContextType ContextExportPC::getType() const
{
    return (ContextType::exportPC);
}

bool ContextExportPC::processClippingExport(Controller& controller)
{
    auto start = std::chrono::steady_clock::now();
    bool resultOk;

    CSVWriter* pCsvWriter = nullptr;
    if(!m_forSubProject)
    {
        pCsvWriter = new CSVWriter(m_parameters.outFolder / "summary.csv");
        *pCsvWriter << "Name;Point_Count;X;Y;Z;SizeX;SizeY;SizeZ;RotationX;RotationY;RotationZ";
        pCsvWriter->endLine();
    }

    switch (m_parameters.method)
    {
        case ExportClippingMethod::SCAN_SEPARATED:
            resultOk = exportScanSeparated(controller, pCsvWriter);
            break;
        case ExportClippingMethod::CLIPPING_SEPARATED:
            resultOk = exportClippingSeparated(controller, pCsvWriter);
            break;
        case ExportClippingMethod::CLIPPING_AND_SCAN_MERGED:
            resultOk = exportClippingAndScanMerged(controller, pCsvWriter);
            break;
    }

    if (pCsvWriter != nullptr)
        delete pCsvWriter;

    float time = std::chrono::duration<float, std::ratio<1>>(std::chrono::steady_clock::now() - start).count();
    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString(resultOk ? TEXT_EXPORT_CLIPPING_SUCCESS_TIME : TEXT_EXPORT_CLIPPING_ERROR_TIME).arg(time)));
    controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));

    return resultOk;
}

bool ContextExportPC::exportClippingAndScanMerged(Controller& controller, CSVWriter* pCsvWriter)
{
    GraphManager& graphManager = controller.getGraphManager();
    TlScanOverseer& overseer = TlScanOverseer::getInstance();

    // Create the clipping assembly
    bool filterActive = (m_parameters.clippingFilter == ExportClippingFilter::ACTIVE);
    bool filterSelected = (m_parameters.clippingFilter == ExportClippingFilter::SELECTED);
    ClippingAssembly clippingAssembly;
    graphManager.getClippingAssembly(clippingAssembly, filterActive, filterSelected);
    std::vector<tls::PointCloudInstance> pcInfos = getPointCloudInstances(graphManager);

    // Get the best origin and bbox based on the clipping used to merge scans.
    //glm::dvec3 bestOrigin; // By default, it is the center of the bbox
    //glm::dquat bestOrientation;
    BoundingBoxD scanBbox = getGlobalBoundingBox(pcInfos);
    //getBestOriginOrientationAndBBox(clippingAssembly, scanBbox, bestOrigin, bestOrientation);
    //bestOrigin += m_scanTranslationToAdd;
    TransformationModule best_transfo = getBestTransformation(clippingAssembly, scanBbox);

    controller.updateInfo(new GuiDataProcessingSplashScreenStart(pcInfos.size(), TEXT_EXPORT_CLIPPING_TITLE_PROGESS, TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(0).arg(pcInfos.size())));

    // Processing
    bool resultOk = true;
    bool fileIsProject = (m_parameters.outFileType == FileType::RCP);
    uint64_t scanExported = 0;

    std::unique_ptr<IScanFileWriter> scanFileWriter = nullptr;
    std::wstring filename = m_parameters.fileName.wstring();
    // Create a Writer per scan or per project
    if (ensureFileWriter(controller, scanFileWriter, filename, pCsvWriter) == false)
        return false;
    tls::ScanHeader mergedHeader;
    mergedHeader.name = filename;
    mergedHeader.precision = m_parameters.encodingPrecision;
    mergedHeader.pointCount = 0;
    //mergedHeader.transfo = tls::Transformation{ {bestOrientation.x, bestOrientation.y, bestOrientation.z, bestOrientation.w}, {bestOrigin.x, bestOrigin.y, bestOrigin.z} };
    mergedHeader.bbox.setEmpty();
    mergedHeader.format = getCommonFormat(pcInfos);
    scanFileWriter->appendPointCloud(mergedHeader, best_transfo);

    for (const auto pcInfo : pcInfos)
    {
        if (m_state != ContextState::running)
        {
            m_state = ContextState::abort;
            break;
        }

        glm::dmat4 modelMatrix = pcInfo.transfo.getTransformation();
        resultOk &= overseer.clipScan(pcInfo.header.guid, modelMatrix, clippingAssembly, scanFileWriter.get()); // [old] merging == true

        // Log GUI
        scanExported++;
        controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(scanExported).arg(pcInfos.size()), scanExported));
    }

    // CSV - merged
    mergedHeader.pointCount = scanFileWriter->getScanPointCount();
    writeHeaderInCSV(*pCsvWriter, mergedHeader);

    // Close writer
    scanFileWriter->flushWrite();
    scanFileWriter.reset();

    return resultOk;
}

// NOTE - Pour l'export de clipping séparées, on n'utilise que les clipping en affichage intérieur.
bool ContextExportPC::exportClippingSeparated(Controller& controller, CSVWriter* pCsvWriter)
{
    bool resultOk = true;
    GraphManager& graphManager = controller.getGraphManager();

    std::vector<SafePtr<AClippingNode>> filteredClippings;
    {
        std::unordered_set<SafePtr<AClippingNode>> clippings = graphManager.getClippingObjects(false, false);
        for (const SafePtr<AClippingNode>& clip : clippings)
        {
            ReadPtr<AClippingNode> rClip = clip.cget();
            // Only the CB selected
            if (rClip && ((rClip->isSelected() && m_parameters.clippingFilter == ExportClippingFilter::SELECTED) || (rClip->isClippingActive() && m_parameters.clippingFilter == ExportClippingFilter::ACTIVE)) &&
                rClip->getClippingMode() == ClippingMode::showInterior)
            {
                filteredClippings.push_back(clip);
            }
        }
    }

    TlScanOverseer& overseer = TlScanOverseer::getInstance();
    std::vector<tls::PointCloudInstance> pcInfos = getPointCloudInstances(graphManager);
    std::unique_ptr<IScanFileWriter> scanFileWriter = nullptr;
    uint32_t clippingExported = 0;
    bool fileIsProject = (m_parameters.outFileType == FileType::RCP);

    controller.updateInfo(new GuiDataProcessingSplashScreenStart(filteredClippings.size(), TEXT_EXPORT_CLIPPING_TITLE_PROGESS, TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(0).arg(filteredClippings.size())));

    for (const SafePtr<AClippingNode>& clip : filteredClippings)
    {
        if (m_state != ContextState::running)
        {
            m_state = ContextState::abort;
            break;
        }

        ClippingAssembly clippingAssembly;
        tls::ScanHeader dstScanHeader;

        {
            ReadPtr<AClippingNode> rClip = clip.cget();
            if (!rClip)
                continue;

            // Create a Writer per scan or per project
            if (ensureFileWriter(controller, scanFileWriter, rClip->getComposedName(), pCsvWriter) == false)
                continue;

            // Clip all the scans for this box
            TransformationModule transfo = (TransformationModule)(*&rClip);
            rClip->pushClippingGeometries(clippingAssembly, transfo);
            //glm::dvec3 bestOrigin; // By default, it is the center of the bbox
            //glm::dquat bestOrientation;
            BoundingBoxD scan_bbox = getGlobalBoundingBox(pcInfos);
            //getBestOriginOrientationAndBBox(clippingAssembly, scan_bbox, bestOrigin, bestOrientation);
            //bestOrigin += m_scanTranslationToAdd;

            dstScanHeader.name = rClip->getComposedName();
            dstScanHeader.precision = m_parameters.encodingPrecision;
            dstScanHeader.pointCount = 0;
            //dstScanHeader.transfo = tls::Transformation{ {bestOrientation.x, bestOrientation.y, bestOrientation.z, bestOrientation.w}, {bestOrigin.x, bestOrigin.y, bestOrigin.z} };
            dstScanHeader.bbox.setEmpty();
            dstScanHeader.format = getCommonFormat(pcInfos);
            scanFileWriter->appendPointCloud(dstScanHeader, getBestTransformation(clippingAssembly, scan_bbox));
        }

        for (auto pcInfo : pcInfos)
        {
            glm::dmat4 modelMatrix = pcInfo.transfo.getTransformation();
            resultOk &= overseer.clipScan(pcInfo.header.guid, modelMatrix, clippingAssembly, scanFileWriter.get()); // [old] merging == true
        }

        scanFileWriter->flushWrite();
        // CSV separated
        dstScanHeader.pointCount = scanFileWriter->getScanPointCount();
        writeHeaderInCSV(*pCsvWriter, dstScanHeader);

        // Log GUI
        clippingExported++;
        controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(QString("Clipping : %1/%2").arg(clippingExported).arg(filteredClippings.size()), clippingExported));

        if (fileIsProject == false)
            scanFileWriter.reset();
    }

    return resultOk;
}

bool ContextExportPC::exportScanSeparated(Controller& controller, CSVWriter* pCsvWriter)
{
    IOLOG << "Export PC scan separated" << LOGENDL;

    bool resultOk = true;
    GraphManager& graphManager = controller.getGraphManager();
    TlScanOverseer& overseer = TlScanOverseer::getInstance();


    // On récupère les clippings à utiliser depuis le projet
    bool filterActive = (m_parameters.clippingFilter == ExportClippingFilter::ACTIVE);
    bool filterSelected = (m_parameters.clippingFilter == ExportClippingFilter::SELECTED);
    ClippingAssembly clippingAssembly;
    graphManager.getClippingAssembly(clippingAssembly, filterActive, filterSelected);

    std::vector<tls::PointCloudInstance> pcInfos = getPointCloudInstances(graphManager);

    controller.updateInfo(new GuiDataProcessingSplashScreenStart(pcInfos.size(), TEXT_EXPORT_CLIPPING_TITLE_PROGESS, TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(0).arg(pcInfos.size())));

    // Processing
    bool fileIsProject = (m_parameters.outFileType == FileType::RCP);
    uint64_t scanExported = 0;

    std::unique_ptr<IScanFileWriter> scanFileWriter = nullptr;

    IOLOG << "pcInfos size : " << pcInfos.size() << LOGENDL;
    for (const auto pcInfo : pcInfos)
    {
        if (m_state != ContextState::running)
        {
            m_state = ContextState::abort;
            break;
        }
        // Create a Writer per scan or per project
        if (!ensureFileWriter(controller, scanFileWriter, pcInfo.header.name + (m_forSubProject ? L"" : L"_clipped"), pCsvWriter))
            continue;

        tls::ScanHeader dstScanHeader = pcInfo.header;
        dstScanHeader.name = dstScanHeader.name + (m_forSubProject ? L"" : L"_clipped");
        dstScanHeader.precision = m_parameters.encodingPrecision;
        dstScanHeader.pointCount = 0;
        dstScanHeader.format = pcInfo.header.format;
        //glm::dvec3 position = pcInfo.transfo.getCenter() + m_scanTranslationToAdd;
        //glm::dquat orientation = pcInfo.transfo.getOrientation();
        //dstScanHeader.transfo = tls::Transformation{ {orientation.x, orientation.y, orientation.z, orientation.w}, {position.x, position.y, position.z} };;
        scanFileWriter->appendPointCloud(dstScanHeader, pcInfo.transfo);

        glm::dmat4 modelMatrix = pcInfo.transfo.getTransformation();
        resultOk &= overseer.clipScan(pcInfo.header.guid, modelMatrix, clippingAssembly, scanFileWriter.get()); // [old] merging == false

        scanFileWriter->flushWrite(); // Rule: 1 flush for 1 append
        // CSV separated
        if (!m_forSubProject)
        {
            dstScanHeader.pointCount = scanFileWriter->getScanPointCount();
            writeHeaderInCSV(*pCsvWriter, dstScanHeader);
        }

        // Log GUI
        scanExported++;
        controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(QString("Scan : %1/%2").arg(scanExported).arg(pcInfos.size()), scanExported));

        if (fileIsProject == false)
            scanFileWriter.reset();
    }

    return resultOk;
}

bool ContextExportPC::processScanExport(Controller& controller)
{
    auto start = std::chrono::steady_clock::now();
    bool resultOk = true;

    // Get the the visible scans
    TlScanOverseer& overseer = TlScanOverseer::getInstance();
    std::vector<tls::PointCloudInstance> pcInfos = getPointCloudInstances(controller.getGraphManager());

    controller.updateInfo(new GuiDataProcessingSplashScreenStart(pcInfos.size(), TEXT_EXPORT_TITLE_NORMAL, TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(0).arg(pcInfos.size())));

    // Processing
    bool mergeScans = (m_parameters.method == ExportClippingMethod::CLIPPING_AND_SCAN_MERGED);
    bool fileIsProject = (m_parameters.outFileType == FileType::RCP);
    uint64_t scanExported = 0;

    std::unique_ptr<IScanFileWriter> scanFileWriter = nullptr;
    tls::ScanHeader mergedHeader;
    ClippingAssembly emptyClipAssembly;

    for (auto pcInfo : pcInfos)
    {
        if (m_state != ContextState::running)
        {
            m_state = ContextState::abort;
            break;
        }


        ReadPtr<APointCloudNode> rScan = pcInfo.scanNode.cget();
        if (!rScan)
            continue;
        std::wstring uniqueName = rScan->getType() == ElementType::Scan ? rScan->getName() : rScan->getComposedName();
        // Particular case of the output TLS
        // Simply copy the tls files (with the same precision required)
        if (m_parameters.outFileType == FileType::TLS && !mergeScans &&
            pcInfo.header.precision == m_parameters.encodingPrecision)
        {
            std::filesystem::path oldPath;
            oldPath = rScan->getCurrentScanPath();

            std::filesystem::path newPath = m_parameters.outFolder / (uniqueName + L".tls");
            try
            {
                std::filesystem::copy(oldPath, newPath, std::filesystem::copy_options::overwrite_existing);
            }
            catch (const std::filesystem::filesystem_error&)
            {
                controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString(TEXT_EXPORT_ERROR_FILE).arg(QString::fromStdWString(newPath))));
                continue;
            }
            
            std::ofstream os;
            os.open(newPath, std::ios::out | std::ios::in | std::ios::binary);
            if (os.fail())
            {
                controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString(TEXT_EXPORT_ERROR_FILE).arg(QString::fromStdWString(newPath))));
                continue;
            }

            glm::dvec3 position = pcInfo.transfo.getCenter();
            glm::dquat orientation = pcInfo.transfo.getOrientation();
            double newOrigin[] = { position.x, position.y, position.z };
            double newOrientation[] = { orientation.x, orientation.y, orientation.z, orientation.w };
            tls::writer::overwriteTransformation(os, newOrigin, newOrientation);

            //overseer.addCopyScanFile(pcInfo.header.guid, m_parameters.outFolder / (uniqueName + L".tls"), false, false, false);
        }
        else
        {
            // Create a Writer per scan or per project
            if (ensureFileWriter(controller, scanFileWriter, uniqueName, nullptr) == false)
                continue;

            tls::ScanHeader dstScanHeader = pcInfo.header;
            dstScanHeader.precision = m_parameters.encodingPrecision;
            dstScanHeader.pointCount = 0;
            // ------------  Merged Header -------------
            if (mergeScans)
            {
                if (scanExported == 0)
                {
                    dstScanHeader.name = m_parameters.fileName.wstring();
                    //dstScanHeader.transfo = getCommonTransformation(pcInfos);
                    dstScanHeader.format = getCommonFormat(pcInfos);
                    scanFileWriter->appendPointCloud(dstScanHeader, getCommonTransformation_EX(pcInfos));
                    mergedHeader = dstScanHeader;
                }
            }
            else
            {
                dstScanHeader.name = uniqueName;
                //glm::dvec3 position = pcInfo.transfo.getCenter() + m_scanTranslationToAdd;
                //glm::dquat orientation = pcInfo.transfo.getOrientation();
                //dstScanHeader.transfo.quaternion[0] = orientation.x;
                //dstScanHeader.transfo.quaternion[1] = orientation.y;
                //dstScanHeader.transfo.quaternion[2] = orientation.z;
                //dstScanHeader.transfo.quaternion[3] = orientation.w;
                //dstScanHeader.transfo.translation[0] = position.x;
                //dstScanHeader.transfo.translation[1] = position.y;
                //dstScanHeader.transfo.translation[2] = position.z;
                scanFileWriter->appendPointCloud(dstScanHeader, pcInfo.transfo);
            }

            glm::dmat4 modelMatrix = pcInfo.transfo.getTransformation();
            resultOk &= overseer.clipScan(pcInfo.header.guid, modelMatrix, emptyClipAssembly, scanFileWriter.get()); // [old] merging == mergeScans

            if (!mergeScans)
                scanFileWriter->flushWrite(); // Rule: 1 flush for 1 append

            if (mergeScans == false && fileIsProject == false)
                scanFileWriter.reset();
        }
        // Log GUI
        scanExported++; 
        controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(scanExported).arg(pcInfos.size()), scanExported));
    }

    if (scanFileWriter != nullptr)
    {
        // Close writer
        scanFileWriter->flushWrite();
        scanFileWriter.reset();
    }

    float time = std::chrono::duration<float, std::ratio<1>>(std::chrono::steady_clock::now() - start).count();
    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString(resultOk ? TEXT_EXPORT_SUCCESS_TIME : TEXT_EXPORT_ERROR_TIME).arg(time)));
    controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));

    return resultOk;
}

bool ContextExportPC::processGridExport(Controller& controller)
{
    bool resultOk = true;
    auto start = std::chrono::steady_clock::now();
    // Create the clipping info lists
    TlScanOverseer& overseer = TlScanOverseer::getInstance();
    GraphManager& graphManager = controller.getGraphManager();

    std::unordered_set<SafePtr<BoxNode>> grid_nodes = graphManager.getGrids();

    // Get the the visible scans
    std::vector<tls::PointCloudInstance> pcInfos = getPointCloudInstances(graphManager);
    std::unique_ptr<IScanFileWriter> scanFileWriter = nullptr;
    tls::PointFormat commonFormat = getCommonFormat(pcInfos);

    // TODO - Treat the particular case of converting a TLS to a TLS?with the same precision.

    // Processing
    bool fileIsProject = (m_parameters.outFileType == FileType::RCP);
    uint64_t subProjectCount = 0;
    std::wstring log;
    for (const SafePtr<BoxNode>& grid_node : grid_nodes)
    {
        if (m_state != ContextState::running)
        {
            m_state = ContextState::abort;
            break;
        }

        ReadPtr<BoxNode> r_grid = grid_node.cget();
        // Only the Grid activated or selected (depending on the source parameter)
        std::vector<GridBox> extracted_boxes;
        if (r_grid->isSelected() == false ||
            !GridCalculation::calculateBoxes(extracted_boxes, *&r_grid))
        {
            continue;
        }

        size_t boxes_count = extracted_boxes.size();
        controller.updateInfo(new GuiDataProcessingSplashScreenStart(boxes_count, TEXT_EXPORT_GRID_TITLE_PROGESS, TEXT_SPLASH_SCREEN_BOXE_PROCESSING.arg(0).arg(boxes_count)));

        std::filesystem::path out_folder = m_parameters.outFolder;
        if (fileIsProject)
            out_folder /= m_parameters.fileName;
        else
            out_folder /= r_grid->getComposedName();
        if (prepareOutputDirectory(controller, out_folder) == false)
            return false;

        // On reset le file writer (si il y plusieurs grilles avec limitation du nombre de scan)
        uint64_t boxesExported = 0;
        scanFileWriter.reset();
        CSVWriter csvWriter(out_folder / "summary.csv");
        csvWriter << "Name;Point_Count;X;Y;Z;SizeX;SizeY;SizeZ;RotationX;RotationY;RotationZ" << CSVWriter::endl;
        for (const GridBox& box : extracted_boxes)
        {
            if (m_state != ContextState::running)
            {
                m_state = ContextState::abort;
                break;
            }

            std::wstring box_xyz_name;
            box_xyz_name = r_grid->getComposedName() 
                + L"_" + Utils::wCompleteWithZeros(box.position.x) 
                + L"_" + Utils::wCompleteWithZeros(box.position.y)
                + L"_" + Utils::wCompleteWithZeros(box.position.z);

            // Create a Writer per scan or per project or per N boxes
            if (scanFileWriter == nullptr)
            {
                std::wstring outFileName = fileIsProject ? m_parameters.fileName.wstring() : box_xyz_name;
                if (fileIsProject && m_parameters.maxScanPerProject > 0)
                {
                    subProjectCount++;
                    outFileName += L"_";
                    outFileName += std::to_wstring(subProjectCount);
                }

                IScanFileWriter* ptr;
                if (getScanFileWriter(out_folder, outFileName, m_parameters.outFileType, log, &ptr) == false)
                {
                    FUNCLOG << "Export: Failed to create the destination file" << LOGENDL;
                    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString(TEXT_EXPORT_ERROR_FILE).arg(outFileName)));
                    return false;
                }
                else
                    scanFileWriter.reset(ptr);

                if (m_parameters.outFileType == FileType::RCP)
                    static_cast<RcpFileWriter*>(scanFileWriter.get())->setExportDensity(m_parameters.pointDensity);
                if ((m_parameters.outFileType == FileType::RCP) && m_parameters.addOriginCube)
                    addOriginCube(scanFileWriter.get(), commonFormat, csvWriter);
            }

            tls::ScanHeader dstScanHeader;
            dstScanHeader.name = box_xyz_name;
            glm::dvec3 t = box.getCenter();// +m_scanTranslationToAdd;
            glm::quat q = box.getOrientation();
            dstScanHeader.transfo = tls::Transformation{ {q.x, q.y, q.z, q.w}, {t.x, t.y, t.z} };
            // Initialize precision and point count
            dstScanHeader.precision = m_parameters.encodingPrecision;
            dstScanHeader.pointCount = 0;
            dstScanHeader.format = commonFormat;

            // ClippingAssembly for the only box
            ClippingAssembly clipAssembly;
            clipAssembly.clippingUnion.push_back(std::make_shared<BoxClippingGeometry>(ClippingMode::showInterior,
                box.getInverseRotationTranslation(),
                glm::vec4(box.getScale().x, box.getScale().y, box.getScale().z, 0.f), 0));

            scanFileWriter->appendPointCloud(dstScanHeader, (TransformationModule)box);
            for (auto pcInfo : pcInfos)
            {
                if (pcInfo.header.guid == tls::ScanGuid())
                    continue;
                if (m_state != ContextState::running)
                {
                    m_state = ContextState::abort;
                    break;
                }

                // NOTE(robin) - Si on veut utiliser une autre transformation pour le scan que celle d'origine il faut changer la ligne suivante.
                glm::dmat4 modelMatrix = pcInfo.transfo.getTransformation();
                resultOk &= overseer.clipScan(pcInfo.header.guid, modelMatrix, clipAssembly, scanFileWriter.get()); // [old] merging == true
            }
            scanFileWriter->flushWrite();
            boxesExported++; 
            controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(TEXT_SPLASH_SCREEN_BOXE_PROCESSING.arg(boxesExported).arg(boxes_count), boxesExported));

            dstScanHeader.pointCount = scanFileWriter->getScanPointCount();
            glm::vec3 scale = box.getScale();
            dstScanHeader.bbox = { -scale.x, scale.x , -scale.y, scale.y, -scale.z, scale.z };
            writeHeaderInCSV(csvWriter, dstScanHeader);

            if (fileIsProject == false ||
                (m_parameters.maxScanPerProject > 0 && (boxesExported % m_parameters.maxScanPerProject) == 0))
            {
                scanFileWriter.reset();
            }
        }
    }

    float time = std::chrono::duration<float, std::ratio<1>>(std::chrono::steady_clock::now() - start).count();
    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString(resultOk ? TEXT_EXPORT_SUCCESS_TIME : TEXT_EXPORT_ERROR_TIME).arg(time)));
    controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));

    return resultOk;
}

void ContextExportPC::addOriginCube(IScanFileWriter* fileWriter, tls::PointFormat pointFormat, CSVWriter& csvWriter)
{
    tls::ScanHeader scanHeader;
    scanHeader.name = L"box_origin";
    scanHeader.transfo = tls::Transformation{ {0., 0., 0., 1.}, {0., 0., 0.} };
    scanHeader.precision = tls::PrecisionType::TL_OCTREE_100UM; // not used
    scanHeader.pointCount = 0;
    scanHeader.format = pointFormat;

    TransformationModule base_transfo;

    fileWriter->appendPointCloud(scanHeader, base_transfo);
    PointXYZIRGB pointBuffer[8] = {
        {0.0, 0.0, 0.0, 255, 0, 0, 0},
        {1.0, 0.0, 0.0, 255, 255, 0, 0},
        {0.0, 1.0, 0.0, 255, 0, 255, 0},
        {1.0, 1.0, 0.0, 255, 255, 255, 0},
        {0.0, 0.0, 1.0, 255, 0, 0, 255},
        {1.0, 0.0, 1.0, 255, 255, 0, 255},
        {0.0, 1.0, 1.0, 255, 0, 255, 255},
        {1.0, 1.0, 1.0, 255, 255, 255, 255}
    };
    fileWriter->addPoints(pointBuffer, 8);
    fileWriter->flushWrite();

    // name;point count;origin;size;orientation
    csvWriter << "box_origin;8;0.0;0.0;0.0;1.0;1.0;1.0;0;0;0" << CSVWriter::endl;
}

bool ContextExportPC::ensureFileWriter(Controller& controller, std::unique_ptr<IScanFileWriter>& scanFileWriter, std::wstring name, CSVWriter* csvWriter)
{
    if (scanFileWriter == nullptr)
    {
        std::wstring outName;
        // Name for merged Scans
        if (m_parameters.method == ExportClippingMethod::CLIPPING_AND_SCAN_MERGED ||
            m_parameters.outFileType == FileType::RCP)
        {
            outName = m_parameters.fileName;
        }
        // Name for other case
        else
        {
            outName = name;
        }

        std::wstring log;
        IScanFileWriter* ptr;
        if (getScanFileWriter(m_parameters.outFolder, outName, m_parameters.outFileType, log, &ptr) == false)
        {
            FUNCLOG << "Export: Failed to create the destination file" << LOGENDL;
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString(TEXT_EXPORT_ERROR_FILE).arg(QString::fromStdWString(outName))));
            return false;
        }
        else
            scanFileWriter.reset(ptr);

        if (m_parameters.outFileType == FileType::RCP)
            static_cast<RcpFileWriter*>(scanFileWriter.get())->setExportDensity(m_parameters.pointDensity);
        if ((m_parameters.outFileType == FileType::RCP) && m_parameters.addOriginCube && csvWriter != nullptr)
            addOriginCube(scanFileWriter.get(), tls::PointFormat::TL_POINT_XYZ_I_RGB, *csvWriter);
    }
    return (true);
}

bool ContextExportPC::prepareOutputDirectory(Controller& controller, const std::filesystem::path& folderPath)
{
    if (std::filesystem::is_directory(folderPath) == false)
    {
        try
        {
            if (std::filesystem::create_directories(folderPath) == false)
            {
                IOLOG << "Error: the path '" << folderPath << "' is not a valid path for a folder." << LOGENDL;
                controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_INVALID_DIRECTORY));
                return false;
            }
        }
        catch (std::exception e)
        {
            IOLOG << "Error: the path '" << folderPath << "' is not a valid path for a folder." << LOGENDL;
            controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_INVALID_DIRECTORY));
            return false;
        }
    }

    /*
    std::filesystem::perms dirPerms = std::filesystem::status(folderPath).permissions();

    if ((dirPerms & std::filesystem::perms::others_write) == std::filesystem::perms::none)
    {
        Logger::log(LoggerMode::IOLog) << "Error: you have not the permission to write to " << folderPath << Logger::endl;
        controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_INVALID_DIRECTORY));
        return false;
    }
    */
    return true;
}

std::vector<tls::PointCloudInstance> ContextExportPC::getPointCloudInstances(GraphManager& graphManager)
{
    if (m_selectedPcs.empty())
    {
        IOLOG << "Get all PCInstance, exportScan ? " << m_exportScans << " exportPcos ? " << m_exportPCOs << " scanFilter ? " << (int)m_parameters.pointCloudFilter << LOGENDL;
        return (graphManager.getPointCloudInstances(xg::Guid(), m_exportScans, m_exportPCOs, m_parameters.pointCloudFilter));
    }
    else
    {
        std::vector<tls::PointCloudInstance> pcis;
        for (const SafePtr<APointCloudNode>& pc : m_selectedPcs)
        {
            ReadPtr<APointCloudNode> rPc = pc.cget();
            if (!rPc)
                continue;

            tls::ScanHeader header;
            tlGetScanHeader(rPc->getScanGuid(), header);
            pcis.emplace_back(pc, header, rPc->getTransformationModule(), rPc->getClippable());
        }
        return pcis;
    }
}

void ContextExportPC::getBestOriginOrientationAndBBox(const ClippingAssembly& _clippingAssembly, const BoundingBoxD& _scanBBox, glm::dvec3& _bestOrigin, glm::dquat& _bestOrientation)
{
    BoundingBoxD union_bbox;
    union_bbox.setEmpty();
    for (const std::shared_ptr<IClippingGeometry>& geom : _clippingAssembly.clippingUnion)
    {
        if (geom->mode == ClippingMode::showInterior)
        {
            BoundingBoxD bbox = extractBBox(*geom);
            union_bbox.extend(bbox);
        }
    }

    BoundingBoxD inter_bbox;
    inter_bbox.setInfinite(); // ou tout l'espace
    for (const std::shared_ptr<IClippingGeometry>& geom : _clippingAssembly.clippingIntersection)
    {
        if (geom->mode == ClippingMode::showInterior)
        {
            // NOTE - Par convention, nous n'avons pas de box interieure dans l'intersection
            BoundingBoxD bbox = extractBBox(*geom);
            inter_bbox.intersect(bbox);
        }
    }

    BoundingBoxD total_bbox = _scanBBox;
    if (!_clippingAssembly.clippingUnion.empty())
        total_bbox.intersect(union_bbox);

    if (!_clippingAssembly.clippingIntersection.empty())
        total_bbox.intersect(inter_bbox);

    _bestOrigin = total_bbox.center();
    // NOTE(robin) - Ce n'est pas la meilleure façon de récupérer la rotation.
    //             - On pourrait avoir directement accès au quaternion de la clipping originale, mais cela demanderai un rework de l'interface IClippingGeometry.
    if (_clippingAssembly.clippingUnion.size() == 1)
        _bestOrientation = glm::quat_cast(glm::inverse(_clippingAssembly.clippingUnion[0]->matRT_inv));
    else
        _bestOrientation = glm::dquat(0.0, 0.0, 0.0, 1.0);

}

TransformationModule ContextExportPC::getBestTransformation(const ClippingAssembly& clipping_assembly, const BoundingBoxD& scan_bbox)
{
    TransformationModule best_transfo;

    BoundingBoxD union_bbox;
    union_bbox.setEmpty();
    for (const std::shared_ptr<IClippingGeometry>& geom : clipping_assembly.clippingUnion)
    {
        if (geom->mode == ClippingMode::showInterior)
        {
            BoundingBoxD bbox = extractBBox(*geom);
            union_bbox.extend(bbox);
        }
    }

    BoundingBoxD inter_bbox;
    inter_bbox.setInfinite(); // ou tout l'espace
    for (const std::shared_ptr<IClippingGeometry>& geom : clipping_assembly.clippingIntersection)
    {
        if (geom->mode == ClippingMode::showInterior)
        {
            // NOTE - Par convention, nous n'avons pas de box interieure dans l'intersection
            BoundingBoxD bbox = extractBBox(*geom);
            inter_bbox.intersect(bbox);
        }
    }

    BoundingBoxD total_bbox = scan_bbox;
    if (!clipping_assembly.clippingUnion.empty())
        total_bbox.intersect(union_bbox);

    if (!clipping_assembly.clippingIntersection.empty())
        total_bbox.intersect(inter_bbox);

    best_transfo.setPosition(total_bbox.center());
    // NOTE(robin) - Ce n'est pas la meilleure façon de récupérer la rotation.
    //             - On pourrait avoir directement accès au quaternion de la clipping originale, mais cela demanderai un rework de l'interface IClippingGeometry.
    if (clipping_assembly.clippingUnion.size() == 1)
        best_transfo.setRotation(glm::quat_cast(glm::inverse(clipping_assembly.clippingUnion[0]->matRT_inv)));
    else
        best_transfo.setRotation(glm::dquat(0.0, 0.0, 0.0, 1.0));

    return best_transfo;
}

BoundingBoxD ContextExportPC::getGlobalBoundingBox(const std::vector<tls::PointCloudInstance>& pcInstances)
{
    BoundingBoxD global_bbox;
    global_bbox.setEmpty();

    for (const tls::PointCloudInstance& pcInst : pcInstances)
    {
        BoundingBoxD&& t_bbox = pcInst.header.bbox.transform(pcInst.transfo.getTransformation());
        global_bbox.extend(t_bbox);
    }
    return global_bbox;
}

BoundingBoxD ContextExportPC::extractBBox(const IClippingGeometry& clippingGeom)
{
    // NOTE(robin) - Ceci ne fonctionne que pour les clippings de type box
    float x = (float)clippingGeom.params.x;
    float y = (float)clippingGeom.params.y;
    float z = (float)clippingGeom.params.z;

    BoundingBoxD bbox = { -x, x, -y, y, -z, z };

    return bbox.transform(glm::inverse(clippingGeom.matRT_inv));
}

tls::Transformation ContextExportPC::getCommonTransformation(const std::vector<tls::PointCloudInstance>& pcInfos)
{
    // Approximative but it works enough for now
    // FIXME - use the bounding box
    glm::dvec3 barycenter(0.0, 0.0, 0.0);
    for (auto pcInfo : pcInfos)
    {
        barycenter += pcInfo.transfo.getCenter();
    }
    barycenter /= pcInfos.size();
    barycenter += m_scanTranslationToAdd;
    return tls::Transformation{ {0, 0, 0, 1}, {barycenter.x, barycenter.y, barycenter.z} };
}

TransformationModule ContextExportPC::getCommonTransformation_EX(const std::vector<tls::PointCloudInstance>& pcInfos)
{
    // Approximative but it works enough for now
    // FIXME - use the bounding box
    glm::dvec3 barycenter(0.0, 0.0, 0.0);
    for (auto pcInfo : pcInfos)
    {
        barycenter += pcInfo.transfo.getCenter();
    }
    barycenter /= pcInfos.size();

    TransformationModule common_transfo;
    common_transfo.setPosition(barycenter);
    return common_transfo;
}

tls::PointFormat ContextExportPC::getCommonFormat(const std::vector<tls::PointCloudInstance>& pcInfos)
{
    tls::PointFormat commonFormat = tls::PointFormat::TL_POINT_FORMAT_UNDEFINED;
    for (const auto pcInfo : pcInfos)
    {
        tls::getCompatibleFormat(commonFormat, pcInfo.header.format);
    }
    return commonFormat;
}

// L.899

ContextExportSubProject::ContextExportSubProject(const ContextId& id)
    : ContextExportPC(id)
    , m_objectFilterType(ObjectStatusFilter::NONE)
{
    m_neededMessageCount = m_neededMessageCount + 1;
    m_forSubProject = true;
}

ContextExportSubProject::~ContextExportSubProject()
{
}

ContextState ContextExportSubProject::start(Controller& controller)
{
    return ContextExportPC::start(controller);
}

ContextState ContextExportSubProject::feedMessage(IMessage* message, Controller& controller)
{
    ContextExportPC::feedMessage(message, controller);
    if (message->getType() == IMessage::MessageType::SUB_PROJECT)
    {
        NewSubProjectMessage* subProjectMessage = static_cast<NewSubProjectMessage*>(message);
        m_subProjectInfo = subProjectMessage->m_subProjectInfo;
        m_subProjectInternal = subProjectMessage->m_subProjectInternal;
        m_objectFilterType = subProjectMessage->m_objectFilterType;
        m_neededMessageCount--;
    }

    if (m_neededMessageCount > 0)
        m_state = ContextState::waiting_for_input;
    else
        m_state = ContextState::ready_for_using;

    return m_state;
}

ContextState ContextExportSubProject::launch(Controller& controller)
{
    Utils::System::createDirectoryIfNotExist(m_subProjectInternal.getScansFolderPath());
    if (controller.getGraphManager().getActiveClippingCount() > 0)
        ContextExportPC::processClippingExport(controller);
    else
        ContextExportPC::processScanExport(controller);

    const GraphManager& graphManager = controller.getGraphManager();

    std::unordered_set<SafePtr<AGraphNode>> fileObjects;
    std::unordered_set<SafePtr<AGraphNode>> exports = graphManager.getProjectNodesByFilterType(m_objectFilterType);

    for (const SafePtr<AGraphNode>& exportObj : exports)
    {
        ReadPtr<AGraphNode> readNode = exportObj.cget();
        if (!readNode)
            continue;
        switch (readNode->getType())
        {
            case ElementType::PCO:
            case ElementType::MeshObject:
                fileObjects.insert(exportObj);
        }
    }

    for (const SafePtr<AGraphNode>& scanPtr : graphManager.getNodesByTypes({ ElementType::Scan }))
    {
        ReadPtr<ScanNode> scan = static_pointer_cast<ScanNode>(scanPtr).cget();
        if (!scan)
            continue;
        std::filesystem::path scanPath = m_subProjectInternal.getScansFolderPath() / scan->getScanPath().filename();
        if (!std::filesystem::exists(scanPath))
            continue;
        exports.insert(scanPtr);
    }

    std::filesystem::path subProjectPath = SaveLoadSystem::ExportProject(controller, exports, m_subProjectInternal, m_subProjectInfo, m_cameraNode);

    SaveLoadSystem::ExportToProjectFileObjects(controller, m_subProjectInternal, fileObjects);

    if (m_parameters.openFolderAfterExport && !subProjectPath.empty())
        controller.updateInfo(new GuiDataOpenInExplorer(subProjectPath));
    return m_state = ContextState::done;
}

bool ContextExportSubProject::canAutoRelaunch() const
{
    return false;
}

ContextType ContextExportSubProject::getType() const
{
    return ContextType::exportSubProject;
}

ContextState ContextExportSubProject::abort(Controller& controller)
{
    try
    {
        if (std::filesystem::exists(m_subProjectInternal.getProjectFolderPath()))
            std::filesystem::remove_all(m_subProjectInternal.getProjectFolderPath());
    }
    catch(...)
    {}
    return AContext::abort(controller);
}

/*
void ContextExportSubProject::recGetObjectsClusters(std::unordered_map<xg::Guid, const Data*>& dataMap, const Data* data, Project* project)
{
    dataMap[data->getId()] = data;
    std::vector<TreeElement*> treeEls = project->searchRawTreeElementsOnDataId(data->getId());
    for (TreeElement* treeEl : treeEls)
    {
        if (treeEl->parent() == nullptr)
            continue;

        xg::Guid parentDataId = treeEl->parent()->getDataId();
        const Data* parentData = project->cgetDataOnId(parentDataId);

        if (dataMap.find(parentDataId) != dataMap.end())
            continue;

        recGetObjectsClusters(dataMap, parentData, project);
}
*/
