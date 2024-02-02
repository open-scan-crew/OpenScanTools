#include "controller/controls/ControlUserList.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "gui/GuiData/GuiDataList.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "io/SaveLoadSystem.h"
#include "gui/Texts.hpp"
#include "utils/Logger.h"
#include "utils/Utils.h"

namespace control
{
	namespace userlist
	{
		//controll::userlist::SendUserLists

		SendUserLists::SendUserLists()
		{

		}

		SendUserLists::~SendUserLists()
		{

		}

		void SendUserLists::doFunction(Controller& controller)
		{
			controller.updateInfo(new GuiDataSendListsList(controller.getContext().getUserLists()));
		}

		ControlType SendUserLists::getType() const
		{
			return (ControlType::sendUserLists);
		}

		//controll::userlist::CreateUserList

		CreateUserList::CreateUserList(std::wstring name)
		{
			_name = name;
		}

		CreateUserList::~CreateUserList()
		{

		}

		void CreateUserList::doFunction(Controller& controller)
		{
			UserList list;
			for (SafePtr<UserList> list : controller.getContext().getUserLists())
			{
				ReadPtr<UserList> rList = list.cget();
				if (rList && rList->getName() == _name)
				{
					controller.updateInfo(new GuiDataWarning(TEXT_USERLIST_NAME_EXIST));
					return;
				}
			}
			list = UserList(_name);
			controller.getContext().setUserLists({ list });
			controller.updateInfo(new GuiDataSendListsList(controller.getContext().getUserLists()));
			CONTROLLOG << "controll::userlist::CreateUserList do" << LOGENDL;
		}

		ControlType CreateUserList::getType() const
		{
			return (ControlType::createUserList);
		}

		//controll::userlist::DeleteUserList

		DeleteUserList::DeleteUserList(SafePtr<UserList> list)
		{
			_list = list;
		}

		DeleteUserList::~DeleteUserList()
		{

		}

		void DeleteUserList::doFunction(Controller& controller)
		{
			controller.getContext().removeUserLists(_list);
			controller.updateInfo(new GuiDataSendListsList(controller.getContext().getUserLists()));
			CONTROLLOG << "controll::userlist::DeleteUserList do" << LOGENDL;
		}

		ControlType DeleteUserList::getType() const
		{
			return (ControlType::deleteUserList);
		}

		//controll::userlist::DuplicateUserList

		DuplicateUserList::DuplicateUserList(SafePtr<UserList> list)
		{
			_list = list;
		}

		DuplicateUserList::~DuplicateUserList()
		{

		}

		void DuplicateUserList::doFunction(Controller& controller)
		{
			UserList newList;

			{
				ReadPtr<UserList> rList = _list.cget();
				if (!rList)
					return;
				newList = *&rList;
			}
			
			std::wstring name;
			int nb = 1;
			while (nb < 1000)
			{
				std::wstringstream ss;
				ss << newList.getName() << "_" << nb;
				if (controller.getContext().verifNameForList(ss.str()) == true)
				{
					name = ss.str();
					break;
				}
				++nb;
			}
			if (nb == 1000)
				return;

			newList.setName(name);
			newList.setId(xg::newGuid());

			controller.getContext().setUserLists({ newList });
			controller.updateInfo(new GuiDataSendListsList(controller.getContext().getUserLists()));

			CONTROLLOG << "controll::userlist::DuplicateUserList do" << LOGENDL;
		}

		ControlType DuplicateUserList::getType() const
		{
			return (ControlType::duplicateUserList);
		}

		//controll::userlist::RenameUserList

		RenameUserList::RenameUserList(SafePtr<UserList> list, std::wstring newName)
		{
			_list = list;
			_newName = newName;
		}

		RenameUserList::~RenameUserList()
		{

		}

		void RenameUserList::doFunction(Controller& controller)
		{
			for (SafePtr<UserList> list : controller.getContext().getUserLists())
			{
				ReadPtr<UserList> rList = list.cget();
				if (rList && rList->getName() == _newName)
				{
					controller.updateInfo(new GuiDataWarning(TEXT_USERLIST_NAME_EXIST));
					return;
				}
			}

			WritePtr<UserList> wList = _list.get();
			if (!wList)
				return;

			wList->setName(_newName);

			controller.updateInfo(new GuiDataSendUserlist(_list));
		}

		ControlType RenameUserList::getType() const
		{
			return (ControlType::renameUserList);
		}

		//controll::userlist::SendInfoUserList

		SendInfoUserList::SendInfoUserList(SafePtr<UserList> list)
		{
			_list = list;
		}

		SendInfoUserList::~SendInfoUserList()
		{

		}

		void SendInfoUserList::doFunction(Controller& controller)
		{
			controller.updateInfo(new GuiDataSendUserlist(_list));
		}

		ControlType SendInfoUserList::getType() const
		{
			return (ControlType::sendInfoUserList);
		}

		//controll::userlist::AddItemToUserList

		AddItemToUserList::AddItemToUserList(SafePtr<UserList> list, std::wstring newItemName)
		{
			_list = list;
			_newName = newItemName;
		}

		AddItemToUserList::~AddItemToUserList()
		{

		}

		void AddItemToUserList::doFunction(Controller& controller)
		{
			WritePtr<UserList> wList = _list.get();
			if (!wList)
				return;

			wList->list().insert(_newName);

			controller.updateInfo(new GuiDataSendUserlist(_list));
		}

