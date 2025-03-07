#include "controller/Controller.h"
#include "controller/Controller_p.h"
#include "controller/ControllerContext.h"
#include "io/SaveLoadSystem.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataAuthor.h"
#include "gui/GuiData/GuiDataMessages.h"

#include "pointCloudEngine/EmbeddedScan.h"

#include "controls/ControlMetaControl.h"
#include "models/graph/CameraNode.h"

#include "utils/FilesAndFoldersDefinitions.h"
#include "utils/Logger.h"

#define CONTROLLERLOG Logger::log(LoggerMode::ControllerLog)

Controller::Controller(IDataDispatcher& dataDispatcher, GraphManager& graphManager)
    : m_p(new Controller_p(dataDispatcher, graphManager, *this))
{
    m_p->dataDispatcher.InitializeControlListener(&m_p->controlListener);

    SaveLoadSystem::ErrorCode errorMsg = SaveLoadSystem::ErrorCode::Success;
    m_p->context.addLocalAuthors(SaveLoadSystem::loadLocalAuthors(*this, errorMsg));
    assert(errorMsg == SaveLoadSystem::ErrorCode::Success);

    m_p->dataDispatcher.updateInformation(new GuiDataSendAuthorsList(m_p->context.getLocalAuthors(), SafePtr<Author>()));
}

Controller::~Controller()
{
    EmbeddedScan::logClipAndWriteTimings();
    Logger::log(LoggerMode::LogConfig) << "Destroying Controller..." << LOGENDL;
    delete m_p;
}

void Controller::actualizeTreeView(const std::unordered_set<SafePtr<AGraphNode>>& toActualizeDatas)
{
    m_p->addTreeViewActualization(toActualizeDatas);
}

void Controller::actualizeTreeView(SafePtr<AGraphNode> toActualizeData)
{
    m_p->addTreeViewActualization(std::unordered_set<SafePtr<AGraphNode>>({ toActualizeData }));
}

void Controller::undoLastAction()
{
    m_p->undoLastControl();
}

void Controller::redoLastAction()
{
    m_p->redoLastControl();
}

void Controller::resetHistoric()
{
    m_p->resetHistoric();
}

void Controller::changeSelection(const std::unordered_set<SafePtr<AGraphNode>>& newSelection, bool updateTree)
{
    // 1. Change the selection status in the model
    getGraphManager().replaceObjectsSelected(newSelection);
    
    // 2.2 Tree Panel
    if(updateTree)
        updateInfo(new GuiDataSelectItems(newSelection));

    // 2.3 Property Panel
    if (newSelection.size() == 1)
        updateInfo(new GuiDataObjectSelected(*newSelection.begin()));
    else
        updateInfo(new GuiDataHidePropertyPanels());
}

IControlListener* Controller::getControlListener()
{
    return (&m_p->controlListener);
}

FunctionManager & Controller::getFunctionManager()
{
    return (m_p->funcManager);
}

void Controller::startMetaControl()
{
    m_p->meta_control_creation_ = true;
}

void Controller::stopMetaControl()
{
    if (!m_p->meta_undo_.empty())
    {
        control::meta::ControlMetaControl* meta = new control::meta::ControlMetaControl(m_p->meta_undo_);
        m_p->meta_undo_.clear();
        if(meta->canUndo())
            m_p->to_undo_.push(meta);
        updateInfo(new GuiDataUndoRedoAble(!m_p->to_undo_.empty(), !m_p->to_redo_.empty()));
    }
    m_p->meta_control_creation_ = false;
}

void Controller::abortMetaControl()
{
    for (AControl* ctrl : m_p->meta_undo_)
        delete ctrl;
    m_p->meta_undo_.clear();
    m_p->meta_control_creation_ = false;
}

#define TEXT_ERROR_PROJECT_DIRECTORY_DELETE QObject::tr("Error : The project couldn't be saved.")

void Controller::saveCurrentProject(const SafePtr<CameraNode>& camera)
{
    std::string err_msg;

    if (SaveLoadSystem::ExportProject(*this, getGraphManager().getProjectNodes(), getContext().cgetProjectInternalInfo(), getContext().getProjectInfo(), camera))
    {
        SaveLoadSystem::ErrorCode code;
        SaveLoadSystem::ExportTemplates(m_p->context.getTemplates(), code, m_p->context.cgetProjectInternalInfo().getTemplatesFolderPath() / File_Templates);
        SaveLoadSystem::ExportLists<UserList>(m_p->context.getUserLists(), m_p->context.cgetProjectInternalInfo().getTemplatesFolderPath() / File_Lists);
        m_p->context.setIsCurrentProjectSaved(true);
    }
    else
    {
        m_p->context.setIsCurrentProjectSaved(false);
        updateInfo(new GuiDataWarning(QString::fromUtf8(err_msg.c_str())));
        CONTROLLOG << "Controller Save [" << m_p->context.cgetProjectInfo().m_projectName << "] error : project not save" << LOGENDL;
    }
}

void Controller::updateInfo(IGuiData* data)
{
    m_p->dataDispatcher.updateInformation(data);
}

IDataDispatcher& Controller::getDataDispatcher() const
{
    return (m_p->dataDispatcher);
}

ControllerContext& Controller::getContext()
{
    return (m_p->context);
}

FilterSystem & Controller::getFilterSystem()
{
    return (m_p->filterSystem);
}

GraphManager& Controller::getGraphManager()
{
    return (m_p->graphManager);
}

const GraphManager& Controller::cgetGraphManager() const
{
    return (m_p->graphManager);
}

const ControllerContext& Controller::cgetContext() const
{
    return (m_p->context);
}

void Controller::activateAutosave(uint64_t period_min)
{
    m_p->activateAutosave(period_min);
}

void Controller::deactivateAutosave()
{
    m_p->deactivateAutosave();
}

void Controller::startScantraInterface()
{
    m_p->scantra_interface_.startInterface();
}

void Controller::stopScantraInterface()
{
    m_p->scantra_interface_.stopInterface();
}

void Controller::scantra_notify_project_created()
{
    const ProjectInfos& proj_infos = m_p->context.cgetProjectInfo();
    const ProjectInternalInfo& proj_internal = m_p->context.cgetProjectInternalInfo();
    m_p->scantra_interface_.project_created(proj_internal.getProjectFolderPath(), proj_infos.m_projectName);
}

void Controller::scantra_notify_project_opened()
{
    const ProjectInternalInfo& proj_internal = m_p->context.cgetProjectInternalInfo();
    m_p->scantra_interface_.project_opened(proj_internal.getProjectFolderPath());
}

uint32_t Controller::getNextUserId(ElementType type) const
{
    return m_p->graphManager.getNextUserId({ type }, m_p->context.getIndexationMethod());
}

std::vector<uint32_t> Controller::getMultipleUserId(ElementType type, int indexAmount) const
{
    return m_p->graphManager.getNextMultipleUserId({ type }, m_p->context.getIndexationMethod(), indexAmount);
}
