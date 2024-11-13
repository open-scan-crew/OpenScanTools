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

void writeHeaderInCSV(CSVWriter* csvWriter, const tls::ScanHeader& header)
{
    if (csvWriter == nullptr)
        return;

    *csvWriter << header.name;
    *csvWriter << (header.pointCount);
    *csvWriter << (header.transfo.translation[0]);
    *csvWriter << (header.transfo.translation[1]);
    *csvWriter << (header.transfo.translation[2]);
    *csvWriter << (header.bbox.xMax - header.bbox.xMin);
    *csvWriter << (header.bbox.yMax - header.bbox.yMin);
    *csvWriter << (header.bbox.zMax - header.bbox.zMin);
    glm::dvec3 eulers(tls::math::quat_to_euler_zyx_deg(glm::dquat(header.transfo.quaternion[3], header.transfo.quaternion[0], header.transfo.quaternion[1], header.transfo.quaternion[2])));
    *csvWriter << (eulers.x);
    *csvWriter << (eulers.y);
    *csvWriter << (eulers.z);
    csvWriter->endLine();
}

ContextState ContextExportPC::launch(Controller& controller)
{
    CSVWriter* csv_writer = new CSVWriter(m_parameters.outFolder / "summary.csv");
    *csv_writer << "Name;Point_Count;X;Y;Z;SizeX;SizeY;SizeZ;RotationX;RotationY;RotationZ";
    csv_writer->endLine();

    bool result = processExport(controller, csv_writer);

    delete csv_writer;

    return (m_state = result ? ContextState::done : ContextState::abort);
}

bool ContextExportPC::canAutoRelaunch() const
{
    return (false);
}

ContextType ContextExportPC::getType() const
{
    return (ContextType::exportPC);
}

void ContextExportPC::copyTls(Controller& controller, CopyTask task)
{
    try
    {
        std::filesystem::copy(task.src_path, task.dst_path, std::filesystem::copy_options::overwrite_existing);
    }
    catch (const std::filesystem::filesystem_error&)
    {
        controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString(TEXT_EXPORT_ERROR_FILE).arg(QString::fromStdWString(task.dst_path))));
        return;
    }

    std::ofstream os;
    os.open(task.dst_path, std::ios::out | std::ios::in | std::ios::binary);
    if (os.fail())
    {
        controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString(TEXT_EXPORT_ERROR_FILE).arg(QString::fromStdWString(task.dst_path))));
        return;
    }

    tls::writer::overwriteTransformation(os, task.dst_transfo.translation, task.dst_transfo.quaternion);
}

bool ContextExportPC::processExport(Controller& controller, CSVWriter* csv_writer)
{
    bool success = true;
    std::unique_ptr<IScanFileWriter> scanFileWriter = nullptr;
    std::vector<ExportTask> export_tasks;
    std::vector<CopyTask> copy_tasks;
    prepareExportTasks(controller, export_tasks);
    prepareCopyTasks(controller, copy_tasks);

    logStart(controller, export_tasks.size());

    // For each new output file
    for (const ExportTask& task : export_tasks)
    {
        // Check process state
        if (m_state != ContextState::running || !success)
            break;

        // Prepare the file writer
        // Reuse the FileWriter if the 'file_name' is the same
        if (ensureFileWriter(controller, scanFileWriter, task.file_name, csv_writer) == false)
            return false;

        scanFileWriter->appendPointCloud(task.header, task.transfo);
        scanFileWriter->setPostTranslation(m_scanTranslationToAdd); // !!

        // Process each input PC
        for (const tls::PointCloudInstance& pc : task.input_pcs)
        {
            if (m_state != ContextState::running || !success)
                break;

            success &= TlScanOverseer::getInstance().clipScan(pc.header.guid, pc.transfo, task.clippings, scanFileWriter.get());
        }

        // TODO - FileType::RCP & m_parameters.pointDensity
        scanFileWriter->finalizePointCloud();

        writeHeaderInCSV(csv_writer, scanFileWriter->getLastScanHeader());

        logProgress(controller);
    }

    for (const CopyTask& task : copy_tasks)
    {
        copyTls(controller, task);
    }

    logEnd(controller, success);

    return success;
}

