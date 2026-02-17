#include "controller/functionSystem/ContextDeletePoints.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/IControlListener.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/messages/CameraMessage.h"
#include "controller/messages/ModalMessage.h"
#include "gui/GuiData/GuiDataContextRequest.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "io/exports/TlsFileWriter.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "pointCloudEngine/PCE_core.h"

#include "models/graph/PointCloudNode.h"
#include "models/graph/GraphManager.h"
#include "models/graph/CameraNode.h"

#include "gui/texts/ExportTexts.hpp"
#include "gui/texts/SplashScreenTexts.hpp"
#include "gui/texts/ContextTexts.hpp"

#include "utils/ColorimetricFilterUtils.h"
#include "utils/Logger.h"

#include <algorithm>

// Note (Aurélien) QT::StandardButtons enum values in qmessagebox.h
#define Yes 0x00004000
#define No 0x00010000
#define Cancel 0x00400000

ContextDeletePoints::ContextDeletePoints(const ContextId& id)
    : AContext(id)
    , m_panoramic(xg::Guid())
{
    m_state = ContextState::waiting_for_input;
    m_warningModal = false;
    m_pendingDeleteConfirmation = false;
    m_waitingSaveModal = false;
    m_hasCameraSettings = false;
}

ContextDeletePoints::~ContextDeletePoints()
{}

