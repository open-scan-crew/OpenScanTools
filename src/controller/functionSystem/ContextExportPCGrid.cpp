#include "controller/functionSystem/ContextExportPCGrid.h"
#include "controller/Controller.h"
#include "controller/controls/ControlProject.h"
#include "controller/messages/ClippingExportParametersMessage.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "pointCloudEngine/PCE_core.h"
#include "pointCloudEngine/PCE_compute.h"
#include "io/exports/IScanFileWriter.h"
#include "io/exports/RcpFileWriter.h"
#include "models/3d/GridCalculation.h"
#include "models/pointCloud/PointXYZIRGB.h"
#include "utils/Utils.h"
#include "utils/math/trigo.h"
#include "io/exports/CSVWriter.h"

ContextExportPCGrid::ContextExportPCGrid(const ContextId& id)
    : ProcessingContext(id)
{
    m_state = ContextState::waiting_for_input;
}

ContextExportPCGrid::~ContextExportPCGrid()
{}

ContextState ContextExportPCGrid::start(Controller& controller)
{
    std::set<dataId> gridSelected = controller.getContext().getGridsSelected();
    std::vector<tls::ScanGuid> scans = controller.getContext().getVisibleScans();

    // Check that at least one clipping box is selected.
    if (gridSelected.empty())
    {
        FUNCLOG << "No Grid boxes selected" << LOGENDL;
        controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_GRID_SELECT_FIRST));
        return (m_state = ContextState::abort);
    }

    if (scans.empty())
    {
        FUNCLOG << "No Scans visibles to export" << LOGENDL;
        controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_NO_SCAN_SELECTED));
        return (m_state = ContextState::abort);
    }

    controller.updateInfo(new GuiDataGridExportParametersDisplay());
    controller.actualizeClippings();

    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextExportPCGrid::feedMessage(IMessage* message, Controller& controller)
{
    switch (message->getType())
    {
    case IMessage::MessageType::CLIPPING_EXPORT_PARAMETERS:
    {
        auto decodedMsg = static_cast<ClippingExportParametersMessage*>(message);
        m_parameters = decodedMsg->m_parameters;
        m_state = ContextState::ready_for_using;
        break;
    }
    break;
    default:
    {
        FUNCLOG << "wrong message type" << LOGENDL;
        //Note (Aurélien) : Not sure that aborting is the best thing to do
        m_state = ContextState::abort;
    }
    }

    return (m_state);
}

void addOriginCube(IScanFileWriter* fileWriter, tls::PointFormat pointFormat)
{
    tls::ScanHeader scanHeader;
    scanHeader.name = "box_origin";
    scanHeader.transfo = tls::Transformation{ {0., 0., 0., 1.}, {0., 0., 0.} };
    scanHeader.precision = tls::PrecisionType::TL_OCTREE_100UM; // not used
    scanHeader.pointCount = 0;
    scanHeader.format = pointFormat;

    fileWriter->appendPointCloud(scanHeader);
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
}

