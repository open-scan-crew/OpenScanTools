#include "controller/functionSystem/ContextPCOCreation.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "controller/controls/ControlPCObject.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/IControlListener.h"
#include "io/exports/IScanFileWriter.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataClipping.h"
#include "gui/GuiData/GuiDataContextRequest.h"
#include "gui/texts/ExportTexts.hpp"
#include "gui/texts/SplashScreenTexts.hpp"
#include "gui/texts/PointCloudTexts.hpp"

#include "models/graph/CameraNode.h"
#include "models/graph/ScanNode.h"
#include "models/graph/BoxNode.h"
#include "models/graph/GraphManager.h"

#include "controller/messages/PointCloudObjectCreationParametersMessage.h"
#include "controller/messages/CameraMessage.h"
#include "utils/Logger.h"

#include "tls_impl.h"

ContextPCOCreation::ContextPCOCreation(const ContextId& id)
    : AContext(id)
{}

ContextPCOCreation::~ContextPCOCreation()
{}

ContextState ContextPCOCreation::start(Controller& controller)
{
    // Ask for camera infos
    controller.updateInfo(new GuiDataContextRequestActiveCamera(m_id));
    // Create the clipping info lists
    GraphManager& graphManager = controller.getGraphManager();

    std::unordered_set<SafePtr<AGraphNode>> selectBoxes = graphManager.getNodesByTypes({ ElementType::Box , ElementType::Grid }, ObjectStatusFilter::SELECTED);
    if (selectBoxes.empty() || selectBoxes.size() != 1)
        return (m_state = ContextState::abort);

    ReadPtr<BoxNode> clipping = static_pointer_cast<BoxNode>(*selectBoxes.begin()).cget();
    if (!clipping)
    {
        FUNCLOG << "ContextPCOCreation failed do find clippingbox " << LOGENDL;
        return (m_state = ContextState::abort);
    }

    point_cloud_transfo_ = (TransformationModule)(*&clipping);
    point_cloud_transfo_.setScale(glm::dvec3(1.0, 1.0, 1.0));

    clipping_assembly_ = ClippingAssembly();
    clipping->pushClippingGeometries(clipping_assembly_, point_cloud_transfo_);

    // On force la création des PCO sur l'intérieur des clippings.
    // C'est sûrement discutable, on pourrait avoir envie un jour de créer un PCO à partir d'une clipping extèrieure.
    clipping_assembly_.clippingUnion.insert(clipping_assembly_.clippingUnion.end(), clipping_assembly_.clippingIntersection.begin(), clipping_assembly_.clippingIntersection.end());
    clipping_assembly_.clippingIntersection.clear();
    for (auto geom : clipping_assembly_.clippingUnion)
    {
        geom->mode = ClippingMode::showInterior;
    }
    m_parameters.fileName = clipping->getName();
    controller.updateInfo(new GuiDataPointCloudObjectDialogDisplay(m_parameters));
	return (m_state = ContextState::waiting_for_input);
}

ContextState ContextPCOCreation::feedMessage(IMessage* message, Controller& controller)
{
    switch (message->getType())
    {
    case IMessage::MessageType::CAMERA:
    {
        CameraMessage* cameraInfo = static_cast<CameraMessage*>(message);
        ReadPtr<CameraNode> rCam = cameraInfo->m_cameraNode.cget();
        if (!rCam)
            break;

        m_panoramic = xg::Guid();
        ReadPtr<ScanNode> rScan = rCam->getPanoramicScan().cget();
        if (!rScan)
            break;
        m_panoramic = rScan->getScanGuid();
    }
    break;
    case IMessage::MessageType::PCO_CREATION_PARAMETERS:
        {
            auto mes = static_cast<PointCloudObjectCreationParametersMessage*>(message);
            m_parameters = mes->m_parameters;
            std::wstring filename(m_parameters.fileName + L".tls");
            if (!std::filesystem::exists(controller.getContext().cgetProjectInternalInfo().getObjectsFilesFolderPath() / filename))
                return (m_state = ContextState::ready_for_using);
            controller.updateInfo(new GuiDataWarning(TEXT_POINT_CLOUD_OBJECT_FILE_EXIST));
            return start(controller);
        }
    }
	return (m_state = ContextState::waiting_for_input);
}

