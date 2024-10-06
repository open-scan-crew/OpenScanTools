#include "controller/controls/ControlProjectTemplate.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "io/SaveLoadSystem.h"
#include "gui/GuiData/GuiDataList.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataTemplate.h"
#include "gui/texts/ProjectTemplateTexts.hpp"
#include "io/FileUtils.h"
#include "Gui/Translator.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <Windows.h>

#include "models/graph/GraphManager.hxx"

// control::projectTemplate::
#define Yes 0x00004000
#define No 0x00010000
#define Cancel 0x00400000
#define MAX_RECENT_PROJECTS_SIZE 10

namespace control
{
	namespace projectTemplate
	{
		/*
		** CreateTemplate
		*/

		CreateTemplate::CreateTemplate(const std::wstring& templateName
			, bool updateTemplate
			/*, const std::filesystem::path& templateOriginPath*/)
			: m_newTemplateName(templateName)
			, m_updateTemplate(updateTemplate)
			//, m_templateOriginPath(templateOriginPath)
		{
		}

		CreateTemplate::~CreateTemplate()
		{
		}

		void CreateTemplate::doFunction(Controller& controller)
		{
			CONTROLLOG << "control::projectTemplate::createTemplate[start] " << LOGENDL;

			std::filesystem::path newTemplatePath(Utils::System::getOSTProgramDataTemplatePath() / m_newTemplateName);
			if (std::filesystem::exists(newTemplatePath))
			{
				if (!m_updateTemplate)
				{
					controller.updateInfo(new GuiDataWarning(QString(TEXT_PROJECT_TEMPLATE_ERROR_EXIST).arg(QString::fromStdWString(m_newTemplateName))));
					return;
				}
			}
			else
			{
				if (m_updateTemplate)
				{
					controller.updateInfo(new GuiDataWarning(QString(TEXT_PROJECT_TEMPLATE_ERROR_NOT_EXIST).arg(QString::fromStdWString(m_newTemplateName))));
					return;
				}
			}

			if (!Utils::System::createDirectoryIfNotExist(newTemplatePath))
			{
				controller.updateInfo(new GuiDataWarning(QString(TEXT_PROJECT_TEMPLATE_ERROR_FAILED_TO_CREATE).arg(QString::fromStdWString(m_newTemplateName))));
				return;
			}

			std::error_code error;
			for (const std::filesystem::path& p : std::filesystem::directory_iterator(controller.getContext().cgetProjectInternalInfo().getTemplatesFolderPath()))
				if (!std::filesystem::is_directory(p))
					std::filesystem::copy_file(p, newTemplatePath / p.filename(), std::filesystem::copy_options::overwrite_existing, error);

			SaveLoadSystem::exportArboFile(newTemplatePath, controller);
			
			if (error)
			{
				IOLOG << "Error template create : " << error.message() << LOGENDL;
				controller.updateInfo(new GuiDataWarning(QString(TEXT_PROJECT_TEMPLATE_ERROR_FAILED_TO_CREATE).arg(QString::fromStdWString(m_newTemplateName))));
				return;
			}
			else
				controller.updateInfo(new GuiDataInfo(m_updateTemplate ? TEXT_PROJECT_TEMPLATE_UPDATE_SUCCES : TEXT_PROJECT_TEMPLATE_CREATE_SUCCES, true));

			controller.getControlListener()->notifyUIControl(new SendList());
			CONTROLLOG << "control::projectTemplate::createTemplate[end] " << newTemplatePath << LOGENDL;
		}

		bool CreateTemplate::canUndo() const
		{
			return (false);
		}

		void CreateTemplate::undoFunction(Controller& controller)
		{
		}

		ControlType CreateTemplate::getType() const
		{
			return (ControlType::createProjectTemplate);
		}

		/*
		** SaveTemplate
		*/

		SaveTemplate::SaveTemplate(const std::wstring& templateName)
			: m_templateName(templateName)
		{}

		SaveTemplate::~SaveTemplate()
		{}

		void SaveTemplate::doFunction(Controller& controller)
		{
			CONTROLLOG << "control::projectTemplate::SaveTemplate[begin]" << LOGENDL;


			std::filesystem::path templatePath(Utils::System::getOSTProgramDataTemplatePath() / m_templateName);
			if(!std::filesystem::exists(templatePath))
			{
				controller.updateInfo(new GuiDataWarning(QString(TEXT_PROJECT_TEMPLATE_ERROR_NOT_EXIST).arg(QString::fromStdWString(m_templateName))));
				CONTROLLOG << "control::projectTemplate::SaveTemplate : Error template does not exist." << LOGENDL;
				return;
			}
			SaveLoadSystem::ErrorCode code;
			SaveLoadSystem::ExportTemplates(controller.getContext().getTemplates(), code,templatePath / File_Templates);
			SaveLoadSystem::ExportLists<UserList>(controller.getContext().getUserLists(),templatePath / File_Lists);
			SaveLoadSystem::ExportLists<StandardList>(controller.getContext().getStandards(StandardType::Pipe), templatePath / File_Pipes);
			//SaveLoadSystem::ExportLists<StandardList>(controller.getContext().getStandards(StandardType::Sphere), templatePath / File_Spheres);
			//SaveLoadSystem::ExportLists<StandardList>(controller.getContext().getStandards(StandardType::Torus), templatePath / File_Torus);

			CONTROLLOG << "control::projectTemplate::SaveTemplate[end] "<< templatePath << LOGENDL;
		}

