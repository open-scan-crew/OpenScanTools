#include "gui/toolBars/ToolBarShareGroup.h"
#include "controller/controls/ControlFunction.h"
#include "gui/GuiData/GuiDataGeneralProject.h"


ToolBarShareGroup::ToolBarShareGroup(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
	, m_primitivesExportDialog(new DialogExportPrimitives(dataDispatcher, this, guiScale))
	, m_subProjectExportDialog(new DialogExportSubProject(dataDispatcher, this, guiScale))
{
	m_ui.setupUi(this);
	setEnabled(false);

	QObject::connect(m_ui.exportToOSTButton, &QToolButton::released, this, &ToolBarShareGroup::slotExportToShareObjects);
	QObject::connect(m_ui.exportSubProject, &QToolButton::released, this, &ToolBarShareGroup::slotExportToSubProject);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);

	m_methods.insert({ guiDType::projectLoaded, &ToolBarShareGroup::onProjectLoad });
}

ToolBarShareGroup::~ToolBarShareGroup()
{
	m_primitivesExportDialog->close();
	m_subProjectExportDialog->close();
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarShareGroup::informData(IGuiData *data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		ShareGroupMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarShareGroup::onProjectLoad(IGuiData* data)
{
	setEnabled(static_cast<GuiDataProjectLoaded*>(data)->m_isProjectLoad);
}

void ToolBarShareGroup::slotExportToShareObjects()
{
	m_dataDispatcher.sendControl(new control::function::Abort());
	m_primitivesExportDialog->setFormat(ObjectExportType::OST);
	m_primitivesExportDialog->show();
}


void ToolBarShareGroup::slotExportToSubProject()
{
	m_subProjectExportDialog->show();
}
