#ifndef CONTROL_USER_LIST_H
#define CONTROL_USER_LIST_H

#include <filesystem>

#include "controller/controls/IControl.h"
#include "models/application/List.h"
#include "models/OpenScanToolsModelEssentials.h"

namespace control
{
	namespace userlist
	{
		class SendUserLists : public AControl
		{
		public:
			SendUserLists();
			~SendUserLists();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class CreateUserList : public AControl
		{
		public:
			CreateUserList(std::wstring name);
			~CreateUserList();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			std::wstring _name;
		};
		
		class DeleteUserList : public AControl
		{
		public:
			DeleteUserList(SafePtr<UserList> list);
			~DeleteUserList();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<UserList> _list;
		};

		class DuplicateUserList : public AControl
		{
		public:
			DuplicateUserList(SafePtr<UserList> list);
			~DuplicateUserList();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<UserList> _list;
		};

		class RenameUserList : public AControl
		{
		public:
			RenameUserList(SafePtr<UserList> list, std::wstring newName);
			~RenameUserList();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<UserList> _list;
			std::wstring _newName;
		};

		class SendInfoUserList : public AControl
		{
		public:
			SendInfoUserList(SafePtr<UserList> list);
			~SendInfoUserList();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<UserList> _list;
		};

		class AddItemToUserList : public AControl
		{
		public:
			AddItemToUserList(SafePtr<UserList> list, std::wstring newItemName);
			~AddItemToUserList();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<UserList> _list;
			std::wstring _newName;
		};

		class RemoveItemFromList : public AControl
		{
		public:
			RemoveItemFromList(SafePtr<UserList> list, std::wstring itemName);
			~RemoveItemFromList();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<UserList> _list;
			std::wstring _itemName;
		};

		class RenameItemFromList : public AControl
		{
		public:
			RenameItemFromList(SafePtr<UserList> list, std::wstring oldItemName, std::wstring newItemName);
			~RenameItemFromList();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<UserList> _list;
			std::wstring _newItemName;
			std::wstring _oldItemName;
		};

		class ClearItemFromList : public AControl
		{
		public:
			ClearItemFromList(SafePtr<UserList> list);
			~ClearItemFromList();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<UserList> _list;
		};

		class SaveLists : public AControl
		{
		public:
			SaveLists();
			~SaveLists();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class ReloadLists : public AControl
		{
		public:
			ReloadLists();
			~ReloadLists();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		};

		class ImportList : public AControl
		{
		public:
			ImportList(std::filesystem::path path);
			~ImportList();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			std::filesystem::path _path;
		};

		class ExportList : public AControl
		{
		public:
			ExportList(std::filesystem::path path, SafePtr<UserList> list);
			~ExportList();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<UserList> m_list;
			std::filesystem::path m_path;
		};
	}
}

#endif // !CONTROL_USER_LIST_H_