ContextState ContextExportPCGrid::process(Controller& controller)
{
    auto start = std::chrono::steady_clock::now();
    // Create the clipping info lists
    std::vector<UIGrid> uiGrids = controller.getContext().getUIGrids();
    std::map<std::string, std::deque<GridBox>> grids;
    std::map<std::string, glm::dvec3> gridsSize;
    GridCalculation gridCalc;
    uint64_t totalBoxes = 0;
    for (UIGrid& uiGrid : uiGrids)
    {
        // Only the Grid selected
        if (uiGrid.isSelected() == false)
            continue;

        // Only the Grid activated if the parameter say so
        if (m_parameters.source == ExportClippingSource::ACTIVE_SELECTED &&
            uiGrid.getActive() == false)
            continue;

        const TransformationModule& transfo = uiGrid;
        if (!gridCalc.calculateBoxes(grids[uiGrid.getComposedName()], transfo, uiGrid.getGridDivision(), uiGrid.getGridType()))
        {
             //TO DO error check
            controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_GRID_SELECT_FIRST));
            return (m_state = ContextState::abort);
        }
        totalBoxes += grids[uiGrid.getComposedName()].size();
        gridsSize[uiGrid.getComposedName()] = transfo.getScale();
    }

    // Get the the visible scans
    std::vector<tls::ScanGuid> visibleScansRef = controller.getContext().getVisibleScans();
    tls::PointFormat commonFormat = tls::PointFormat::TL_POINT_FORMAT_UNDEFINED;
    for (tls::ScanGuid scanGuid : visibleScansRef)
    {
        if (scanGuid == tls::ScanGuid())
            continue;
        tls::ScanHeader srcScanHeader;
        tlGetScanHeader(scanGuid, srcScanHeader);
        tls::getCompatibleFormat(commonFormat, srcScanHeader.format);
    }

    controller.updateInfo(new GuiDataProcessingSplashScreenStart(totalBoxes, TEXT_EXPORT_GRID_TITLE_PROGESS));

    // TODO - Treat the particular case of converting a TLS to a TLS with the same precision.

    // Processing
    bool resultOk = true;
    bool fileIsProject = (m_parameters.outFileType == FileType::RCP);
    std::wstring log;
    IScanFileWriter* scanFileWriter = nullptr;
    for (const auto& grid : grids)
    {
        if (!m_isRunning)
        {
            m_state = ContextState::abort;
            break;
        }
        std::filesystem::path path = m_parameters.outFolder / grid.first;
        if (Utils::System::createDirectoryIfNotExist(path) == false)
        {
            resultOk = false;
            break;
        }
        uint64_t boxesExported = 0;
        uint64_t subProjectCount = 0;
        CSVWriter csvWriter(path / "summary.csv");
        csvWriter << "Name;Point_Count;X;Y;Z;SizeX;SizeY;SizeZ;RotationX;RotationY;RotationZ" << CSVWriter::endl;
        for(const GridBox& boxe : grid.second)
        {
            if (!m_isRunning)
            {
                m_state = ContextState::abort;
                break;
            }
            std::ostringstream stringStream;
            stringStream << grid.first << "_" << Utils::completeWithZeros(boxe.position.x) << "_" << Utils::completeWithZeros(boxe.position.y) << "_" << Utils::completeWithZeros(boxe.position.z);
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::wstring pointCloudName = converter.from_bytes(stringStream.str());
            std::wstring gridWName = converter.from_bytes(grid.first);

            // Create a Writer per scan or per project or per N boxes
            if (scanFileWriter == nullptr)
            {
                std::wstring outFileName = fileIsProject ? gridWName : pointCloudName;
                if (fileIsProject && m_parameters.maxScanPerProject > 0)
                {
                    subProjectCount++;
                    outFileName += L"_";
                    outFileName += std::to_wstring(subProjectCount);
                }
                if (getScanFileWriter(path, outFileName, m_parameters.outFileType, log, &scanFileWriter) == false)
                {
                    FUNCLOG << "Export: Failed to create the destination file" << LOGENDL;
                    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString(TEXT_EXPORT_ERROR_FILE).arg(outFileName)));
                    resultOk = false;
                    m_isRunning = false;
                    break;
                }
                if (m_parameters.outFileType == FileType::RCP)
                    static_cast<RcpFileWriter*>(scanFileWriter)->setExportDensity(m_parameters.pointDensity);
                if ((m_parameters.outFileType == FileType::RCP) && m_parameters.addOriginCube)
                    addOriginCube(scanFileWriter, commonFormat);
            }

            tls::ScanHeader dstScanHeader;
            dstScanHeader.name = stringStream.str();
            dstScanHeader.transfo = tls::Transformation{ {0, 0, 0, 1}, {boxe.getCenter().x, boxe.getCenter().y, boxe.getCenter().z} };
            // Initialize precision and point count
            dstScanHeader.precision = m_parameters.encodingPrecision;
            dstScanHeader.pointCount = 0;
            dstScanHeader.format = commonFormat;

            scanFileWriter->appendPointCloud(dstScanHeader);
            for (tls::ScanGuid scanGuid : visibleScansRef)
            {
                if (scanGuid == tls::ScanGuid())
                    continue;
                if (!m_isRunning)
                {
                    m_state = ContextState::abort;
                    break;
                }
                resultOk &= tlClipScan(scanGuid, { boxe.getInverseTransformation() }, {}, scanFileWriter, true);
            }
            scanFileWriter->flushWrite();
            boxesExported++;
            controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(QString(), boxesExported));
            uint64_t pointCount(scanFileWriter->getScanPointCount());
            csvWriter.writeValue(stringStream.str());
            csvWriter.writeValue(pointCount);
            csvWriter.writeValue(boxe.getCenter().x);
            csvWriter.writeValue(boxe.getCenter().y);
            csvWriter.writeValue(boxe.getCenter().z);
            csvWriter.writeValue(boxe.getScale().x * 2.0);
            csvWriter.writeValue(boxe.getScale().y * 2.0);
            csvWriter.writeValue(boxe.getScale().z * 2.0);
            glm::dvec3 eulers(tls::math::quat_to_euler_zyx_deg(boxe.getOrientation()));
            csvWriter.writeValue(eulers.x);
            csvWriter.writeValue(eulers.y);
            csvWriter.writeValue(eulers.z);
            csvWriter.endLine();
            
            if (fileIsProject == false ||
                (m_parameters.maxScanPerProject > 0 && (boxesExported % m_parameters.maxScanPerProject) == 0))
            {
                delete scanFileWriter;
                scanFileWriter = nullptr;
            }
        }
    }

    if (scanFileWriter != nullptr)
    {
        delete scanFileWriter;
        scanFileWriter = nullptr;
    }

    float time = std::chrono::duration<float, std::ratio<1>>(std::chrono::steady_clock::now() - start).count();
    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString(resultOk ? TEXT_EXPORT_SUCCESS_TIME : TEXT_EXPORT_ERROR_TIME).arg(time)));
    controller.updateInfo(new GuiDataProcessingSplashScreenEnd());

    m_state = resultOk ? ContextState::done : ContextState::abort;
    return (m_state);
}

ContextState ContextExportPCGrid::validate(Controller& controller)
{
    return (m_state);
}

bool ContextExportPCGrid::isBackground() const
{
    return (false);
}

bool ContextExportPCGrid::canAutoRelaunch() const
{
    return (false);
}

void ContextExportPCGrid::stop()
{
    // Here, abort or done make no difference.
    m_state = ContextState::done;
}

void ContextExportPCGrid::kill()
{
    // Here, abort or done make no difference.
    m_state = ContextState::abort;
}

ContextType ContextExportPCGrid::getType() const
{
    return (ContextType::exportPCGrid);
}

AContext* ContextExportPCGrid::clone()
{
    ContextExportPCGrid* context = new ContextExportPCGrid(*this);
    return (context);
}