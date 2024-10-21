#include "controller/functionSystem/ContextSaveCloseLoadProject.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h" // forward declaration
#include "controller/messages/FilesMessage.h"
#include "controller/messages/ModalMessage.h"
#include "controller/messages/NewProjectMessage.h"
#include "controller/messages/CameraMessage.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataContextRequest.h"
#include "gui/GuiData/GuiDataIO.h"
#include "controller/controls/ControlProject.h"
#include "controller/controls/ControlApplication.h"
#include "utils/Logger.h"
#include "gui/texts/ContextTexts.hpp"
#include "io/SaveLoadSystem.h"

// Note (Aurélien) QT::StandardButtons enum values in qmessagebox.h
#define Yes 0x00004000
#define No 0x00010000
#define Cancel 0x00400000


// *******************************************
//           Save & Create Project
// *******************************************

ContextSaveCloseCreateProject::ContextSaveCloseCreateProject(const ContextId& id)
//: m_state(ContextState::waiting_for_input)
	: AContext(id)
	, m_folderPath("")
	, m_projectInfo()
	, m_saveLastProject(false)
	, m_closeLastProject(false)
	, m_isWaitingModal(false)
{}

ContextSaveCloseCreateProject::~ContextSaveCloseCreateProject()
{}

ContextState ContextSaveCloseCreateProject::start(Controller& controller)
{
    controller.updateInfo(new GuiDataContextRequestActiveCamera(m_id));
    controller.updateInfo(new GuiDataNewProject(controller.getContext().getProjectsPath(), Utils::System::getFolderFromDirectory(Utils::System::getOSTProgramDataTemplatePath())));

    m_closeLastProject = controller.getContext().isProjectLoaded();

    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextSaveCloseCreateProject::feedMessage(IMessage* message, Controller& controller)
{
    switch (message->getType())
    {
    case IMessage::MessageType::NEW_PROJECT:
    {
        NewProjectMessage* msg = static_cast<NewProjectMessage*>(message);
        m_projectInfo = msg->m_projectInfo;
        m_folderPath = msg->m_folderPath;
        m_templatePath = msg->m_templatePath;
        m_projectTemplate = msg->m_baseProjectTemplate;

		if (controller.getContext().getIsCurrentProjectSaved() == false) {
			controller.updateInfo(new GuiDataModal(Yes | No | Cancel, TEXT_SAVELOADCLOSE_SAVE_QUESTION));
			m_isWaitingModal = true;
		}
		else
			return (m_state = ContextState::ready_for_using);
    }
    break;
    case IMessage::MessageType::MODAL:
    {
		if (!m_isWaitingModal)
			break;

        ModalMessage* modal = static_cast<ModalMessage*>(message);
        switch (modal->m_returnedValue)
        {
        case Yes:
            m_saveLastProject = true;
			return (m_state = ContextState::ready_for_using);
        case No:
            m_saveLastProject = false;
			return (m_state = ContextState::ready_for_using);
        case Cancel:
            return (m_state = ContextState::abort);
        }
    }
    break;
    case IMessage::MessageType::CAMERA:
    {
        CameraMessage* modal = static_cast<CameraMessage*>(message);
        m_cameraNode = modal->m_cameraNode;
    }
    break;
    }
    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextSaveCloseCreateProject::launch(Controller& controller)
{
    if (m_closeLastProject)
    {
        if (m_saveLastProject)
            controller.getControlListener()->notifyUIControl(new control::project::Save(m_cameraNode));
        controller.getControlListener()->notifyUIControl(new control::project::Close());
    }
    m_projectInfo.m_author = controller.getContext().getActiveAuthor();

    if (m_projectTemplate.m_lists.empty() || m_projectTemplate.m_template.empty())
        controller.getControlListener()->notifyUIControl(new control::project::Create(m_projectInfo, m_folderPath, m_templatePath));
    else
        controller.getControlListener()->notifyUIControl(new control::project::Create(m_projectInfo, m_folderPath, m_projectTemplate));

    return (m_state = ContextState::done);
}

bool ContextSaveCloseCreateProject::canAutoRelaunch() const
{
    return false;
}


ContextType ContextSaveCloseCreateProject::getType() const
{
    return ContextType::saveCloseCreateProject;
}

// *******************************************
//           Save & Close Project
// *******************************************

ContextSaveCloseProject::ContextSaveCloseProject(const ContextId& id)
    : AContext(id)
    , m_saveLastProject(false)
{}

ContextSaveCloseProject::~ContextSaveCloseProject()
{}

ContextState ContextSaveCloseProject::start(Controller& controller)
{
    controller.updateInfo(new GuiDataContextRequestActiveCamera(m_id));
    if (controller.getContext().getIsCurrentProjectSaved() == false)
        controller.updateInfo(new GuiDataModal(Yes | No | Cancel, TEXT_SAVELOADCLOSE_SAVE_QUESTION));
    else
        return (m_state = ContextState::ready_for_using);
    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextSaveCloseProject::feedMessage(IMessage* message, Controller& controller)
{
    switch (message->getType())
    {
    case IMessage::MessageType::MODAL:
    {
        ModalMessage* modal = static_cast<ModalMessage*>(message);
        switch (modal->m_returnedValue)
        {
        case Yes:
            m_saveLastProject = true;
            return (m_state = ContextState::ready_for_using);
        case No:
            m_saveLastProject = false;
            return (m_state = ContextState::ready_for_using);
        case Cancel:
            return (m_state = ContextState::abort);
        }
    }
    break;
    case IMessage::MessageType::CAMERA:
    {
        CameraMessage* modal = static_cast<CameraMessage*>(message); 
        m_cameraNode = modal->m_cameraNode;
        return (m_state = ContextState::waiting_for_input);
    }
    break;
    }
    return (m_state);
}

ContextState ContextSaveCloseProject::launch(Controller& controller)
{
    m_state = ContextState::running;
    if (m_saveLastProject)
        controller.getControlListener()->notifyUIControl(new control::project::Save(m_cameraNode));
    controller.getControlListener()->notifyUIControl(new control::project::Close());

    return (m_state = ContextState::done);
}

bool ContextSaveCloseProject::canAutoRelaunch() const
{
    return false;
}

ContextType ContextSaveCloseProject::getType() const
{
    return ContextType::saveCloseProject;
}

// *******************************************
//         Save, Close & Load Project
// *******************************************

ContextSaveCloseLoadProject::ContextSaveCloseLoadProject(const ContextId& id)
    : AContext(id)
    , m_saveLastProject(false)
    , m_saveAndRestoreModals(false)
    , m_restoreProject(false)
    , m_modalsReturn(WaitFor::Save)
{}

ContextSaveCloseLoadProject::~ContextSaveCloseLoadProject()
{}

#include "utils/System.h"

ContextState ContextSaveCloseLoadProject::start(Controller& controller)
{
    controller.updateInfo(new GuiDataContextRequestActiveCamera(m_id));
	return (m_state = ContextState::waiting_for_input);
}

ContextState ContextSaveCloseLoadProject::feedMessage(IMessage* message, Controller& controller)
{
	switch (message->getType())
	{
		case IMessage::MessageType::FILES:
		{
			CONTROLLOG << "Files" << LOGENDL;
			FilesMessage* files = static_cast<FilesMessage*>(message);
			if (files->m_inputFiles.size() != 1)
				return m_state;
			m_projectToLoad = (*files->m_inputFiles.begin());
            if(!m_projectToLoad.empty())
                m_backups = Utils::System::getFilesFromDirectory(m_projectToLoad.parent_path(), File_Extension_Backup, true);

            if (controller.getContext().getIsCurrentProjectSaved() == false)
            {
                m_modalsReturn = WaitFor::Save;
                controller.updateInfo(new GuiDataModal(Yes | No | Cancel, TEXT_SAVELOADCLOSE_SAVE_QUESTION));
            }
            else if (!m_backups.empty())
                prepareRestoreModal(controller);
			else
				return (m_state = ContextState::ready_for_using);
		}
		break;
		case IMessage::MessageType::MODAL:
		{
			CONTROLLOG << "modal" << LOGENDL;
			ModalMessage* modal = static_cast<ModalMessage*>(message);
			switch (m_modalsReturn)
			{
            case WaitFor::Save:
                m_state = processSaveReturn(modal->m_returnedValue);
                if (m_state != ContextState::abort && !m_backups.empty())
                    prepareRestoreModal(controller);
                return m_state;
            case WaitFor::Restore:
                return (m_state = processRestoreReturn(modal->m_returnedValue));
            case WaitFor::Central:
                return (m_state = processOpenCentral(modal->m_returnedValue, controller));
			}
		}
		break;
        case IMessage::MessageType::CAMERA:
        {
			CONTROLLOG << "camera" << LOGENDL;
            CameraMessage* modal = static_cast<CameraMessage*>(message); 
            m_cameraNode = modal->m_cameraNode;
        }
        break;
	}
	return (m_state = ContextState::waiting_for_input);
}

ContextState ContextSaveCloseLoadProject::launch(Controller& controller)
{
    m_state = ContextState::running;
    FUNCLOG << "Context id " << m_id << " SaveCloseLoad Project, launching..." << Logger::endl;
    if (m_saveLastProject == true)
        controller.getControlListener()->notifyUIControl(new control::project::Save(m_cameraNode));
    controller.getControlListener()->notifyUIControl(new control::project::Close());
    
    if (m_projectToLoad.empty() == false)
    {
        if (m_restoreProject)
            SaveLoadSystem::RestoreBackupFiles(m_backups);
        else
            Utils::System::cleanDirectoryFromFiles(m_projectToLoad.parent_path(), File_Extension_Backup, true);

        bool isCentral = false;
        std::filesystem::path pathCentral;
        SaveLoadSystem::readProjectTypes(controller, m_projectToLoad, isCentral, pathCentral);
        if (isCentral)
        {
            m_modalsReturn = WaitFor::Central;
            controller.updateInfo(new GuiDataOpenProjectCentral(m_projectToLoad));
            return (m_state = ContextState::waiting_for_input);
        }
        else
            controller.getControlListener()->notifyUIControl(new control::project::Load(m_projectToLoad));
    }
	return (m_state = ContextState::done);
}

void ContextSaveCloseLoadProject::prepareRestoreModal(Controller& controller)
{
    m_modalsReturn = WaitFor::Restore;
    m_state = ContextState::waiting_for_input;

    std::filesystem::path projectFolder(m_projectToLoad.parent_path());
    QString files;
    for (const std::filesystem::path& file : m_backups)
        files += QString::fromStdWString(L"- " + std::filesystem::proximate(file, projectFolder).wstring()) + "\n";
    controller.updateInfo(new GuiDataModal(Yes | No | Cancel, TEXT_SAVELOADCLOSE_RESTORE_QUESTION.arg(files)));
}

ContextState ContextSaveCloseLoadProject::processOpenCentral(const uint32_t& value, Controller& controller)
{
    switch (value)
    {
    case 1:
        controller.getControlListener()->notifyUIControl(new control::project::Load(m_projectToLoad));
        return (m_state = ContextState::done);
    case 0:
        return (m_state = ContextState::abort);
    }
    return (m_state = ContextState::abort);
}

ContextState ContextSaveCloseLoadProject::processSaveReturn(const uint32_t& value)
{
    switch (value)
    {
        case Yes:
            m_saveLastProject = true;
            return (m_state = ContextState::ready_for_using);
        case No:
            m_saveLastProject = false;
            return (m_state = ContextState::ready_for_using);
        case Cancel:
            return (m_state = ContextState::abort);
    }
    return (m_state = ContextState::abort);
}

ContextState ContextSaveCloseLoadProject::processRestoreReturn(const uint32_t& value)
{
    switch (value)
    {
    case Yes:
        m_restoreProject = true;
        return (m_state = ContextState::ready_for_using);
    case No:
        m_restoreProject = false;
        return (m_state = ContextState::ready_for_using);
    case Cancel:
        return (m_state = ContextState::abort);
    }
    return (m_state = ContextState::abort);
}

bool ContextSaveCloseLoadProject::canAutoRelaunch() const
{
	return false;
}

ContextType ContextSaveCloseLoadProject::getType() const
{
	return ContextType::saveCloseLoadProject;
}

// *******************************************
//          Save & Quit Application
// *******************************************

ContextSaveQuitProject::ContextSaveQuitProject(const ContextId& id)
    : AContext(id)
    , m_saveLastProject(false)
    , m_disableSave(false)
{}

ContextSaveQuitProject::~ContextSaveQuitProject()
{}

ContextState ContextSaveQuitProject::start(Controller& controller)
{
    controller.updateInfo(new GuiDataContextRequestActiveCamera(m_id));
    m_closeLastProject = controller.getContext().isProjectLoaded();

    if (controller.getContext().getIsCurrentProjectSaved() == false)
    {
        controller.updateInfo(new GuiDataModal(Yes | No | Cancel, TEXT_SAVELOADCLOSE_SAVE_QUESTION));
        m_disableSave = false;
    }
    else
        return (m_state = ContextState::ready_for_using);
    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextSaveQuitProject::feedMessage(IMessage* message, Controller& controller)
{
    switch (message->getType())
    {
    case IMessage::MessageType::MODAL:
    {
        ModalMessage* modal = static_cast<ModalMessage*>(message);

        switch (modal->m_returnedValue)
        {
        case Yes:
            m_saveLastProject = !m_disableSave;
            return (m_state = ContextState::ready_for_using);
            break;
        case No:
            m_saveLastProject = false;
            return (m_state = ContextState::ready_for_using);
            break;
        case Cancel:
            return (m_state = ContextState::abort);
        }
    }
    break;
    case IMessage::MessageType::CAMERA:
    {
        CameraMessage* modal = static_cast<CameraMessage*>(message);
        m_cameraNode = modal->m_cameraNode;
        return (m_state = ContextState::waiting_for_input);
    }
    break;
    }
    return (m_state);
}

ContextState ContextSaveQuitProject::launch(Controller& controller)
{
    m_state = ContextState::running;
    if (m_saveLastProject)
        controller.getControlListener()->notifyUIControl(new control::project::Save(m_cameraNode));
    //NOTE(robin) Direct action in the controller ?
    //controller.saveCurrentProject();
    if (m_closeLastProject)
        controller.getControlListener()->notifyUIControl(new control::project::Close());

    controller.getControlListener()->notifyUIControl(new control::application::Quit());

    return (m_state = ContextState::done);
}

bool ContextSaveQuitProject::canAutoRelaunch() const
{
    return false;
}

ContextType ContextSaveQuitProject::getType() const
{
    return ContextType::saveQuitProject;
}

// *******************************************
//          Save Project
// *******************************************

ContextSaveProject::ContextSaveProject(const ContextId& id)
    : AContext(id)
{}

ContextSaveProject::~ContextSaveProject()
{}

ContextState ContextSaveProject::start(Controller& controller)
{
    controller.updateInfo(new GuiDataContextRequestActiveCamera(m_id));
    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextSaveProject::feedMessage(IMessage* message, Controller& controller)
{
    switch (message->getType())
    {
    case IMessage::MessageType::CAMERA:
    {
        CameraMessage* modal = static_cast<CameraMessage*>(message);
        m_cameraNode = modal->m_cameraNode;
        return (m_state = ContextState::ready_for_using);
    }
    break;
    }
    return (m_state);
}

ContextState ContextSaveProject::launch(Controller& controller)
{
    m_state = ContextState::running;
    controller.getControlListener()->notifyUIControl(new control::project::Save(m_cameraNode));
    return (m_state = ContextState::done);
}

bool ContextSaveProject::canAutoRelaunch() const
{
    return false;
}

ContextType ContextSaveProject::getType() const
{
    return ContextType::saveProject;
}

// *******************************************
//          Autosave Project
// *******************************************

ContextAutosaveProject::ContextAutosaveProject(const ContextId& id)
    : AContext(id)
{}

ContextAutosaveProject::~ContextAutosaveProject()
{}

ContextState ContextAutosaveProject::start(Controller& controller)
{
    controller.updateInfo(new GuiDataActivatedFunctions(ContextType::autosaveProject));
    controller.updateInfo(new GuiDataContextRequestActiveCamera(m_id));
    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextAutosaveProject::feedMessage(IMessage* message, Controller& controller)
{
    switch (message->getType())
    {
    case IMessage::MessageType::CAMERA:
    {
        CameraMessage* modal = static_cast<CameraMessage*>(message);
        m_cameraNode = modal->m_cameraNode;
        return (m_state = ContextState::ready_for_using);
    }
    break;
    }
    return (m_state);
}

ContextState ContextAutosaveProject::launch(Controller& controller)
{
    m_state = ContextState::running;
    controller.getControlListener()->notifyUIControl(new control::project::Save(m_cameraNode));
    return (m_state = ContextState::done);
}

bool ContextAutosaveProject::canAutoRelaunch() const
{
    return false;
}

ContextType ContextAutosaveProject::getType() const
{
    return ContextType::autosaveProject;
}
