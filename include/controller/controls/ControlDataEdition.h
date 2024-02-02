#ifndef CONTROL_DATA_EDITION_H_
#define CONTROL_DATA_EDITION_H_

#include "controller/controls/AEditionControl.h"
#include "models/data/Data.h"

class AObjectNode;

namespace control
{
	namespace dataEdition
	{
		class SetColor : public ATEditionControl<AObjectNode, Color32>
		{
		public:
			SetColor(SafePtr<AObjectNode> toEditData, const Color32& newColor);
			SetColor(const std::unordered_set<SafePtr<AObjectNode>>& toEditDatas, const Color32& newColor);
			~SetColor();
			ControlType getType() const override;
		};

		class SetUserId : public AEditionControl
		{
		public:
			SetUserId(SafePtr<AObjectNode> toEditData, uint32_t newId);
			~SetUserId();
			bool changeId(Controller& controller, uint32_t toChangeId, std::string actionText);
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			void redoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<AObjectNode> m_toEditData;
			uint32_t m_oldId;
			uint32_t m_newId;
			ElementType m_type;
		};

		class SetDescription : public ATEditionControl<AObjectNode, std::wstring>
		{
		public:
			SetDescription(SafePtr<AObjectNode> toEditData, const std::wstring& newDesc);
			SetDescription(const std::unordered_set<SafePtr<AObjectNode>>& toEditDatas, const std::wstring& newDesc);
			~SetDescription();
			ControlType getType() const override;
		};

		class SetDiscipline : public ATEditionControl<AObjectNode, std::wstring>
		{
		public:
			SetDiscipline(SafePtr<AObjectNode> toEditData, const std::wstring& discipline);
			SetDiscipline(const std::unordered_set<SafePtr<AObjectNode>>& toEditDatas, const std::wstring& discipline);
			~SetDiscipline();
			ControlType getType() const override;
		};

		class SetPhase : public ATEditionControl<AObjectNode, std::wstring>
		{
		public:
			SetPhase(SafePtr<AObjectNode> toEditData, const std::wstring& prefix);
			SetPhase(const std::unordered_set<SafePtr<AObjectNode>>& toEditDatas, const std::wstring& prefix);
			~SetPhase();
			ControlType getType() const override;
		};

		class SetIdentifier : public ATEditionControl<AObjectNode, std::wstring>
		{
		public:
			SetIdentifier(SafePtr<AObjectNode> toEditData, const std::wstring& identifer);
			SetIdentifier(const std::unordered_set<SafePtr<AObjectNode>>& toEditDatas, const std::wstring& identifer);
			~SetIdentifier();
			ControlType getType() const override;
		};

		class SetName : public ATEditionControl<AObjectNode, std::wstring>
		{
		public:
			SetName(SafePtr<AObjectNode> toEditData, const std::wstring& name);
			SetName(const std::unordered_set<SafePtr<AObjectNode>>& toEditDatas, const std::wstring& name);
			~SetName();
			ControlType getType() const override;
		};

		class SetHyperLinks : public ATEditionControl<AObjectNode, std::unordered_map<hLinkId, s_hyperlink>>
		{
		public:
			SetHyperLinks(SafePtr<AObjectNode> toEditData, const std::vector<s_hyperlink>& links);
			SetHyperLinks(const std::unordered_set<SafePtr<AObjectNode>>& toEditDatas, const std::vector<s_hyperlink>& links);
			~SetHyperLinks();
			ControlType getType() const override;
		};

		class addHyperlink : public AEditionControl
		{
		public:
			addHyperlink(SafePtr<AObjectNode> toEditData, const std::wstring& link, std::wstring name);
			addHyperlink(const std::unordered_set<SafePtr<AObjectNode>>& toEditDatas, const std::wstring& link, std::wstring name);
			~addHyperlink();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			const std::unordered_set<SafePtr<AObjectNode>> m_dataPtrs;
			std::wstring m_link;
			std::wstring m_name;
			std::unordered_map<SafePtr<AObjectNode>, hLinkId> m_dataPtrLinks;
		};

		class removeHyperlink : public AEditionControl
		{
		public:
			removeHyperlink(SafePtr<AObjectNode> toEditData, hLinkId idToDel);
			~removeHyperlink();
			void doFunction(Controller& controller) override;
			bool canUndo() const override;
			void undoFunction(Controller& controller) override;
			ControlType getType() const override;
		private:
			SafePtr<AObjectNode> m_dataPtr;
			hLinkId m_idToDel;
			std::wstring m_link;
			std::wstring m_name;
			bool m_canUndo;
		};
	}
}

#endif // !CONTROL_DATA_EDITION_H_