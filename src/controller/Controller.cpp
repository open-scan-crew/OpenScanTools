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
	: m_p(new Controller_p(dataDispatcher, graphManager))
	, m_metaContralCreation(false)
	, m_waitingActualizeNodes()
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
	cleanHistory();
    delete m_p;
}

void Controller::run(int refreshPerSecond)
{
    // Guaranted to start the controller only once
    bool valueFalse = false;
    if (m_p->threadWorking.compare_exchange_strong(valueFalse, true) == false)
        return;

    m_p->clockLock.resetClock(refreshPerSecond);
    m_p->continueRun.store(true);

    while (m_p->continueRun.load() == true)
    {
		update();

        m_p->clockLock.frame();
    }

    Logger::log(LoggerMode::LogConfig) << "Controller Thread stop frame per second : " << refreshPerSecond << LOGENDL;
    Logger::log(LoggerMode::LogConfig) << "Nbr frame       : " << m_p->clockLock.getNbrFrame() << LOGENDL;
    Logger::log(LoggerMode::LogConfig) << "tt Time elapsed : " << std::setprecision(6) << m_p->clockLock.getTotalTimeSeconds() << LOGENDL;
    Logger::log(LoggerMode::LogConfig) << "Worked seconds  : " << std::setprecision(6) << m_p->clockLock.getSecondWorked() << LOGENDL;
    Logger::log(LoggerMode::LogConfig) << "Sleeped seconds : " << std::setprecision(6) << m_p->clockLock.getSecondsSleeped() << LOGENDL;
    Logger::log(LoggerMode::LogConfig) << "Average time(%) worked per frame : " << std::setprecision(6) << m_p->clockLock.getSecondWorked() / m_p->clockLock.getTotalTimeSeconds() * 100.0 << "%" << LOGENDL;

    m_p->threadWorking.store(false);

    return;
}

void Controller::stop()
{
    m_p->continueRun.store(false);
}

void Controller::update()
{
	updateControls();
	m_p->funcManager.updateContexts(*this);
}

bool Controller::updateControls()
{
	std::list<AControl*> events = m_p->controlListener.popBlockControls();

	if (events.size() > 0)
	{
		for (AControl* actualControl : events)
		{
			actualControl->doFunction(*this);
			if (actualControl->canUndo() == false)
				delete (actualControl);
			else
			{
				m_p->context.setIsCurrentProjectSaved(false);

				if (m_metaContralCreation)
					m_metaToUndo.push_front(actualControl);
				else
					m_p->toUndo.push(actualControl);
				cleanRedoStack();
				updateInfo(new GuiDataUndoRedoAble(!m_p->toUndo.empty(), !m_p->toRedo.empty()));
			}
		}
		logHistoric();
		applyWaitingActualize();
		return (true);
	}
	return (false);
}


void Controller::logHistoric()
{
	if (!m_p->toUndo.empty())
		Logger::log(LoggerMode::HistoricLog) << "Undo stack size: " << m_p->toUndo.size() << "\tNext undo " << std::to_string((uint64_t)m_p->toUndo.top()->getType()) << LOGENDL;
	if (!m_p->toRedo.empty())
		Logger::log(LoggerMode::HistoricLog) << "Redo stack size: " << m_p->toRedo.size() << "\tNext redo " << std::to_string((uint64_t)m_p->toRedo.top()->getType()) << LOGENDL;
}

void Controller::actualizeNodes(const ActualizeOptions& options, const std::unordered_set<SafePtr<AGraphNode>>& toActualizeDatas)
{
	if (toActualizeDatas.empty())
		return;
	if (m_waitingActualizeNodes.find(options) == m_waitingActualizeNodes.end())
		m_waitingActualizeNodes[options] = std::unordered_set<SafePtr<AGraphNode>>();
	m_waitingActualizeNodes.at(options).insert(toActualizeDatas.begin(), toActualizeDatas.end());
}

void Controller::actualizeNodes(const ActualizeOptions& options, SafePtr<AGraphNode> toActualizeData)
{
	actualizeNodes(options, std::unordered_set<SafePtr<AGraphNode>>({ toActualizeData }));
}

void Controller::applyWaitingActualize()
{
	for (std::pair<const ActualizeOptions&, const std::unordered_set<SafePtr<AGraphNode>>&> actualizePair : m_waitingActualizeNodes)
	{
		const ActualizeOptions& options = actualizePair.first;
		const std::unordered_set<SafePtr<AGraphNode>>& nodes = actualizePair.second;

		// Actualize Tree
		if(options.m_treeActualize)
			updateInfo(new GuiDataTreeActualizeNodes(nodes));
	}

	// Actualize Properties
	if(!m_waitingActualizeNodes.empty())
		updateInfo(new GuiDataObjectSelected());

	m_waitingActualizeNodes.clear();
}

void Controller::undoLastAction()
{
	if (!m_metaContralCreation)
	{
		if (m_p->toUndo.empty())
		{
			updateInfo(new GuiDataUndoRedoAble(false, !m_p->toRedo.empty()));
			return;
		}
		AControl* toUndoControl = m_p->toUndo.top();
		m_p->toUndo.pop();
		toUndoControl->undoFunction(*this);
		m_p->toRedo.push(toUndoControl);
		updateInfo(new GuiDataUndoRedoAble(!m_p->toUndo.empty(), true));
	}
}

void Controller::redoLastAction()
{
	if (!m_metaContralCreation)
	{
		if (m_p->toRedo.empty())
		{
			updateInfo(new GuiDataUndoRedoAble(!m_p->toUndo.empty(), false));
			return;
		}
		AControl* toRedoControl = m_p->toRedo.top();
		m_p->toRedo.pop();
		toRedoControl->redoFunction(*this);
		m_p->toUndo.push(toRedoControl);
		updateInfo(new GuiDataUndoRedoAble(true, !m_p->toRedo.empty()));
	}
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

void Controller::cleanHistory()
{
	while (!m_p->toUndo.empty())
	{
		delete(m_p->toUndo.top());
		m_p->toUndo.pop();
	}
	cleanRedoStack();
}

void Controller::cleanRedoStack()
{
	while (!m_p->toRedo.empty())
	{
		delete(m_p->toRedo.top());
		m_p->toRedo.pop();
	}
}

bool Controller::isUndoPossible()
{
	if (m_p->toUndo.empty() || m_metaContralCreation)
		return (false);
	return (true);
}

bool Controller::isRedoPossible()
{
	if (m_p->toRedo.empty() || m_metaContralCreation)
		return (false);
	return (true);
}

void Controller::startMetaControl()
{
	m_metaContralCreation = true;
}

void Controller::stopMetaControl()
{
	if (!m_metaToUndo.empty())
	{
		control::meta::ControlMetaControl* meta = new control::meta::ControlMetaControl(m_metaToUndo);
		m_metaToUndo.clear();
		if(meta->canUndo())
			m_p->toUndo.push(meta);
		updateInfo(new GuiDataUndoRedoAble(!m_p->toUndo.empty(), !m_p->toRedo.empty()));
	}
	m_metaContralCreation = false;
}

void  Controller::abortMetaControl()
{
	for (AControl* ctrl : m_metaToUndo)
		delete ctrl;
	m_metaToUndo.clear();
	m_metaContralCreation = false;
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