ContextState ContextDeletePoints::start(Controller& controller)
{
    m_warningModal = false;
    m_clippingFilter = ExportClippingFilter::ACTIVE;
    m_pendingDeleteConfirmation = true;
    m_waitingSaveModal = false;
    m_hasCameraSettings = false;

    controller.updateInfo(new GuiDataContextRequestActiveCamera(m_id));

    if (controller.getContext().getIsCurrentProjectSaved() == false)
    {
        controller.updateInfo(new GuiDataModal(Yes | No, TEXT_DELETE_SAVE_BEFORE_QUESTION));
        m_waitingSaveModal = true;
        return (m_state = ContextState::waiting_for_input);
    }

    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextDeletePoints::feedMessage(IMessage* message, Controller& controller)
{
    switch (message->getType())
    {
    case IMessage::MessageType::CAMERA:
    {
        CameraMessage* cameraMsg = static_cast<CameraMessage*>(message);
        ReadPtr<CameraNode> rCamera = cameraMsg->m_cameraNode.cget();
        if (!rCamera)
        {
            controller.updateInfo(new GuiDataWarning(TEXT_FILTER_ACTIVATE_FIRST));
            return (m_state = ContextState::abort);
        }

        const DisplayParameters& display = rCamera->getDisplayParameters();
        m_colorimetricFilterSettings = display.m_colorimetricFilter;
        m_polygonalSelectorSettings = display.m_polygonalSelector;
        m_renderMode = display.m_mode;
        m_hasCameraSettings = true;

        if (m_pendingDeleteConfirmation && !m_waitingSaveModal)
        {
            m_pendingDeleteConfirmation = false;
            if (!beginDeleteConfirmation(controller))
                m_state = ContextState::abort;
        }
        break;
    }
    case IMessage::MessageType::MODAL:
    {
        if (!m_warningModal)
        {
            ModalMessage* modal = static_cast<ModalMessage*>(message);
            if (modal->m_returnedValue == Yes)
                m_saveContext = controller.getFunctionManager().launchBackgroundFunction(controller, ContextType::saveProject, m_id);

            m_waitingSaveModal = false;
            if (m_pendingDeleteConfirmation && m_hasCameraSettings)
            {
                m_pendingDeleteConfirmation = false;
                if (!beginDeleteConfirmation(controller))
                    m_state = ContextState::abort;
            }
        }
        else
        {
            ModalMessage* modal = static_cast<ModalMessage*>(message);
            if (modal->m_returnedValue == Yes)
                m_state = ContextState::ready_for_using;
            else
                m_state = ContextState::abort;
        }
    }
    break;
    default:
    {
        FUNCLOG << "Context delete points: wrong message type" << LOGENDL;
        //m_state = ContextState::abort;
    }
    }

    return (m_state);
}

bool ContextDeletePoints::beginDeleteConfirmation(Controller& controller)
{
    GraphManager& graphManager = controller.getGraphManager();

    std::unordered_set<SafePtr<AClippingNode>> clips = graphManager.getClippingObjects(true, false);
    if (clips.empty() && !hasActiveFilter())
    {
        FUNCLOG << "No active clipping boxes or filters" << LOGENDL;
        controller.updateInfo(new GuiDataWarning(TEXT_DELETE_POINTS_NO_ACTIVE_CLIPPING));
        return false;
    }

    if (graphManager.getVisibleScans(m_panoramic).empty())
    {
        FUNCLOG << "No Scans visibles to clean" << LOGENDL;
        controller.updateInfo(new GuiDataWarning(TEXT_EXPORT_NO_SCAN_SELECTED));
        return false;
    }

    m_warningModal = true;
    controller.updateInfo(new GuiDataModal(Yes | Cancel, TEXT_DELETE_POINTS_QUESTION));
    return true;
}


// Input :
// * scans visibles
// * clippings actives et/ou sélectionnées

// Traitement :
// * créer un dossier temporaire pour le traitement "project/scans/temp"
// * stopper le rendu (car les EmbeddedScan vont être remplacés)
// * stopper le streaming (car les fichiers vont être remplacés)

// * pour chaque scan :
//    - créer le même scan tronqué dans le dossier de backup
//    - créer un nouveau EmbeddedScan avec les informations d'octree du nouveau tls
//    - copier les adresses des buffers graphiques entre les EmbeddedScan
//    - réinitialiser les buffers de points tronqués (avec moins de point)
//    - supprimer l'ancien EmbeddedScan et les buffers restant non utilisées (tout les points supprimés)
//    - changer le guid du tls dans le Scan du projet (sauvegardable)

// Questions :
// * Doit-on changer les guid des scan ? --> A priori oui
// * Doit-on changer les dataId des scan ? --> Non, on conserver le xg::Guid  pour les undo/redo avec le scan.

// * supprimer le dossier temp et tout ce qu'il contient (ou pas si on veut un backup)
// * redémarrer le streaming
// * redémarrer le rendu
ContextState ContextDeletePoints::launch(Controller& controller)
{
    GraphManager& graphManager = controller.getGraphManager();

    std::filesystem::path temp_folder = controller.getContext().cgetProjectInternalInfo().getPointCloudFolderPath(false) / "temp";
    prepareOutputDirectory(controller, temp_folder);

    TlStreamLock streamLock;

    // Listing des scan à traiter
    std::unordered_set<SafePtr<PointCloudNode>> scans = graphManager.getVisibleScans(m_panoramic);

    controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString()));
    const uint64_t totalScans = scans.size();
    const uint64_t totalProgressSteps = totalScans * 100;
    controller.updateInfo(new GuiDataProcessingSplashScreenStart(totalProgressSteps, TEXT_EXPORT_CLIPPING_TITLE_PROGESS, TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(0).arg(totalScans)));

    // On récupère les clippings à utiliser depuis le projet
    bool filterActive = (m_clippingFilter == ExportClippingFilter::ACTIVE);
    bool filterSelected = (m_clippingFilter == ExportClippingFilter::SELECTED);
    ClippingAssembly clippingAssembly;
    graphManager.getClippingAssembly(clippingAssembly, filterActive, filterSelected);

    uint64_t scan_count = 0;
    auto updateProgress = [&](uint64_t scansDone, int percent, uint64_t progressValue)
    {
        QString state = QString("%1 - %2%")
                            .arg(TEXT_SPLASH_SCREEN_SCAN_PROCESSING.arg(scansDone).arg(totalScans))
                            .arg(percent);
        controller.updateInfo(new GuiDataProcessingSplashScreenProgressBarUpdate(state, progressValue));
    };
    for (const SafePtr<PointCloudNode>& scan : scans)
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

        size_t initial_point_count = wScan->getNbPoint();
        uint64_t deleted_point_count = 0;
        std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
        std::filesystem::path temp_path;
        tls::ScanGuid old_guid = wScan->getScanGuid();
        tls::ScanGuid new_guid = old_guid;

        const uint64_t currentScanIndex = scan_count;
        TlsFileWriter* tls_writer = nullptr;
        std::wstring log;
        TlsFileWriter::getWriter(temp_folder, wScan->getName(), log, (IScanFileWriter**)&tls_writer);
        if (tls_writer == nullptr)
            continue;

        temp_path = tls_writer->getFilePath();
        tls::ScanHeader header;
        TlScanOverseer::getInstance().getScanHeader(old_guid, header);
        header.guid = xg::newGuid();
        tls_writer->appendPointCloud(header, wScan->getTransformation());

        auto progressCallback = [currentScanIndex, &updateProgress](size_t processed, size_t total)
        {
            if (total == 0)
                return;
            int percent = static_cast<int>((processed * 100) / total);
            percent = std::clamp(percent, 0, 99);
            uint64_t progressValue = currentScanIndex * 100 + static_cast<uint64_t>(percent);
            updateProgress(currentScanIndex, percent, progressValue);
        };

        uint64_t kept_points = 0;
        bool res = TlScanOverseer::getInstance().filterAndWrite(old_guid,
                                                                 (TransformationModule)*&wScan,
                                                                 *clippingToUse,
                                                                 m_colorimetricFilterSettings,
                                                                 m_polygonalSelectorSettings,
                                                                 m_renderMode,
                                                                 tls_writer,
                                                                 kept_points,
                                                                 progressCallback);

        res &= tls_writer->finalizePointCloud();

        if (kept_points < initial_point_count)
        {
            deleted_point_count = initial_point_count - kept_points;
            // Register the tls in the Overseer and get the Guid.
            res &= TlScanOverseer::getInstance().getScanGuid(temp_path, new_guid);
        }
        else
        {
            std::error_code ec;
            std::filesystem::remove(temp_path, ec);
        }

        delete tls_writer;

        // LOG
        scan_count++;
        float seconds = std::chrono::duration<float, std::ratio<1>>(std::chrono::steady_clock::now() - startTime).count();
        QString qScanName = QString::fromStdWString(wScan->getName());
        updateProgress(scan_count, 100, scan_count * 100);

        // Remplace le scan guid dans le model scan du projet.
        if (new_guid != old_guid)
        {
            std::filesystem::path absolutePath = wScan->getTlsFilePath();
            // NOTE - On choisi de ne pas supprimer le fichier au cas où cela soit une mauvaise manipulation.
            TlScanOverseer::getInstance().freeScan_async(old_guid, false);
            wScan->setTlsFilePath(temp_path, false, tls::ScanGuid(), false);
            if (new_guid == xg::Guid())
            {
                controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Scan %1 totally deleted from project.").arg(qScanName)));
            }
            else
            {
                TlScanOverseer::getInstance().copyScanFile_async(new_guid, absolutePath, false, true, true);
                controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("%1 points deleted in scan %2 in %3 seconds.").arg(deleted_point_count).arg(qScanName).arg(seconds)));
            }
        }
        else
        {
            controller.updateInfo(new GuiDataProcessingSplashScreenLogUpdate(QString("Scan %1 not affected by point suppression.").arg(qScanName)));
        }
    }

    controller.updateInfo(new GuiDataProcessingSplashScreenEnd(TEXT_SPLASH_SCREEN_DONE));

    m_state = ContextState::done;
    return (m_state);
}

bool ContextDeletePoints::canAutoRelaunch() const
{
    return (false);
}

ContextType ContextDeletePoints::getType() const
{
    return (ContextType::deletePoints);
}

bool ContextDeletePoints::prepareOutputDirectory(Controller& controller, const std::filesystem::path& folderPath)
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

bool ContextDeletePoints::hasActiveFilter() const
{
    bool hasActiveColorimetricFilter = false;
    if (m_colorimetricFilterSettings.enabled)
    {
        auto ordered = ColorimetricFilterUtils::normalizeSettings(m_colorimetricFilterSettings, m_renderMode);
        for (const auto& entry : ordered)
        {
            if (entry.enabled)
            {
                hasActiveColorimetricFilter = true;
                break;
            }
        }
    }

    const bool hasActivePolygonalFilter = m_polygonalSelectorSettings.enabled
        && m_polygonalSelectorSettings.active
        && m_polygonalSelectorSettings.appliedPolygonCount > 0
        && !m_polygonalSelectorSettings.polygons.empty();

    return hasActiveColorimetricFilter || hasActivePolygonalFilter;
}
