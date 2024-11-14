#ifndef TOOLBAR_EXPORT_POINT_CLOUD_H
#define TOOLBAR_EXPORT_POINT_CLOUD_H

#include <QtWidgets/qwidget.h>
#include "ui_toolbar_exportPointCloud.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

class ToolBarExportPointCloud;

typedef void (ToolBarExportPointCloud::* ExportPCGroupMethod)(IGuiData*);

class ToolBarExportPointCloud : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarExportPointCloud(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale);

	void informData(IGuiData *keyValue) override;

private:
	void onProjectLoad(IGuiData *data);
    ~ToolBarExportPointCloud();

public slots:
	void slotExportGrid();
	void slotExportClipping();
	void slotExportScans();
	void slotExportPCOs();

private:
	std::unordered_map<guiDType, ExportPCGroupMethod> m_methods;
    IDataDispatcher &m_dataDispatcher;
	Ui::toolbar_exportPointCloud m_ui;
};

#endif // TOOLBAR_EXPORT_POINT_CLOUD_H