		bool SaveTemplate::canUndo() const
		{
			return (false);
		}

		void SaveTemplate::undoFunction(Controller& controller)
		{}

		ControlType SaveTemplate::getType() const
		{
			return (ControlType::saveProjectTemplate);
		}

		/*
		** RenameTemplate
		*/

		RenameTemplate::RenameTemplate(const std::wstring& oldTemplateName, const std::wstring& newTemplateName)
			: m_oldTemplateName(oldTemplateName)
			, m_newTemplateName(newTemplateName)
		{}

		RenameTemplate::~RenameTemplate()
		{}

		void RenameTemplate::doFunction(Controller& controller)
		{
			CONTROLLOG << "control::projectTemplate::RenameTemplate[begin]" << LOGENDL;

			std::filesystem::path newTemplatePath(Utils::System::getOSTProgramDataTemplatePath() / m_newTemplateName);
			std::filesystem::path oldTemplatePath(Utils::System::getOSTProgramDataTemplatePath() / m_oldTemplateName);

			if (std::filesystem::exists(newTemplatePath))
			{
				controller.updateInfo(new GuiDataWarning(QString(TEXT_PROJECT_TEMPLATE_ERROR_NOT_EXIST).arg(QString::fromStdWString(m_newTemplateName))));
				CONTROLLOG << "control::projectTemplate::RenameTemplate : new name already exist." << LOGENDL;
				return;
			}

			if (!std::filesystem::exists(oldTemplatePath))
			{
				controller.updateInfo(new GuiDataWarning(QString(TEXT_PROJECT_TEMPLATE_ERROR_NOT_EXIST).arg(QString::fromStdWString(m_oldTemplateName))));
				CONTROLLOG << "control::projectTemplate::RenameTemplate : template does not exist." << LOGENDL;
				return;
			}

			std::filesystem::rename(oldTemplatePath, newTemplatePath);
			controller.getControlListener()->notifyUIControl(new SendList());

			CONTROLLOG << "control::projectTemplate::RenameTemplate[end] " << newTemplatePath << LOGENDL;
		}

		bool RenameTemplate::canUndo() const
		{
			return (false);
		}
		void RenameTemplate::undoFunction(Controller& controller)
		{}

		ControlType RenameTemplate::getType() const
		{
			return (ControlType::renameProjectTemplate);
		}
	
		/*
		** DeleteTemplate
		*/

		DeleteTemplate::DeleteTemplate(const std::wstring& templateName)
			: m_templateName(templateName)
		{}

		DeleteTemplate::~DeleteTemplate()
		{}

		void DeleteTemplate::doFunction(Controller& controller)
		{
			CONTROLLOG << "control::projectTemplate::DeleteTemplate[begin]" << LOGENDL;

			std::filesystem::path templatePath(Utils::System::getOSTProgramDataTemplatePath() / m_templateName);

			if (!std::filesystem::exists(templatePath))
			{
				controller.updateInfo(new GuiDataWarning(QString(TEXT_PROJECT_TEMPLATE_ERROR_NOT_EXIST).arg(QString::fromStdWString(m_templateName))));
				CONTROLLOG << "control::projectTemplate::DeleteTemplate : template does not exist." << LOGENDL;
				return;
			}

			Utils::System::cleanDirectory(templatePath);

			controller.getControlListener()->notifyUIControl(new SendList());

			CONTROLLOG << "control::projectTemplate::DeleteTemplate[end] " << templatePath << LOGENDL;
		}

		bool DeleteTemplate::canUndo() const
		{
			return (false);
		}

		void DeleteTemplate::undoFunction(Controller& controller)
		{}

		ControlType DeleteTemplate::getType() const
		{
			return (ControlType::deleteProjectTemplate);
		}

		/*
		** SendList
		*/

		SendList::SendList()
		{}

		SendList::~SendList()
		{}

		void SendList::doFunction(Controller& controller)
		{
			CONTROLLOG << "control::projectTemplate::SendList[begin]" << LOGENDL;

			std::vector<std::filesystem::path> templates(Utils::System::getFolderFromDirectory(Utils::System::getOSTProgramDataTemplatePath()));
			controller.updateInfo(new GuiDataProjectTemplateList(templates));
			CONTROLLOG << "control::projectTemplate::SendList[end] found  " << templates.size() << " templates." << LOGENDL;
		}

		bool SendList::canUndo() const
		{
			return (false);
		}

		void SendList::undoFunction(Controller& controller)
		{}

		ControlType SendList::getType() const
		{
			return (ControlType::sendListProjectTemplate);
		}

		/*
		** CloseTemplateEdition
		*/

		CloseTemplateEdition::CloseTemplateEdition()
		{}

		CloseTemplateEdition::~CloseTemplateEdition()
		{}

		void CloseTemplateEdition::doFunction(Controller& controller)
		{
		}

		bool CloseTemplateEdition::canUndo() const
		{
			return (false);
		}

		void CloseTemplateEdition::undoFunction(Controller& controller)
		{}

		ControlType CloseTemplateEdition::getType() const
		{
			return (ControlType::stopEditionProjectTemplate);
		}
	
	}
}