ContextState ContextPCOCreation::launch(Controller& controller)
{
    m_state = ContextState::running;
    auto start = std::chrono::steady_clock::now();
    GraphManager& graphManager = controller.getGraphManager();

    // Get the the visible scans
    std::vector<tls::PointCloudInstance> scanInstances = graphManager.getVisiblePointCloudInstances(m_panoramic, true, false);

    controller.updateInfo(new GuiDataProcessingSplashScreenStart(scanInstances.size(), TEXT_EXPORT_CLIPPING_TITLE_PROGESS, TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(0).arg(scanInstances.size())));

    // Processing
    bool resultOk = true;
    std::wstring log;
    uint64_t scanExported = 0;
  
    // Create a common file writer for each scan
    IScanFileWriter* scanFileWriter;
    std::filesystem::path dirPath = controller.getContext().cgetProjectInternalInfo().getObjectsFilesFolderPath();

    std::filesystem::path converter(m_parameters.fileName);
    std::wstring clippingName(converter.wstring());
    if (getScanFileWriter(dirPath, clippingName, FileType::TLS, log, &scanFileWriter) == false)
    {
        FUNCLOG << "Export: Failed to create the destination file" << LOGENDL;
        return (m_state = ContextState::abort);
    }

    tls::ScanHeader dstScanHeader;
    dstScanHeader.name = m_parameters.fileName;
    // The bounding box should be calculated by the scan writer (for the TLS it is the octreeCtor, for the e57 ??)
    dstScanHeader.limits;
    // Initialize precision and point count
    dstScanHeader.precision = tls::PrecisionType::TL_OCTREE_100UM;
    dstScanHeader.pointCount = 0;
    // Get a compliant format for all the scans
    dstScanHeader.format = tls::PointFormat::TL_POINT_FORMAT_UNDEFINED;
    for (tls::PointCloudInstance pcInst : scanInstances)
    {
        tls::getCompatibleFormat(dstScanHeader.format, pcInst.header.format);
    }

    scanFileWriter->appendPointCloud(dstScanHeader, point_cloud_transfo_);

    for (tls::PointCloudInstance pcInst : scanInstances)
    {
        glm::dmat4 modelMatrix = pcInst.transfo.getTransformation();
        resultOk &= TlScanOverseer::getInstance().clipScan(pcInst.header.guid, modelMatrix, clipping_assembly_, scanFileWriter); // [old] merging == true
        scanExported++;
        controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(scanExported).arg(scanInstances.size()), scanExported));
    }
    scanFileWriter->finalizePointCloud();
    uint64_t pointCount(scanFileWriter->getTotalPoints());
    std::filesystem::path newScanPath = scanFileWriter->getFilePath();
    delete scanFileWriter;
    
    float time = std::chrono::duration<float, std::ratio<1>>(std::chrono::steady_clock::now() - start).count();
    if (resultOk)
    {
        controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString(TEXT_EXPORT_SUCCESS_TIME).arg(time)));
    }
    else
    {
        controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(TEXT_EXPORT_ERROR));
        controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString(TEXT_EXPORT_ERROR_TIME).arg(time)));
    }

    controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));
    if(!pointCount)
        return (m_state = ContextState::done);

    controller.getControlListener()->notifyUIControl(new control::pcObject::CreatePCObjectFromFile({ newScanPath }));

    return (m_state = ContextState::done);
}

bool ContextPCOCreation::canAutoRelaunch() const
{
	return (false);
}

ContextType ContextPCOCreation::getType() const
{
	return (ContextType::pointCloudObjectCreation);
}
