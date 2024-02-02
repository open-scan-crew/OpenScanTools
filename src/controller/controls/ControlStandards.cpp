#include "controller/controls/ControlStandards.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "gui/GuiData/GuiDataList.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "io/SaveLoadSystem.h"
#include "gui/Texts.hpp"

#define PIPE_FILE "Pipes_standards.tll"
#define SPHERE_FILE "Spheres_standards.tll"

namespace control
{
	namespace standards
	{
		//controll::standard::SelectStandard
	
		SelectStandard::SelectStandard(const SafePtr<StandardList>& standard, const StandardType& type)
			: m_standard(standard)
			, m_type(type)
		{}

		SelectStandard::~SelectStandard()
		{}

		void SelectStandard::doFunction(Controller& controller)
		{
			controller.getContext().setCurrentStandard(m_standard, m_type);
			CONTROLLOG << "control::standards::SetCurrentStandard " << LOGENDL;
		}

		bool SelectStandard::canUndo() const
		{
			return (false);
		}

		void SelectStandard::undoFunction(Controller& controller)
		{}

		ControlType SelectStandard::getType() const
		{
			return (ControlType::selectStandard);
		}

		//controll::standard::SendStandards

		SendStandards::SendStandards(const StandardType& type)
			: m_type(type)
		{}

		SendStandards::~SendStandards()
		{}

		void SendStandards::doFunction(Controller& controller)
		{
			controller.updateInfo(new GuiDataSendListsStandards(m_type, controller.getContext().getStandards(m_type)));
		}

		bool SendStandards::canUndo() const
		{
			return (false);
		}

		void SendStandards::undoFunction(Controller& controller)
		{ }

		ControlType SendStandards::getType() const
		{
			return (ControlType::sendStandards);
		}

		//controll::standard::CreateStandard

		CreateStandard::CreateStandard(const std::wstring& name, const StandardType& type)
			: m_name(name)
			, m_type(type)
		{}

		CreateStandard::~CreateStandard()
		{}

		void CreateStandard::doFunction(Controller& controller)
		{
			StandardList list;
			
			for (SafePtr<StandardList> standard : controller.getContext().getStandards(m_type))
			{
				ReadPtr<StandardList> rStandard = standard.cget();
				if (rStandard && rStandard->getName() == m_name)
				{
					controller.updateInfo(new GuiDataWarning(TEXT_STANDARDS_NAME_EXIST));
					controller.updateInfo(new GuiDataSendListsStandards(m_type, controller.getContext().getStandards(m_type)));
					return;
				}
			}
			list = StandardList(m_name);

			controller.getContext().setStandards({ list }, m_type);
			controller.updateInfo(new GuiDataSendListsStandards(m_type, controller.getContext().getStandards(m_type)));
			CONTROLLOG << "controll::standard::CreateStandard do" << LOGENDL;
		}

		ControlType CreateStandard::getType() const
		{
			return (ControlType::createStandard);
		}

		//controll::standard::DeleteStandard

		DeleteStandard::DeleteStandard(const SafePtr<StandardList>& standard, const StandardType& type)
			: m_standard(standard)
			, m_type(type)
		{}

		DeleteStandard::~DeleteStandard()
		{}

		void DeleteStandard::doFunction(Controller& controller)
		{
			controller.getContext().getStandards(m_type).erase(m_standard);
			m_standard.destroy();
			controller.updateInfo(new GuiDataSendListsStandards(m_type, controller.getContext().getStandards(m_type)));
			
			CONTROLLOG << "controll::standard::DeleteStandard do" << LOGENDL;
		}

		ControlType DeleteStandard::getType() const
		{
			return (ControlType::deleteStandard);
		}

		//controll::standard::DuplicateStandard

		DuplicateStandard::DuplicateStandard(const SafePtr<StandardList>& standard, const StandardType& type)
			: m_standard(standard)
			, m_type(type)
		{}

		DuplicateStandard::~DuplicateStandard()
		{}

		void DuplicateStandard::doFunction(Controller& controller)
		{
			StandardList newStandard;

			{
				ReadPtr<StandardList> rStandard = m_standard.cget();
				if (!rStandard)
					return;
				newStandard = *&rStandard;
			}

			std::wstring name;
			int nb = 1;
			while (nb < 1000)
			{
				std::wstringstream ss;
				ss << newStandard.getName() << "_" << nb;
				if (controller.getContext().verifNameForStandards(m_type, ss.str()) == true)
				{
					name = ss.str();
					break;
				}
				++nb;
			}
			if (nb == 1000)
				return;

			newStandard.setName(name);
			newStandard.setId(xg::newGuid());

			controller.getContext().setStandards({ newStandard }, m_type);
			controller.updateInfo(new GuiDataSendListsStandards(m_type, controller.getContext().getStandards(m_type)));

			CONTROLLOG << "controll::standard::DuplicateStandard do" << LOGENDL;
		}

