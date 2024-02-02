#ifndef CONTROL_PIPESTANDARDS_H_
#define CONTROL_PIPESTANDARDS_H_

#include <filesystem>

#include "controller/controls/IControl.h"
#include "models/application/List.h"
#include "models/OpenScanToolsModelEssentials.h"

namespace control
{
	namespace standards
	{
		class SelectStandard : public AControl
		{
		public:
			SelectStandard(const SafePtr<StandardList>& standard, const StandardType& type);
			~SelectStandard();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<StandardList> m_standard;
			const StandardType m_type;
		};

		class SendStandards : public AControl
		{
		public:
			SendStandards(const StandardType& type);
			~SendStandards();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const StandardType m_type;
		};

		class CreateStandard : public AControl
		{
		public:
			CreateStandard(const std::wstring& name, const StandardType& type);
			~CreateStandard();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const std::wstring m_name;
			const StandardType m_type;
		};
		
		class DeleteStandard : public AControl
		{
		public:
			DeleteStandard(const SafePtr<StandardList>& standard, const StandardType& type);
			~DeleteStandard();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<StandardList> m_standard;
			const StandardType m_type;
		};

		class DuplicateStandard : public AControl
		{
		public:
			DuplicateStandard(const SafePtr<StandardList>& standard, const StandardType& type);
			~DuplicateStandard();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<StandardList> m_standard;
			const StandardType m_type;
		};

		class RenameStandard : public AControl
		{
		public:
			RenameStandard(const SafePtr<StandardList>& standard, const std::wstring& newName, const StandardType& type);
			~RenameStandard();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const SafePtr<StandardList> m_standard;
			const std::wstring m_newName;
			const StandardType m_type;
		};

		class SendInfoStandard : public AControl
		{
		public:
			SendInfoStandard(const SafePtr<StandardList>& standard, const StandardType& type);
			~SendInfoStandard();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const SafePtr<StandardList> m_standard;
			const StandardType m_type;
		};

		class AddItemToStandard : public AControl
		{
		public:
			AddItemToStandard(const SafePtr<StandardList>& standard, const double& newItemName, const StandardType& type);
			~AddItemToStandard();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const SafePtr<StandardList> m_standard;
			const double m_newName;
			const StandardType m_type;
		};

		class RemoveItemFromList : public AControl
		{
		public:
			RemoveItemFromList(const SafePtr<StandardList>& standard, const double& itemName, const StandardType& type);
			~RemoveItemFromList();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const SafePtr<StandardList> m_standard;
			const double m_itemName;
			bool m_works;
			const StandardType m_type;
		};

		class RenameItemFromList : public AControl
		{
		public:
			RenameItemFromList(const SafePtr<StandardList>& standard, const double& oldItemName, const double& newItemName, const StandardType& type);
			~RenameItemFromList();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const SafePtr<StandardList>& m_standard;
			const double m_oldItemName;
			const double m_newItemName;
			const StandardType m_type;
		};

		class ClearItemFromList : public AControl
		{
		public:
			ClearItemFromList(const SafePtr<StandardList>& standard, const StandardType& type);
			~ClearItemFromList();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const SafePtr<StandardList> m_standard;
			std::set<double> m_storedItems;
			const StandardType m_type;
		};

		class SaveLists : public AControl
		{
		public:
			SaveLists(const StandardType& type);
			~SaveLists();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const StandardType m_type;
		};

		class ReloadLists : public AControl
		{
		public:
			ReloadLists(const StandardType& type);
			~ReloadLists();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const StandardType m_type;
		};

		class ImportList : public AControl
		{
		public:
			ImportList(const std::filesystem::path& path, const StandardType& type);
			~ImportList();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const std::filesystem::path m_path;
			const StandardType m_type;
		};

		class ExportList : public AControl
		{
		public:
			ExportList(std::filesystem::path path, const SafePtr<StandardList>& standard, const StandardType& type);
			~ExportList();
			void doFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const SafePtr<StandardList> m_standard;
			const std::filesystem::path m_path;
			const StandardType m_type;
		};
	}
}

#endif // !CONTROL_PIPESTANDARDS_H_
