#include "gui/toolBars/ToolBarExportGroup.h"
#include "controller/controls/ControlFunction.h"
#include "gui/GuiData/GuiDataGeneralProject.h"

#include "utils/Logger.h"

ToolBarExportGroup::ToolBarExportGroup(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
	, m_primitivesExportDialog(new DialogExportPrimitives(dataDispatcher, this, guiScale))
{
	m_ui.setupUi(this);
	setEnabled(false);
		
	QObject::connect(m_ui.exportToCsvButton, &QToolButton::clicked, this, &ToolBarExportGroup::slotExportCsv);
	QObject::connect(m_ui.exportToDxfButton, &QToolButton::clicked, this, &ToolBarExportGroup::slotExportDxf);
	QObject::connect(m_ui.exportToStepButton, &QToolButton::clicked, this, &ToolBarExportGroup::slotExportStep);
	QObject::connect(m_ui.exportToObjButton, &QToolButton::clicked, this, &ToolBarExportGroup::slotExportObj);
	QObject::connect(m_ui.exportToFbxButton, &QToolButton::clicked, this, &ToolBarExportGroup::slotExportFbx);
	
	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);

	m_methods.insert({ guiDType::projectLoaded, &ToolBarExportGroup::onProjectLoad });


}

ToolBarExportGroup::~ToolBarExportGroup()
{
	m_primitivesExportDialog->close();
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarExportGroup::informData(IGuiData *data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		ExportGroupMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarExportGroup::onProjectLoad(IGuiData* data)
{
	setEnabled(static_cast<GuiDataProjectLoaded*>(data)->m_isProjectLoad);
}

void ToolBarExportGroup::slotExportCsv()
{
	m_dataDispatcher.sendControl(new control::function::Abort());
	m_primitivesExportDialog->setFormat(ObjectExportType::CSV);
	m_primitivesExportDialog->show();
	GUI_LOG << "exportCSV click" << LOGENDL;
}

void ToolBarExportGroup::slotExportDxf()
{
	m_dataDispatcher.sendControl(new control::function::Abort());
	m_primitivesExportDialog->setFormat(ObjectExportType::DXF);
	m_primitivesExportDialog->show();
	GUI_LOG << "exportDxf click" << LOGENDL;
}

void ToolBarExportGroup::slotExportStep()
{
	m_dataDispatcher.sendControl(new control::function::Abort());
	m_primitivesExportDialog->setFormat(ObjectExportType::STEP);
	m_primitivesExportDialog->show();
	GUI_LOG << "exportStep click" << LOGENDL;
}

void ToolBarExportGroup::slotExportObj()
{
	m_dataDispatcher.sendControl(new control::function::Abort());
	m_primitivesExportDialog->setFormat(ObjectExportType::OBJ);
	m_primitivesExportDialog->show();
	GUI_LOG << "exportStep click" << LOGENDL;
}

void ToolBarExportGroup::slotExportFbx()
{
	m_dataDispatcher.sendControl(new control::function::Abort());
	m_primitivesExportDialog->setFormat(ObjectExportType::FBX);
	m_primitivesExportDialog->show();
	GUI_LOG << "exportStep click" << LOGENDL;
}