		ControlType DuplicateStandard::getType() const
		{
			return (ControlType::duplicateStandard);
		}

		//controll::standard::RenameStandard

		RenameStandard::RenameStandard(const SafePtr<StandardList>& standard, const std::wstring& newName, const StandardType& type)
			: m_standard(standard)
			, m_newName(newName)
			, m_type(type)
		{}

		RenameStandard::~RenameStandard()
		{}

		void RenameStandard::doFunction(Controller& controller)
		{
			
			for (SafePtr<StandardList> standard : controller.getContext().getStandards(m_type))
			{
				ReadPtr<StandardList> rStand = standard.cget();
				if (rStand && rStand->getName() == m_newName)
				{
					controller.updateInfo(new GuiDataWarning(TEXT_STANDARDS_NAME_EXIST));
					controller.updateInfo(new GuiDataSendListsStandards(m_type, controller.getContext().getStandards(m_type)));
					return;
				}
			}

			WritePtr<StandardList> wStand = m_standard.get();
			if (wStand)
				wStand->setName(m_newName);

			controller.updateInfo(new GuiDataSendListsStandards(m_type, controller.getContext().getStandards(m_type)));
		}

		ControlType RenameStandard::getType() const
		{
			return (ControlType::renameStandard);
		}

		//controll::standard::SendInfoStandard

		SendInfoStandard::SendInfoStandard(const SafePtr<StandardList>& standard, const StandardType& type)
			: m_standard(standard)
			, m_type(type)
		{}

		SendInfoStandard::~SendInfoStandard()
		{}

		void SendInfoStandard::doFunction(Controller& controller)
		{
			controller.updateInfo(new GuiDataSendStandards(m_type, m_standard));
		}

		ControlType SendInfoStandard::getType() const
		{
			return (ControlType::sendInfoStandard);
		}

		//controll::standard::AddItemToStandard

		AddItemToStandard::AddItemToStandard(const SafePtr<StandardList>& standard, const double& newItemName, const StandardType& type)
			: m_standard(standard)
			, m_newName(newItemName)
			, m_type(type)
		{}

		AddItemToStandard::~AddItemToStandard()
		{}

		void AddItemToStandard::doFunction(Controller& controller)
		{
			WritePtr<StandardList> wStand = m_standard.get();
			if (!wStand)
				return;

			if (wStand->clist().find(m_newName) != wStand->clist().end())
				controller.updateInfo(new GuiDataWarning(TEXT_STANDARDS_ITEM_EXIST));

			wStand->list().insert(m_newName);

			controller.updateInfo(new GuiDataSendStandards(m_type, m_standard));
		}

		ControlType AddItemToStandard::getType() const
		{
			return (ControlType::addItemStandard);
		}

		//controll::standard::RemoveItemFromList

		RemoveItemFromList::RemoveItemFromList(const SafePtr<StandardList>& standard, const double& itemName, const StandardType& type)
			: m_standard(standard)
			, m_itemName(itemName)
			, m_type(type)
		{}

		RemoveItemFromList::~RemoveItemFromList()
		{}

		void RemoveItemFromList::doFunction(Controller& controller)
		{
			WritePtr<StandardList> wStand = m_standard.get();
			if (!wStand)
				return;

			wStand->list().erase(m_itemName);
			controller.updateInfo(new GuiDataSendStandards(m_type, m_standard));
		}

		ControlType RemoveItemFromList::getType() const
		{
			return (ControlType::removeItemStandard);
		}

		//controll::standard::RenameItemFromList

		RenameItemFromList::RenameItemFromList(const SafePtr<StandardList>& standard, const double& oldName, const double& newName, const StandardType& type)
			: m_standard(standard)
			, m_oldItemName(oldName)
			, m_newItemName(newName)
			, m_type(type)
		{}

		RenameItemFromList::~RenameItemFromList()
		{}

		void RenameItemFromList::doFunction(Controller& controller)
		{
			WritePtr<StandardList> wStand = m_standard.get();
			if (!wStand)
				return;

			if (wStand->clist().find(m_newItemName) != wStand->clist().end())
				controller.updateInfo(new GuiDataWarning(TEXT_STANDARDS_ITEM_EXIST_RENAME));

			wStand->list().erase(m_oldItemName);
			wStand->list().insert(m_newItemName);

			controller.updateInfo(new GuiDataSendStandards(m_type, m_standard));
		}

		ControlType RenameItemFromList::getType() const
		{
			return (ControlType::renameItemStandard);
		}

		//controll::standard::ClearItemFromList

		ClearItemFromList::ClearItemFromList(const SafePtr<StandardList>& standard, const StandardType& type)
			: m_standard(standard)
			, m_type(type)
		{}

