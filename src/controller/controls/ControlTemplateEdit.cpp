#include "controller/controls/ControlTemplateEdit.h"
#include "controller/Controller.h"
#include "controller/IControlListener.h"
#include "controller/ControllerContext.h"
#include "controller/functionSystem/FunctionManager.h"

#include "controller/messages/DataIdListMessage.h"
#include "controller/messages/TemplateMessage.h"
#include "controller/messages/TemplateListMessage.h"

#include "io/SaveLoadSystem.h"

#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataTemplate.h"
#include "gui/Texts.hpp"

#include "models/graph/TagNode.h"
#include "models/graph/GraphManager.h"

#include "utils/FilesAndFoldersDefinitions.h"

#include "magic_enum/magic_enum.hpp"

namespace control
{
	namespace tagTemplate
	{
		SendTemplateList::SendTemplateList()
		{
		}

		SendTemplateList::~SendTemplateList()
		{

		}

		void SendTemplateList::doFunction(Controller& controller)
		{
			controller.updateInfo(new GuiDataSendTemplateList(controller.getContext().getTemplates()));
			CONTROLLOG << "control::tagTemplates::SendTemplateList do " << controller.getContext().getTemplates().size() << " elems" << LOGENDL;
		}

		bool SendTemplateList::canUndo() const
		{
			return (false);
		}

		void SendTemplateList::undoFunction(Controller& controller)
		{ }

		ControlType SendTemplateList::getType() const
		{
			return (ControlType::sendTemplateList);
		}

		//controll::tagTemplate::SendTagTemplate

		SendTagTemplate::SendTagTemplate(SafePtr<sma::TagTemplate> temp)
		{
			m_temp = temp;
		}

		SendTagTemplate::~SendTagTemplate()
		{

		}

		void SendTagTemplate::doFunction(Controller& controller)
		{
			if (controller.getContext().getTemplates().find(m_temp) != controller.getContext().getTemplates().end())
				controller.updateInfo(new GuiDataSendTagTemplate(m_temp, controller.getContext().getUserLists()));
			else
				controller.updateInfo(new GuiDataWarning(TEXT_EDIT_TEMPLATE_NOT_EXIST));
			CONTROLLOG << "control::tagTemplate::SendtagTemplate do " << LOGENDL;
		}

		ControlType SendTagTemplate::getType() const
		{
			return (ControlType::sendTagTemplate);
		}

		//controll::tagTemplate::CreateTagTemplate

		CreateTagTemplate::CreateTagTemplate(std::wstring name)
		{
			_name = name;
		}

		CreateTagTemplate::~CreateTagTemplate()
		{

		}

		void CreateTagTemplate::doFunction(Controller& controller)
		{
			SafePtr<sma::TagTemplate> currentTemplate;
			for (SafePtr<sma::TagTemplate> temp : controller.getContext().getTemplates())
			{
				ReadPtr<sma::TagTemplate> rTemp = temp.cget();
				if (!rTemp)
				{
					controller.getContext().getTemplates().erase(temp);
					continue;
				}

				if (rTemp->getName() == _name)
				{
					controller.updateInfo(new GuiDataWarning(TEXT_TAG_NAME_TEMPLATE_ALREADY_TAKEN));
					controller.getControlListener()->notifyUIControl(new SendTemplateList());
					return;
				}
			}

			currentTemplate = make_safe<sma::TagTemplate>(_name);

			controller.getContext().getTemplates().insert(currentTemplate);
			if (controller.getContext().getTemplates().size() == 1)
				controller.getContext().setCurrentTemplate(currentTemplate);

			controller.getControlListener()->notifyUIControl(new SendTemplateList());
			CONTROLLOG << "control::tagTemplates::CreateTagTemplate do" << LOGENDL;
		}

		ControlType CreateTagTemplate::getType() const
		{
			return (ControlType::createTagTemplate);
		}

		//controll::tagTemplate::SaveTemplates

		SaveTemplates::SaveTemplates()
		{
		}

		SaveTemplates::~SaveTemplates()
		{ }

