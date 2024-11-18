#ifndef GUI_DATA_LIST_H_
#define GUI_DATA_LIST_H_

#include "gui/GuiData/IGuiData.h"
#include "models/application/List.h"
#include "utils/safe_ptr.h"

#include <unordered_set>

class GuiDataSendListsList : public IGuiData
{
public:
	GuiDataSendListsList(std::unordered_set<SafePtr<UserList>> lists);
	~GuiDataSendListsList();
	guiDType getType() override;
public:
	std::unordered_set<SafePtr<UserList>> _lists;
};

class GuiDataSendUserlist : public IGuiData
{
public:
	GuiDataSendUserlist(SafePtr<UserList> list);
	~GuiDataSendUserlist();
	guiDType getType() override;
public:
	SafePtr<UserList> _list;
};

class GuiDataSendListsStandards : public IGuiData
{
public:
	GuiDataSendListsStandards(const StandardType& type, const std::unordered_set<SafePtr<StandardList>>& lists);
	~GuiDataSendListsStandards();
	guiDType getType() override;
public:
	const StandardType m_type;
	std::unordered_set<SafePtr<StandardList>> m_lists;
};


class GuiDataSendStandards : public IGuiData
{
public:
	GuiDataSendStandards(const StandardType& type, const SafePtr<StandardList>& list);
	~GuiDataSendStandards();
	guiDType getType() override;
public:
	const StandardType m_type;
	SafePtr<StandardList> m_list;
};

class GuiDataSendDisciplineSelected : public IGuiData
{
public:
	GuiDataSendDisciplineSelected(const uint32_t& id);
	~GuiDataSendDisciplineSelected();
	guiDType getType() override;
public:
	const uint32_t m_id;
};

class GuiDataSendPhaseSelected : public IGuiData
{
public:
	GuiDataSendPhaseSelected(const uint32_t& id);
	~GuiDataSendPhaseSelected();
	guiDType getType() override;
public:
	const uint32_t m_id;
};
#endif // !GUIDATA_LIST_H_
