#ifndef GUI_DATA_TEMPLATE_H_
#define GUI_DATA_TEMPLATE_H_

#include "gui/GuiData/IGuiData.h"
#include "models/application/TagTemplate.h"
#include "models/application/List.h"
#include "models/OpenScanToolsModelEssentials.h"

#include <unordered_set>

class GuiDataSendTemplateList : public IGuiData
{
public:
	GuiDataSendTemplateList(const std::unordered_set<SafePtr<sma::TagTemplate>>& templates);
	~GuiDataSendTemplateList();
	guiDType getType() override;
public:
	const std::unordered_set<SafePtr<sma::TagTemplate>> m_templates;
};

class GuiDataSendTagTemplate : public IGuiData
{
public:
	GuiDataSendTagTemplate(const SafePtr<sma::TagTemplate>& temp, const std::unordered_set<SafePtr<UserList>>& lists);
	~GuiDataSendTagTemplate();
	guiDType getType() override;
public:
	SafePtr<sma::TagTemplate> m_temp;
	std::unordered_set<SafePtr<UserList>> m_lists;
};

class GuiDataTagTemplateSelected : public IGuiData
{
public:
	GuiDataTagTemplateSelected(const SafePtr<sma::TagTemplate>& temp);
	~GuiDataTagTemplateSelected();
	guiDType getType() override;
public:
	SafePtr<sma::TagTemplate> m_temp;
};

#endif // !GUI_DATA_TEMPLATE_H_