		void SaveTemplates::doFunction(Controller& controller)
		{
			std::unordered_set<SafePtr<sma::TagTemplate>> templates = controller.getContext().getTemplates();

			SaveLoadSystem::ErrorCode error;
			std::vector<sma::TagTemplate> templatesData;
			for (SafePtr<sma::TagTemplate> temp : templates)
			{
				ReadPtr<sma::TagTemplate> rTemp = temp.cget();
				if (rTemp)
					templatesData.push_back(*&rTemp);
			}
			SaveLoadSystem::ExportTemplates(templatesData, error, controller.getContext().cgetProjectInternalInfo().getTemplatesFolderPath() / File_Templates);
			controller.updateInfo(new GuiDataObjectSelected());
			CONTROLLOG << "control::tagTemplates::SaveTemplates do" << LOGENDL;
		}

		bool SaveTemplates::canUndo() const
		{
			return (false);
		}

		void SaveTemplates::undoFunction(Controller& controller)
		{ }

		ControlType SaveTemplates::getType() const
		{
			return (ControlType::saveTemplate);
		}

		//controll::tagTemplate::DeleteTagTemplate

		DeleteTagTemplate::DeleteTagTemplate(SafePtr<sma::TagTemplate> tagTemp)
		{
			m_toDeleteTemplate = tagTemp;
		}

		DeleteTagTemplate::~DeleteTagTemplate()
		{
		}

		void DeleteTagTemplate::doFunction(Controller& controller)
		{
			GraphManager& graphManager = controller.getGraphManager();

			std::unordered_set<SafePtr<TagNode>> tags = graphManager.getTagsWithTemplate(m_toDeleteTemplate);
			if (tags.empty())
			{
				controller.getContext().getTemplates().erase(m_toDeleteTemplate);
				m_toDeleteTemplate.destroy();
				controller.getControlListener()->notifyUIControl(new SendTemplateList());
			}
			else
			{
				controller.getFunctionManager().launchFunction(controller, ContextType::tagDeletion);
				std::unordered_set<SafePtr<AGraphNode>> nodes;
				for (const SafePtr<TagNode>& tag : tags)
					nodes.insert(tag);
				DataListMessage message(nodes, ElementType::Tag);
				controller.getFunctionManager().feedMessage(controller, &message);
			}
			controller.resetHistoric();
			CONTROLLOG << "control::tagTemplates::DeleteTagTemplate do" << LOGENDL;
		}

		ControlType DeleteTagTemplate::getType() const
		{
			return (ControlType::deleteTagTemplate);
		}

		//controll::tagTemplate::DuplicateTagTemplate

		DuplicateTagTemplate::DuplicateTagTemplate(SafePtr<sma::TagTemplate> tagTemp)
		{
			m_toDuplicateTemplate = tagTemp;
		}

		DuplicateTagTemplate::~DuplicateTagTemplate()
		{ }

		void DuplicateTagTemplate::doFunction(Controller& controller)
		{

			/*sma::TagTemplate originTemp = controller.getContext().getTemplates().at(_originId);
			std::wstring newName = originTemp.getName();
			int nb = 1;

			while (nb < 1000)
			{
				std::wstringstream ss;
				ss << newName << "_" << nb;
				if (controller.getContext().verifNameForTemplate(ss.str()) == true)
				{
					newName = ss.str();
					break;
				}
				++nb;
			}
			if (nb == 1000)
				return;
			sma::TagTemplate newTemp = sma::TagTemplate(newName);
			std::vector<sma::tField> fields = originTemp.getFieldsCopy();

			for (auto it = fields.begin(); it != fields.end(); it++)
				newTemp.addNewField(*it);

			controller.getContext().getTemplates().insert(std::pair<sma::templateId, sma::TagTemplate>(newTemp.getId(), newTemp));
			_newTemp = newTemp;*/

			sma::TagTemplate newTempData;
			{
				ReadPtr<sma::TagTemplate> rTemp = m_toDuplicateTemplate.cget();
				if (!rTemp)
					return;

				newTempData = *&rTemp;
			}

			newTempData.setId(xg::newGuid());

			controller.getContext().setTemplates({ newTempData });
			controller.getControlListener()->notifyUIControl(new SendTemplateList());
			CONTROLLOG << "control::tagTemplates::DuplicateTagTemplates do" << LOGENDL;
		}
		/*
		bool DuplicateTagTemplate::canUndo() const
		{
			return (false);
		}

		void DuplicateTagTemplate::undoFunction(Controller& controller)
		{
			if (controller.getContext().getTemplates().find(_newTemp.getId()) != controller.getContext().getTemplates().end())
			{
				controller.getContext().getTemplates().erase(_newTemp.getId());
				controller.getControlListener()->notifyUIControl(new SendTemplateList());
			}
			else
				controller.updateInfo(new GuiDataWarning(TEXT_TEMPLATE_DUPLICATE_UNDO_FAILED));
			CONTROLLOG << "controll::tagTemplates::DuplicateTagTemplate undo" << LOGENDL;
		}
		*/

