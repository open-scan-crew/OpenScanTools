#include "gui/GuiData/GuiDataTemplate.h"

GuiDataSendTemplateList::GuiDataSendTemplateList(const std::unordered_set<SafePtr<sma::TagTemplate>>& templates)
	: m_templates(templates)
{}

GuiDataSendTemplateList::~GuiDataSendTemplateList()
{}

guiDType GuiDataSendTemplateList::getType()
{
	return (guiDType::sendTemplateList);
}

GuiDataSendTagTemplate::GuiDataSendTagTemplate(const SafePtr<sma::TagTemplate>& temp, const std::unordered_set<SafePtr<UserList>>& lists)
	: m_temp(temp)
	, m_lists(lists)
{ }

GuiDataSendTagTemplate::~GuiDataSendTagTemplate()
{ }

guiDType GuiDataSendTagTemplate::getType()
{
	return (guiDType::sendTagTemplate);
}

GuiDataTagTemplateSelected::GuiDataTagTemplateSelected(const SafePtr<sma::TagTemplate>& temp)
	: m_temp(temp)
{}

GuiDataTagTemplateSelected::~GuiDataTagTemplateSelected()
{}

guiDType GuiDataTagTemplateSelected::getType() 
{
	return (guiDType::sendTagTemplateSelected);
}