		ControlType AddItemToUserList::getType() const
		{
			return (ControlType::addItemUserList);
		}

		//controll::userlist::RemoveItemFromList

		RemoveItemFromList::RemoveItemFromList(SafePtr<UserList> list, std::wstring itemName)
		{
			_list = list;
			_itemName = itemName;
		}

		RemoveItemFromList::~RemoveItemFromList()
		{

		}

		void RemoveItemFromList::doFunction(Controller& controller)
		{
			WritePtr<UserList> wList = _list.get();
			if (!wList)
				return;

			wList->list().erase(_itemName);

			controller.updateInfo(new GuiDataSendUserlist(_list));
		}

		ControlType RemoveItemFromList::getType() const
		{
			return (ControlType::removeItemUserList);
		}

		//controll::userlist::RenameItemFromList

		RenameItemFromList::RenameItemFromList(SafePtr<UserList> list, std::wstring oldItemName, std::wstring newName)
		{
			_list = list;
			_newItemName = newName;
			_oldItemName = oldItemName;
		}

		RenameItemFromList::~RenameItemFromList()
		{}

		void RenameItemFromList::doFunction(Controller& controller)
		{
			WritePtr<UserList> wList = _list.get();
			if (!wList)
				return;

			if (wList->clist().find(_newItemName) != wList->clist().end())
			{
				controller.updateInfo(new GuiDataWarning(TEXT_USERLIST_ITEM_EXIST_RENAME));
				return;
			}

			if (wList->clist().find(_oldItemName) != wList->clist().end())
			{
				wList->list().erase(_oldItemName);
				wList->list().insert(_newItemName);
			}

			controller.updateInfo(new GuiDataSendUserlist(_list));
		}

		ControlType RenameItemFromList::getType() const
		{
			return (ControlType::renameItemUserList);
		}

		//controll::userlist::ClearItemFromList

		ClearItemFromList::ClearItemFromList(SafePtr<UserList> list)
		{
			_list = list;
		}

		ClearItemFromList::~ClearItemFromList()
		{ }

		void ClearItemFromList::doFunction(Controller& controller)
		{
			WritePtr<UserList> wList = _list.get();
			if (wList)
			{
				wList->list().clear();
				controller.updateInfo(new GuiDataSendUserlist(_list));
			}
		}

		ControlType ClearItemFromList::getType() const
		{
			return (ControlType::clearUserList);
		}

		//controll::userlist::SaveLists

		SaveLists::SaveLists()
		{ }

		SaveLists::~SaveLists()
		{ }

		void SaveLists::doFunction(Controller& controller)
		{
			controller.updateInfo(new GuiDataSendListsList(controller.getContext().getUserLists()));
			SaveLoadSystem::ExportLists<UserList>(controller.getContext().getUserLists(), controller.getContext().cgetProjectInternalInfo().getTemplatesFolderPath() / File_Lists);
		}

		ControlType SaveLists::getType() const
		{
			return (ControlType::saveLists);
		}

		//controll::userlist::ReloadLists

		ReloadLists::ReloadLists()
		{ }

		ReloadLists::~ReloadLists()
		{ }

		void ReloadLists::doFunction(Controller& controller)
		{
			controller.getContext().setUserLists(SaveLoadSystem::ImportLists<UserList>(controller.getContext().cgetProjectInternalInfo().getTemplatesFolderPath() / "Lists.tll"));
		}

		ControlType ReloadLists::getType() const
		{
			return (ControlType::reloadLists);
		}

		//controll::userlist::ImportList

		ImportList::ImportList(std::filesystem::path path)
		{
			_path = path;
		}

		ImportList::~ImportList()
		{ }

		void ImportList::doFunction(Controller& controller)
		{
			UserList newlist = SaveLoadSystem::ImportNewList<UserList>(_path);

			if (newlist.isValid() == true)
			{
				controller.getContext().setUserLists({ newlist });
				controller.updateInfo(new GuiDataSendListsList(controller.getContext().getUserLists()));
			}
			else
				controller.updateInfo(new GuiDataWarning(TEXT_LIST_OPEN_FILE_FAILED));
		}

		ControlType ImportList::getType() const
		{
			return (ControlType::importList);
		}

		//controll::userlist::ExportList

		ExportList::ExportList(std::filesystem::path path, SafePtr<UserList> list)
		{
			m_path = path;
			m_list = list;
		}

		ExportList::~ExportList()
		{ }

		void ExportList::doFunction(Controller& controller)
		{
			if (m_path == "")
			{
				controller.updateInfo(new GuiDataWarning(TEXT_NO_DIRECTORY_SELECTED));
				return;
			}

			std::wstring msg;
			ReadPtr<UserList> rUl = m_list.cget();
			if(rUl)
				msg = SaveLoadSystem::ExportCSVList<UserList>(*&rUl, m_path).wstring();

			if (msg.empty())
				controller.updateInfo(new GuiDataWarning(TEXT_LIST_EXPORT_FAILED));
			else
				controller.updateInfo(new GuiDataInfo(TEXT_LIST_EXPORT_SUCCESS.arg(QString::fromStdWString(msg)), true));
			
		}

		ControlType ExportList::getType() const
		{
			return (ControlType::exportList);
		}
	}
}