void ContextExportPC::prepareExportTasks(Controller& controller, std::vector<ContextExportPC::ExportTask>& export_tasks)
{
    std::vector<tls::PointCloudInstance> pcInfos = getPointCloudInstances(controller);
    tls::PointFormat common_format = getCommonFormat(pcInfos);
    GraphManager& graph = controller.getGraphManager();

    ClippingAssembly clipping_assembly;
    graph.getClippingAssembly(clipping_assembly,
        m_parameters.clippingFilter == ExportClippingFilter::ACTIVE,
        m_parameters.clippingFilter == ExportClippingFilter::SELECTED
    );
    bool is_rcp = (m_parameters.outFileType == FileType::RCP);

    // The output files are based on the grid
    if (m_parameters.clippingFilter == ExportClippingFilter::GRIDS)
    {
        std::unordered_set<SafePtr<BoxNode>> grid_nodes = graph.getGrids();

        for (const SafePtr<BoxNode>& grid_node : grid_nodes)
        {
            std::vector<GridBox> extracted_boxes;
            ReadPtr<BoxNode> r_grid = grid_node.cget();
            if (!r_grid || r_grid->isSelected() == false ||
                !GridCalculation::calculateBoxes(extracted_boxes, *&r_grid))
                continue;

            size_t box_per_sub_project = 0;

            for (const GridBox& box : extracted_boxes)
            {
                ExportTask task;
                std::wstring box_xyz_name;
                box_xyz_name = r_grid->getComposedName()
                    + L"_" + Utils::wCompleteWithZeros(box.position.x)
                    + L"_" + Utils::wCompleteWithZeros(box.position.y)
                    + L"_" + Utils::wCompleteWithZeros(box.position.z);

                if (is_rcp)
                {
                    task.file_name += m_parameters.fileName.wstring();
                    if (m_parameters.maxScanPerProject > 0)
                        task.file_name += L"_" + std::to_wstring(box_per_sub_project++ / m_parameters.maxScanPerProject);
                }
                else
                    task.file_name += box_xyz_name;

                task.header.name = box_xyz_name;
                task.header.precision = m_parameters.encodingPrecision;
                task.header.format = common_format;

                task.transfo = (TransformationModule)box;
                task.transfo.setScale(glm::dvec3(1.0));

                task.input_pcs = pcInfos;
                task.clippings.clippingUnion.push_back(std::make_shared<BoxClippingGeometry>(ClippingMode::showInterior,
                    box.getInverseRotationTranslation(),
                    glm::vec4(box.getScale(), 0.f), 0));

                export_tasks.push_back(task);
            }
        }
    }
    // The output files are based on the clippings
    else if (m_parameters.method == ExportClippingMethod::CLIPPING_SEPARATED)
    {
        std::unordered_set<SafePtr<AClippingNode>> clippings_to_export = graph.getClippingObjects(
            m_parameters.clippingFilter == ExportClippingFilter::ACTIVE,
            m_parameters.clippingFilter == ExportClippingFilter::SELECTED
        );

        for (SafePtr<AClippingNode> clipping : clippings_to_export)
        {
            ReadPtr<AClippingNode> r_clipping = clipping.cget();
            if (!r_clipping)
                continue;

            ExportTask task;
            task.file_name = r_clipping->getComposedName();

            task.header.name = r_clipping->getComposedName();
            task.header.precision = m_parameters.encodingPrecision;
            task.header.format = common_format;

            task.transfo = (TransformationModule)(*&r_clipping);
            task.transfo.setScale(glm::dvec3(1.0));

            task.input_pcs = pcInfos;
            r_clipping->pushClippingGeometries(task.clippings, *&r_clipping);

            export_tasks.push_back(task);
        }
    }
    // The output files are based on the scans
    else if (m_parameters.method == ExportClippingMethod::CLIPPING_AND_SCAN_MERGED)
    {
        ExportTask task;
        task.file_name = m_parameters.fileName.wstring();

        task.header.name = m_parameters.fileName.wstring();
        task.header.precision = m_parameters.encodingPrecision;
        task.header.format = common_format;

        task.transfo = getBestTransformation(clipping_assembly, pcInfos);

        task.input_pcs = pcInfos;
        task.clippings = clipping_assembly;

        export_tasks.push_back(task);
    }
    // Scan separated (with or without clipping)
    else
    {
        for (const tls::PointCloudInstance& pcInfo : pcInfos)
        {
            // Filter out the tls that we can copy
            if (m_parameters.outFileType == FileType::TLS &&
                pcInfo.header.precision == m_parameters.encodingPrecision)
                continue;

            ExportTask task;
            task.file_name = pcInfo.header.name + (m_forSubProject ? L"" : L"_clipped");

            task.header = pcInfo.header;
            task.header.name = task.header.name + (m_forSubProject ? L"" : L"_clipped");
            task.header.precision = m_parameters.encodingPrecision;
            task.header.format = pcInfo.header.format;

            task.transfo = pcInfo.transfo;

            task.input_pcs = { pcInfo };
            task.clippings = clipping_assembly;

            export_tasks.push_back(task);
        }
    }
}

