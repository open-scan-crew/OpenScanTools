#include "gui/toolBars/ToolBarExportPointCloud.h"
#include "controller/controls/ControlExportPC.h"
#include "gui/GuiData/GuiDataGeneralProject.h"

ToolBarExportPointCloud::ToolBarExportPointCloud(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);
	setEnabled(false);

	m_ui.toolButton_clipping->setIconSize(QSize(20, 20) * guiScale);
	m_ui.toolButton_grid->setIconSize(QSize(20, 20) * guiScale);
	m_ui.toolButton_scans->setIconSize(QSize(20, 20) * guiScale);
	m_ui.toolButton_pco->setIconSize(QSize(20, 20) * guiScale);

	QObject::connect(m_ui.toolButton_grid, &QToolButton::released, this, &ToolBarExportPointCloud::slotExportGrid);
	QObject::connect(m_ui.toolButton_clipping, &QToolButton::released, this, &ToolBarExportPointCloud::slotExportClipping);
	QObject::connect(m_ui.toolButton_scans, &QToolButton::released, this, &ToolBarExportPointCloud::slotExportScans);
	QObject::connect(m_ui.toolButton_pco, &QToolButton::released, this, &ToolBarExportPointCloud::slotExportPCOs);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_methods.insert({ guiDType::projectLoaded, &ToolBarExportPointCloud::onProjectLoad });
}

ToolBarExportPointCloud::~ToolBarExportPointCloud()
{
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarExportPointCloud::informData(IGuiData *data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		ExportPCGroupMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarExportPointCloud::onProjectLoad(IGuiData * data)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(plData->m_isProjectLoad);
}

void ToolBarExportPointCloud::slotExportGrid()
{
	ExportInitMessage message(false, true, true, false, ObjectStatusFilter::VISIBLE);
    m_dataDispatcher.sendControl(new control::exportPC::StartExport(message));
}

void ToolBarExportPointCloud::slotExportClipping()
{
	ExportInitMessage message(true, false, true, false, ObjectStatusFilter::VISIBLE);
    m_dataDispatcher.sendControl(new control::exportPC::StartExport(message));
}

void ToolBarExportPointCloud::slotExportScans()
{
	ExportInitMessage message(false, false, true, false, ObjectStatusFilter::VISIBLE);
    m_dataDispatcher.sendControl(new control::exportPC::StartExport(message));
}

void ToolBarExportPointCloud::slotExportPCOs()
{
	ExportInitMessage message(false, false, false, true, ObjectStatusFilter::VISIBLE);
	m_dataDispatcher.sendControl(new control::exportPC::StartExport(message));
}
