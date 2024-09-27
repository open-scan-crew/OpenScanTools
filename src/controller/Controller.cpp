#include "controller/Controller.h"
#include "controller/Controller_p.h"
#include "io/SaveLoadSystem.h"
#include "gui/Texts.hpp"
#include "gui/GuiData/GuiDataGeneralProject.h" //
#include "gui/GuiData/GuiDataTag.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataTree.h"
#include "gui/GuiData/GuiDataTemplate.h" //
#include "gui/GuiData/GuiDataMessages.h"
#include "controller/controls/ControlApplication.h"
#include "controller/controls/ControlTagEdition.h"
#include "controller/controls/ControlScanEdition.h"
#include "controller/controls/ControlPicking.h"
#include "controller/controls/ControlTree.h"
#include "controller/controls/ControlProject.h"
#include "controller/ControllerContext.h"
#include "controls/ControlMetaControl.h"
#include "pointCloudEngine/RenderingTypes.h"

//~~~~~~~~~~~~~~~~ Data Model ~~~~~~~~~~~~~~~~~//

#include "models/3d/Graph/TagNode.h"
#include "models/3d/Graph/CameraNode.h"
#include "models/3d/Graph/CylinderNode.h"
#include "models/3d/Graph/TorusNode.h"
#include "models/3d/Graph/SphereNode.h"
#include "models/3d/Graph/MeshObjectNode.h"
#include "models/3d/Graph/BeamBendingMeasureNode.h"
#include "models/3d/Graph/ColumnTiltMeasureNode.h"
#include "models/3d/Graph/ScanObjectNode.h"
#include "models/3d/Graph/ScanNode.h"
#include "models/3d/Graph/ViewPointNode.h"
#include "models/3d/Graph/BoxNode.h"
#include "models/3d/Graph/ClusterNode.h"
#include "models/3d/Graph/PointNode.h"
#include "models/3d/Graph/SimpleMeasureNode.h"
#include "models/3d/Graph/PolylineMeasureNode.h"
#include "models/3d/Graph/PipeToPipeMeasureNode.h"
#include "models/3d/Graph/PipeToPlaneMeasureNode.h"
#include "models/3d/Graph/PointToPlaneMeasureNode.h"
#include "models/3d/Graph/PointToPipeMeasureNode.h"

#include "utils/Logger.h"
#include <magic_enum/magic_enum.hpp>

#define CONTROLLERLOG Logger::log(LoggerMode::ControllerLog)

Controller::Controller(IDataDispatcher& dataDispatcher, OpenScanToolsGraphManager& graphManager)
	: m_p(new Controller_p(dataDispatcher, graphManager, *this))
{
	m_p->dataDispatcher.InitializeControlListener(&m_p->controlListener);

	SaveLoadSystem::ErrorCode errorMsg = SaveLoadSystem::ErrorCode::Success;
	m_p->context.addLocalAuthors(SaveLoadSystem::loadLocalAuthors(*this, errorMsg));
	assert(errorMsg == SaveLoadSystem::ErrorCode::Success);

	m_p->dataDispatcher.updateInformation(new GuiDataSendAuthorsList(m_p->context.getLocalAuthors(), -1));
}

Controller::~Controller()
{
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
    getOpenScanToolsGraphManager().replaceObjectsSelected(newSelection);
	
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

void Controller::saveCurrentProject(const SafePtr<CameraNode>& camera)
{
	bool saveDone;
    if ((saveDone = !SaveLoadSystem::ExportProject(*this, getOpenScanToolsGraphManager().getProjectNodes(), getContext().cgetProjectInternalInfo(), getContext().getProjectInfo(), camera).empty()))
    {
		SaveLoadSystem::ErrorCode code;
		SaveLoadSystem::ExportTemplates(m_p->context.getTemplates(), code, m_p->context.cgetProjectInternalInfo().getTemplatesFolderPath() / File_Templates);
		SaveLoadSystem::ExportLists<UserList>(m_p->context.getUserLists(), m_p->context.cgetProjectInternalInfo().getTemplatesFolderPath() / File_Lists);
		m_p->context.setIsCurrentProjectSaved(true);
    }
    else
    {
		m_p->context.setIsCurrentProjectSaved(false);
        updateInfo(new GuiDataWarning(TEXT_ERROR_PROJECT_DIRECTORY_DELETE));
        CONTROLLOG << "Controller Save [" << m_p->context.cgetProjectInfo().m_projectName << "] error : project not save" << LOGENDL;
    }
}

/*void Controller::autosaveCurrentProject(const SafePtr<CameraNode>& camera)
{
	if (m_p->context.getIsCurrentProjectSaved())
		return;
	if (SaveLoadSystem::ExportProject(*this, getOpenScanToolsGraphManager().getProjectNodes(), getContext().cgetProjectInternalInfo(), getContext().getProjectInfo(), camera).empty())
		CONTROLLOG << "Controller Autosave [" << m_p->context.cgetProjectInfo().m_projectName << "] error : project not save" << LOGENDL;
	else
	{
		SaveLoadSystem::ErrorCode code;
		SaveLoadSystem::ExportTemplates(m_p->context.getTemplates(), code, m_p->context.cgetProjectInternalInfo().getTemplatesFolderPath() / File_Templates, true);
		SaveLoadSystem::ExportLists<UserList>(m_p->context.getUserLists(), m_p->context.cgetProjectInternalInfo().getTemplatesFolderPath() / File_Lists, true);
	}
}*/

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

OpenScanToolsGraphManager& Controller::getOpenScanToolsGraphManager()
{
	return (m_p->graphManager);
}

const OpenScanToolsGraphManager& Controller::cgetOpenScanToolsGraphManager() const
{
	return (m_p->graphManager);
}

const ControllerContext& Controller::cgetContext() const
{
	return (m_p->context);
}

bool Controller::startAutosaveThread(const uint64_t& timing)
{
	return (m_p->startAutosaveThread(timing, *this));
}

bool Controller::stopAutosaveThread()
{
	return (m_p->stopAutosaveThread());
}

void Controller::startScantraInterface()
{
	m_p->scantra_interface_.startInterface();
}

void Controller::stopScantraInterface()
{
	m_p->scantra_interface_.stopInterface();
}

uint32_t Controller::getNextUserId(ElementType type) const
{
	return m_p->graphManager.getNextUserId({ type }, m_p->context.getIndexationMethod());
}

std::vector<uint32_t> Controller::getMultipleUserId(ElementType type, int indexAmount) const
{
	return m_p->graphManager.getNextMultipleUserId({ type }, m_p->context.getIndexationMethod(), indexAmount);
}

void Controller::setDefaultAuthor()
{
	Author authData = Author();
	SafePtr<Author> newAuth = getContext().createAuthor(authData);
	getContext().addLocalAuthors({ newAuth });
	getContext().setActiveAuthor(newAuth);
	updateInfo(new GuiDataSendAuthorsList(getContext().getLocalAuthors(), -1));
	updateInfo(new GuiDataAuthorNameSelection(authData.getName()));
}