		ControlType DuplicateTagTemplate::getType() const
		{
			return (ControlType::duplicateTagTemplate);
		}

		//controll::tagTemplate::RenameTagTemplate

		RenameTagTemplate::RenameTagTemplate(SafePtr<sma::TagTemplate> tagTemp, std::wstring newName)
		{
			m_newName = newName;
			m_toChangeTemplate = tagTemp;
		}

		RenameTagTemplate::~RenameTagTemplate()
		{ }

		void RenameTagTemplate::doFunction(Controller& controller)
		{
			WritePtr<sma::TagTemplate> wTemp = m_toChangeTemplate.get();
			if (!wTemp)
			{
				controller.updateInfo(new GuiDataWarning(TEXT_TEMPLATE_RENAME_FAILED));
				return;
			}

			wTemp->renameTagTemplate(m_newName);

			controller.getControlListener()->notifyUIControl(new SendTemplateList());
			controller.getControlListener()->notifyUIControl(new SendTagTemplate(m_toChangeTemplate));

			controller.updateInfo(new GuiDataObjectSelected());
			CONTROLLOG << "control::tagTemplates::RenameTagTemplate do" << LOGENDL;
		}
		/*
		bool RenameTagTemplate::canUndo() const
		{
			return (false);
		}

		void RenameTagTemplate::undoFunction(Controller& controller)
		{
			if (controller.getContext().getTemplates().find(_id) != controller.getContext().getTemplates().end())
			{
				controller.getContext().getTemplates().at(_id).renameTagTemplate(_oldName);
				controller.getControlListener()->notifyUIControl(new SendTemplateList());
				controller.getControlListener()->notifyUIControl(new SendTagTemplate(_id));
			}
			else
				controller.updateInfo(new GuiDataWarning(TEXT_TEMPLATE_RENAME_FAILED));
			UpdatePropertyPanel(controller);
			CONTROLLOG << "controll::tagTemplates::RenameTagTemplate undo" << LOGENDL;
		}
		*/

		ControlType RenameTagTemplate::getType() const
		{
			return (ControlType::renameTagTemplate);
		}

		//controll::tagTemplate::TemplateCreateField

		TemplateCreateField::TemplateCreateField(SafePtr<sma::TagTemplate> tagTemp, std::wstring name)
		{
			m_toChangeTemplate = tagTemp;
			m_name = name;
		}

		TemplateCreateField::~TemplateCreateField()
		{ }

		void TemplateCreateField::doFunction(Controller& controller)
		{
			if (m_name.empty())
				return;

			{
				WritePtr<sma::TagTemplate> wTagTemp = m_toChangeTemplate.get();
				if (!wTagTemp)
					return;
				wTagTemp->addNewField(m_name, sma::tFieldType::string);
			}

			controller.updateInfo(new GuiDataObjectSelected());

			controller.updateInfo(new GuiDataSendTagTemplate(m_toChangeTemplate, controller.getContext().getUserLists()));

			CONTROLLOG << "control::tagTemplates::TemplateCreateField do" << LOGENDL;
		}

		bool TemplateCreateField::canUndo() const
		{
			return (false);
		}

		void TemplateCreateField::undoFunction(Controller& controller)
		{ }

		ControlType TemplateCreateField::getType() const
		{
			return (ControlType::createFieldTemplate);
		}

		//controll::tagTemplate::TemplateDeleteField

		TemplateDeleteField::TemplateDeleteField(SafePtr<sma::TagTemplate> tagTemp, sma::tFieldId fieldId)
		{
			m_toChangeTemplate = tagTemp;
			m_fieldId = fieldId;
		}

		TemplateDeleteField::~TemplateDeleteField()
		{ }