void ContextExportPC::prepareCopyTasks(Controller& controller, std::vector<CopyTask>& copy_tasks)
{
    if (m_parameters.outFileType != FileType::TLS)
        return;

    bool is_scan = m_exportScans;
    bool is_pco = m_exportPCOs;
    ObjectStatusFilter status = m_parameters.pointCloudFilter;
    // This is the same function as 'getPointCloudInstances' but with 'APointCloudNode'
    std::unordered_set<SafePtr<APointCloudNode>> point_clouds = controller.cgetGraphManager().getNodesOnFilter<APointCloudNode>([is_scan, is_pco, status](ReadPtr<AGraphNode>& node)
        {
            bool verifType = is_scan && node->getType() == ElementType::Scan
                || is_pco && node->getType() == ElementType::PCO;
            bool verifState = (status == ObjectStatusFilter::ALL ||
                (status == ObjectStatusFilter::VISIBLE && node->isVisible()) ||
                (status == ObjectStatusFilter::SELECTED && node->isSelected()));
            return verifType && verifState;
        }
    );

    for (const SafePtr<APointCloudNode>& pc : point_clouds)
    {
        ReadPtr<APointCloudNode> r_pc = pc.cget();
        if (!r_pc)
            continue;

        tls::ScanHeader header;
        tlGetScanHeader(r_pc->getScanGuid(), header);

        if (header.precision == m_parameters.encodingPrecision)
        {
            CopyTask task;
            task.src_path = r_pc->getCurrentScanPath();

            std::wstring name = r_pc->getType() == ElementType::Scan ? r_pc->getName() : r_pc->getComposedName();
            task.dst_path = m_parameters.outFolder / (name + L".tls");

            glm::dvec3 pos = r_pc->getCenter() + m_scanTranslationToAdd;
            glm::dquat rot = r_pc->getOrientation();
            task.dst_transfo = { { rot.x, rot.y, rot.z, rot.w }, { pos.x, pos.y, pos.z } };

            copy_tasks.push_back(task);
        }
    }
}

void ContextExportPC::logStart(Controller& controller, size_t total_steps)
{
    process_time_ = std::chrono::steady_clock::now();
    total_steps_ = total_steps;
    current_step_ = 0;

    QString title_text;
    QString state_text;
    switch (m_parameters.clippingFilter)
    {
    case ExportClippingFilter::SELECTED:
    case ExportClippingFilter::ACTIVE:
        title_text = TEXT_EXPORT_CLIPPING_TITLE_PROGESS;
        state_text = TEXT_SPLASH_SCREEN_SCAN_PROCESSING;
        break;
    case ExportClippingFilter::GRIDS:
        title_text = TEXT_EXPORT_GRID_TITLE_PROGESS;
        state_text = TEXT_SPLASH_SCREEN_BOX_PROCESSING;
        break;
    case ExportClippingFilter::NONE:
        title_text = TEXT_EXPORT_TITLE_NORMAL;
        state_text = TEXT_SPLASH_SCREEN_SCAN_PROCESSING;
        break;
    }

    controller.updateInfo(new GuiDataProcessingSplashScreenStart(total_steps_, title_text, state_text.arg(0).arg(total_steps_)));
}

void ContextExportPC::logProgress(Controller& controller)
{
    current_step_++;

    QString state_text;
    switch (m_parameters.clippingFilter)
    {
    case ExportClippingFilter::SELECTED:
    case ExportClippingFilter::ACTIVE:
        state_text = m_parameters.method == ExportClippingMethod::CLIPPING_SEPARATED ? TEXT_SPLASH_SCREEN_CLIPPING_PROCESSING : TEXT_SPLASH_SCREEN_SCAN_PROCESSING;
        break;
    case ExportClippingFilter::GRIDS:
        state_text = TEXT_SPLASH_SCREEN_BOX_PROCESSING;
        break;
    case ExportClippingFilter::NONE:
        state_text = TEXT_SPLASH_SCREEN_SCAN_PROCESSING;
        break;
    }

    controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(state_text.arg(current_step_).arg(total_steps_), current_step_));
}