		ClearItemFromList::~ClearItemFromList()
		{}

		void ClearItemFromList::doFunction(Controller& controller)
		{
			WritePtr<StandardList> wStand = m_standard.get();
			if (!wStand)
				return;

			wStand->list().clear();

			controller.updateInfo(new GuiDataSendStandards(m_type, m_standard));
		}

		ControlType ClearItemFromList::getType() const
		{
			return (ControlType::clearStandard);
		}

		//controll::standard::SaveLists

		SaveLists::SaveLists(const StandardType& type)
			: m_type(type)
		{ }

		SaveLists::~SaveLists()
		{ }

		void SaveLists::doFunction(Controller& controller)
		{
			std::unordered_set<SafePtr<StandardList>> lists = controller.getContext().getStandards(m_type);

			if (!lists.empty())
			{
				std::filesystem::path outdir;
				switch (m_type)
				{
				case StandardType::Pipe:
					outdir = controller.getContext().cgetProjectInternalInfo().getTemplatesFolderPath() / PIPE_FILE;
					break;
				case StandardType::Sphere:
					outdir = controller.getContext().cgetProjectInternalInfo().getTemplatesFolderPath() / SPHERE_FILE;
					break;
				}
				SaveLoadSystem::ExportLists<StandardList>(lists, outdir);
			}
		}

		bool SaveLists::canUndo() const
		{
			return (false);
		}

		void SaveLists::undoFunction(Controller& controller)
		{ }

		ControlType SaveLists::getType() const
		{
			return (ControlType::saveStandards);
		}

		//controll::standard::ReloadLists

		ReloadLists::ReloadLists(const StandardType& type)
			: m_type(type)
		{ }

		ReloadLists::~ReloadLists()
		{ }

		void ReloadLists::doFunction(Controller& controller)
		{
			std::filesystem::path indir;
			switch (m_type)
			{
			case StandardType::Pipe:
				indir = controller.getContext().cgetProjectInternalInfo().getTemplatesFolderPath() / PIPE_FILE;
				break;
			case StandardType::Sphere:
				indir = controller.getContext().cgetProjectInternalInfo().getTemplatesFolderPath() / SPHERE_FILE;
				break;
			}
			controller.getContext().setStandards(SaveLoadSystem::ImportLists<StandardList>(indir), m_type);
		}

		bool ReloadLists::canUndo() const
		{
			return (false);
		}

		void ReloadLists::undoFunction(Controller& controller)
		{ }

		ControlType ReloadLists::getType() const
		{
			return (ControlType::reloadStandards);
		}

		//controll::standard::ImportList

		ImportList::ImportList(const std::filesystem::path& path, const StandardType& type)
			: m_path(path)
			, m_type(type)
		{}

		ImportList::~ImportList()
		{}

		void ImportList::doFunction(Controller& controller)
		{
			StandardList newlist = SaveLoadSystem::ImportNewList<StandardList>(m_path);
			if (newlist.isValid() == true)
			{
				uint16_t count(1);
				bool conti(true);
				while (conti)
				{
					conti = false;
					for (const SafePtr<StandardList>& standard : controller.getContext().getStandards(m_type))
					{
						ReadPtr<StandardList> rStand = standard.cget();
						if (rStand && rStand->getName() == newlist.getName())
						{
							newlist.setName(newlist.getName() + L"_" + std::to_wstring(count++));
							conti = true;
							break;
						}
					}
					
				}

				controller.getContext().setStandards({ newlist }, m_type);
				controller.updateInfo(new GuiDataSendListsStandards(m_type, controller.getContext().getStandards(m_type)));
			}
			else
			{
				controller.updateInfo(new GuiDataWarning(TEXT_LIST_OPEN_FILE_FAILED));
			}
		}

		ControlType ImportList::getType() const
		{
			return (ControlType::importStandard);
		}

		//controll::standard::ExportList

		ExportList::ExportList(std::filesystem::path path, const SafePtr<StandardList>& standard, const StandardType& type)
			: m_standard(standard)
			, m_path(path)
			, m_type(type)
		{}

		ExportList::~ExportList()
		{}

		void ExportList::doFunction(Controller& controller)
		{
			std::wstring msg = SaveLoadSystem::ExportCSVList<StandardList>(m_standard, m_path).wstring();
			if (msg.empty())
				controller.updateInfo(new GuiDataWarning(TEXT_LIST_EXPORT_FAILED));
			else
				controller.updateInfo(new GuiDataInfo(TEXT_LIST_EXPORT_SUCCESS.arg(QString::fromStdWString(msg)), true));
		}

		ControlType ExportList::getType() const
		{
			return (ControlType::exportStandard);
		}
	}
}