		void TemplateDeleteField::doFunction(Controller& controller)
		{
			GraphManager& graphManager = controller.getGraphManager();

			{
				WritePtr<sma::TagTemplate> wTagTemp = m_toChangeTemplate.get();
				if (!wTagTemp || wTagTemp->getFields().find(m_fieldId) == wTagTemp->getFields().end())
					return;
				wTagTemp->removeField(m_fieldId);
			}

			controller.updateInfo(new GuiDataSendTagTemplate(m_toChangeTemplate, controller.getContext().getUserLists()));

			std::unordered_set<SafePtr<TagNode>> tags(graphManager.getTagsWithTemplate(m_toChangeTemplate));
			for (const SafePtr<TagNode>& tag : tags)
			{
				WritePtr<TagNode> writeTag = tag.get();
				if (!writeTag)
					continue;
				writeTag->removeField(m_fieldId);
				doTimeModified(*&writeTag);
			}
			controller.updateInfo(new GuiDataObjectSelected());

			CONTROLLOG << "control::tagTemplates::TemplateDeleteField do" << LOGENDL;
		}

		bool TemplateDeleteField::canUndo() const
		{
			return (false);
		}

		void TemplateDeleteField::undoFunction(Controller& controller)
		{ }

		ControlType TemplateDeleteField::getType() const
		{
			return (ControlType::deleteFieldTemplate);
		}

		//controll::tagTemplate::TemplateRenameField

		TemplateRenameField::TemplateRenameField(SafePtr<sma::TagTemplate> tagTemp, sma::tFieldId fieldId, std::wstring newName)
		{
			m_toChangeTemplate = tagTemp;
			m_fieldId = fieldId;
			m_newName = newName;
		}

		TemplateRenameField::~TemplateRenameField()
		{ }

		void TemplateRenameField::doFunction(Controller& controller)
		{
			{
				WritePtr<sma::TagTemplate> wTagTemp = m_toChangeTemplate.get();
				if (!wTagTemp || wTagTemp->getFields().find(m_fieldId) == wTagTemp->getFields().end())
					return; 
				
				for (auto it = wTagTemp->getFields().begin(); it != wTagTemp->getFields().end(); it++)
				{
					if (it->second.m_name == m_newName)
					{
						CONTROLLOG << "control::tagTemplates::TemplateRenameField refuse for same name" << LOGENDL;
						return;
					}
				}
				wTagTemp->modifyFieldName(m_fieldId, m_newName);
				
			}

			controller.updateInfo(new GuiDataSendTagTemplate(m_toChangeTemplate, controller.getContext().getUserLists()));

			CONTROLLOG << "control::tagTemplates::TemplateRenameField do" << LOGENDL;
		}

		bool TemplateRenameField::canUndo() const
		{
			return (false);
		}

		void TemplateRenameField::undoFunction(Controller& controller)
		{ }

		ControlType TemplateRenameField::getType() const
		{
			return (ControlType::renameFieldTemplate);
		}

		//controll::tagTemplate::TemplateChangeTypeField

		TemplateChangeTypeField::TemplateChangeTypeField(SafePtr<sma::TagTemplate> tagTemp, sma::tFieldId fieldId, sma::tFieldType newType)
		{
			m_toChangeTemplate = tagTemp;
			m_fieldId = fieldId;
			m_newType = newType;
		}

		TemplateChangeTypeField::~TemplateChangeTypeField()
		{ }

		void TemplateChangeTypeField::doFunction(Controller& controller)
		{
			GraphManager& graphManager = controller.getGraphManager();

			std::unordered_set<SafePtr<TagNode>> tags = graphManager.getTagsWithTemplate(m_toChangeTemplate);
			std::unordered_set<SafePtr<AGraphNode>> nodes;

			for (const SafePtr<TagNode>& tag : tags)
				nodes.insert(tag);

			TemplateMessage templateMessage(m_toChangeTemplate, m_fieldId, m_newType);
			DataListMessage dataMessage(nodes, ElementType::Tag);
			controller.getFunctionManager().launchFunction(controller, ContextType::templateModification);
			controller.getFunctionManager().feedMessage(controller, &templateMessage);
			controller.getFunctionManager().feedMessage(controller, &dataMessage);

			CONTROLLOG << "control::tagTemplates::TemplateChangeTypeField do " << magic_enum::enum_name<sma::tFieldType>(m_newType) << LOGENDL;
		}