void ContextExportPC::logEnd(Controller& controller, bool success)
{
    float time = std::chrono::duration<float, std::ratio<1>>(std::chrono::steady_clock::now() - process_time_).count();

    QString log_text;
    switch (m_parameters.clippingFilter)
    {
    case ExportClippingFilter::SELECTED:
    case ExportClippingFilter::ACTIVE:
        log_text = success ? TEXT_EXPORT_CLIPPING_SUCCESS_TIME : TEXT_EXPORT_CLIPPING_ERROR_TIME;
        break;
    case ExportClippingFilter::GRIDS:
    case ExportClippingFilter::NONE:
        log_text = success ? TEXT_EXPORT_SUCCESS_TIME : TEXT_EXPORT_ERROR_TIME;
        break;
    }

    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(log_text.arg(time)));
    controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));
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
    std::vector<tls::PointCloudInstance> pcInfos = getPointCloudInstances(controller);
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
        controller.updateInfo(new GuiDataProcessingSplashScreenStart(boxes_count, TEXT_EXPORT_GRID_TITLE_PROGESS, TEXT_SPLASH_SCREEN_BOX_PROCESSING.arg(0).arg(boxes_count)));

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
                glm::vec4(box.getScale(), 0.f), 0));

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

                resultOk &= overseer.clipScan(pcInfo.header.guid, pcInfo.transfo, clipAssembly, scanFileWriter.get()); // [old] merging == true
            }
            scanFileWriter->finalizePointCloud();
            boxesExported++; 
            controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(TEXT_SPLASH_SCREEN_BOX_PROCESSING.arg(boxesExported).arg(boxes_count), boxesExported));

            dstScanHeader.pointCount = scanFileWriter->getScanPointCount();
            glm::vec3 scale = box.getScale();
            dstScanHeader.bbox = { -scale.x, scale.x , -scale.y, scale.y, -scale.z, scale.z };
            writeHeaderInCSV(&csvWriter, dstScanHeader);

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
    fileWriter->finalizePointCloud();

    // name;point count;origin;size;orientation
    csvWriter << "box_origin;8;0.0;0.0;0.0;1.0;1.0;1.0;0;0;0" << CSVWriter::endl;
}

bool ContextExportPC::ensureFileWriter(Controller& controller, std::unique_ptr<IScanFileWriter>& scanFileWriter, std::wstring name, CSVWriter* csvWriter)
{
    if (scanFileWriter != nullptr &&
        scanFileWriter->getFilePath().stem() == name)
    {
        return true;
    }
    else
        scanFileWriter.reset();

    std::wstring log;
    IScanFileWriter* ptr;
    if (getScanFileWriter(m_parameters.outFolder, name, m_parameters.outFileType, log, &ptr) == false)
    {
        FUNCLOG << "Export: Failed to create the destination file" << LOGENDL;
        controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString(TEXT_EXPORT_ERROR_FILE).arg(QString::fromStdWString(m_parameters.outFolder / name))));
        return false;
    }
    else
        scanFileWriter.reset(ptr);

    if (m_parameters.outFileType == FileType::RCP)
        static_cast<RcpFileWriter*>(scanFileWriter.get())->setExportDensity(m_parameters.pointDensity);
    if ((m_parameters.outFileType == FileType::RCP) && m_parameters.addOriginCube && csvWriter != nullptr)
        addOriginCube(scanFileWriter.get(), tls::PointFormat::TL_POINT_XYZ_I_RGB, *csvWriter);

    return true;
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

std::vector<tls::PointCloudInstance> ContextExportPC::getPointCloudInstances(Controller& controller)
{
    if (m_selectedPcs.empty())
    {
        IOLOG << "Get all PCInstance, exportScan ? " << m_exportScans << " exportPcos ? " << m_exportPCOs << " scanFilter ? " << (int)m_parameters.pointCloudFilter << LOGENDL;
        return (controller.cgetGraphManager().getPointCloudInstances(xg::Guid(), m_exportScans, m_exportPCOs, m_parameters.pointCloudFilter));
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
            pcis.emplace_back(header, rPc->getTransformationModule(), rPc->getClippable());
        }
        return pcis;
    }
}

TransformationModule ContextExportPC::getBestTransformation(const ClippingAssembly& clipping_assembly, const std::vector<tls::PointCloudInstance>& pc_instances)
{
    TransformationModule best_transfo;

    BoundingBoxD scan_bbox;
    scan_bbox.setEmpty();

    for (const tls::PointCloudInstance& pc : pc_instances)
    {
        BoundingBoxD&& t_bbox = pc.header.bbox.transform(pc.transfo.getTransformation());
        scan_bbox.extend(t_bbox);
    }

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

BoundingBoxD ContextExportPC::extractBBox(const IClippingGeometry& clippingGeom)
{
    // NOTE(robin) - Ceci ne fonctionne que pour les clippings de type box
    float x = (float)clippingGeom.params.x;
    float y = (float)clippingGeom.params.y;
    float z = (float)clippingGeom.params.z;

    BoundingBoxD bbox = { -x, x, -y, y, -z, z };

    return bbox.transform(glm::inverse(clippingGeom.matRT_inv));
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
{}

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
    m_parameters.clippingFilter = ExportClippingFilter::ACTIVE;
    m_parameters.method = ExportClippingMethod::SCAN_SEPARATED;

    processExport(controller, nullptr);

    exportObjects(controller);

    return m_state = ContextState::done;
}

bool ContextExportSubProject::canAutoRelaunch() const
{
    return false;
}

void ContextExportSubProject::exportObjects(Controller& controller) const
{
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
