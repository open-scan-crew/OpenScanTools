#include "gui/GuiData/GuiDataList.h"


GuiDataSendListsList::GuiDataSendListsList(std::unordered_set<SafePtr<UserList>> lists)
{
	_lists = lists;
}

GuiDataSendListsList::~GuiDataSendListsList()
{}

guiDType GuiDataSendListsList::getType()
{
	return (guiDType::sendListsList);
}

GuiDataSendUserlist::GuiDataSendUserlist(SafePtr<UserList> list)
{
	_list = list;
}

GuiDataSendUserlist::~GuiDataSendUserlist()
{ }

guiDType GuiDataSendUserlist::getType()
{
	return (guiDType::sendUserList);
}


GuiDataSendListsStandards::GuiDataSendListsStandards(const StandardType& type, const std::unordered_set<SafePtr<StandardList>>& lists)
	: m_type(type)
	, m_lists(lists)
{}

GuiDataSendListsStandards::~GuiDataSendListsStandards()
{}

guiDType GuiDataSendListsStandards::getType()
{
	return (guiDType::sendListsStandards);
}

GuiDataSendStandards::GuiDataSendStandards(const StandardType& type, const SafePtr<StandardList>& list)
	: m_type(type)
	, m_list(list)
{}

GuiDataSendStandards::~GuiDataSendStandards()
{}

guiDType GuiDataSendStandards::getType()
{
	return (guiDType::sendStandardList);
}


GuiDataSendDisciplineSelected::GuiDataSendDisciplineSelected(const uint32_t& id)
	: m_id(id)
{}

GuiDataSendDisciplineSelected::~GuiDataSendDisciplineSelected()
{}

guiDType GuiDataSendDisciplineSelected::getType()
{
	return (guiDType::sendDisciplineSelected);
}

GuiDataSendPhaseSelected::GuiDataSendPhaseSelected(const uint32_t& id) 
	: m_id(id)
{}

GuiDataSendPhaseSelected::~GuiDataSendPhaseSelected()
{}

guiDType GuiDataSendPhaseSelected::getType()
{
	return (guiDType::sendPhaseSelected);
}

/* GuiDataSendUserOrientation */