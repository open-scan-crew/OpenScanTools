#include "gui/toolBars/ToolBarListGroup.h"

#include "gui/Dialog/ListListDialog.h"
#include "gui/Dialog/TemplateManagerDialog.h"
#include "controller/controls/ControlUserList.h"
#include "controller/controls/ControlTemplateEdit.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataTemplate.h"
#include "gui/GuiData/GuiDataMessages.h"

ToolBarListGroup::ToolBarListGroup(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);
	setEnabled(false);

	connect(m_ui.ListManagerBtn, SIGNAL(clicked()), this, SLOT(manageLists()));
	connect(m_ui.TemplatesManagerBtn, SIGNAL(clicked()), this, SLOT(manageTemplate()));

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
}

ToolBarListGroup::~ToolBarListGroup()
{
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarListGroup::informData(IGuiData *data)
{
	if (data->getType() == guiDType::projectLoaded)
		onProjectLoad(data);
}

void ToolBarListGroup::onProjectLoad(IGuiData* data)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(plData->m_isProjectLoad);
}

void ToolBarListGroup::manageLists()
{
	ListListDialog *dialog = new ListListDialog(m_dataDispatcher, static_cast<QWidget*>(this->parent()), true);
	m_dataDispatcher.sendControl(new control::userlist::SendUserLists());
	dialog->show();
}

void ToolBarListGroup::manageTemplate()
{
	TemplateManagerDialog *dialog = new TemplateManagerDialog(m_dataDispatcher, static_cast<QWidget*>(this->parent()), true);
	m_dataDispatcher.sendControl(new control::tagTemplate::SendTemplateList());
	dialog->show();
}