		bool TemplateChangeTypeField::canUndo() const
		{
			return (false);
		}

		void TemplateChangeTypeField::undoFunction(Controller& controller)
		{ }

		ControlType TemplateChangeTypeField::getType() const
		{
			return (ControlType::changeTypeFieldTemplate);
		}

		//controll::tagTemplate::TemplateChangeRefField

		TemplateChangeRefField::TemplateChangeRefField(SafePtr<sma::TagTemplate> tagTemp, sma::tFieldId fieldId, SafePtr<UserList> newRef)
		{
			m_toChangeTemplate = tagTemp;
			m_fieldId = fieldId;
			m_newRef = newRef;
		}

		TemplateChangeRefField::~TemplateChangeRefField()
		{ }

		void TemplateChangeRefField::doFunction(Controller& controller)
		{
			GraphManager& graphManager = controller.getGraphManager();

			std::unordered_set<SafePtr<TagNode>> tags = graphManager.getTagsWithTemplate(m_toChangeTemplate);
			std::unordered_set<SafePtr<AGraphNode>> nodes;

			for (const SafePtr<TagNode>& tag : tags)
				nodes.insert(tag);

			TemplateListMessage templateMessage(m_toChangeTemplate, m_fieldId, m_newRef);
			DataListMessage dataMessage(nodes, ElementType::Tag);
			controller.getFunctionManager().launchFunction(controller, ContextType::templateListModification);
			controller.getFunctionManager().feedMessage(controller, &templateMessage);
			controller.getFunctionManager().feedMessage(controller, &dataMessage);

			//if (controller.getContext().getTemplates().find(_tempId) != controller.getContext().getTemplates().end()
			//	&& controller.getContext().getTemplates().at(_tempId).getFields().find(_fieldId)
			//	!= controller.getContext().getTemplates().at(_tempId).getFields().end())
			//{
			//	controller.getContext().getTemplates().at(_tempId).modifyFieldReference(_fieldId, _newRef);
			//	controller.updateInfo(new GuiDataSendTagTemplate(controller.getContext().getTemplates().at(_tempId), controller.getContext//().getUserLists()));
			//}
			//UpdatePropertyPanel(controller);

			CONTROLLOG << "control::tagTemplates::TemplateChangeRefField do" << LOGENDL;
		}

		bool TemplateChangeRefField::canUndo() const
		{
			return (false);
		}

		void TemplateChangeRefField::undoFunction(Controller& controller)
		{ }

		ControlType TemplateChangeRefField::getType() const
		{
			return (ControlType::changeRefFieldTemplate);
		}

		//controll::tagTemplate::TemplateChangeDefaultValue

		TemplateChangeDefaultValue::TemplateChangeDefaultValue(SafePtr<sma::TagTemplate> tagTemp, sma::tFieldId fieldId, std::wstring newValue)
		{
			m_toChangeTemplate = tagTemp;
			m_fieldId = fieldId;
			m_defaultValue = newValue;
		}

		TemplateChangeDefaultValue::~TemplateChangeDefaultValue()
		{ }

		void TemplateChangeDefaultValue::doFunction(Controller& controller)
		{

			WritePtr<sma::TagTemplate> wTagTemp = m_toChangeTemplate.get();
			if (!wTagTemp || wTagTemp->getFields().find(m_fieldId) == wTagTemp->getFields().end())
				return;
			wTagTemp->modifyFieldDefaultValue(m_fieldId, m_defaultValue);

			controller.updateInfo(new GuiDataSendTagTemplate(m_toChangeTemplate, controller.getContext().getUserLists()));

			controller.updateInfo(new GuiDataObjectSelected());
			CONTROLLOG << "control::tagTemplates::TemplateChangeDefaultValue do" << LOGENDL;
		}

		bool TemplateChangeDefaultValue::canUndo() const
		{
			return (false);
		}

		void TemplateChangeDefaultValue::undoFunction(Controller& controller)
		{ }

		ControlType TemplateChangeDefaultValue::getType() const
		{
			return (ControlType::changeDefaultValueFieldTemplate);
		}

	}
}