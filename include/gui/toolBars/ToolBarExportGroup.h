#ifndef TOOLBAR_EXPORTGROUP_H
#define TOOLBAR_EXPORTGROUP_H

#include <QtWidgets/qwidget.h>
#include "ui_toolbar_exportgroup.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

#include "gui/Dialog/DialogExportPrimitives.h"

class ToolBarExportGroup;

typedef void (ToolBarExportGroup::* ExportGroupMethod)(IGuiData*);

class ToolBarExportGroup : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarExportGroup(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale);

	void informData(IGuiData *keyValue) override;

private:
    ~ToolBarExportGroup();
	void onProjectLoad(IGuiData *data);

public slots:
	void slotExportCsv();
	void slotExportDxf();
	void slotExportStep();
	void slotExportObj();
	void slotExportFbx();

private:
	std::unordered_map<guiDType, ExportGroupMethod> m_methods;
    IDataDispatcher &m_dataDispatcher;
	Ui::ToolBarExportGroup m_ui;
	QString m_openPath;

	DialogExportPrimitives* m_primitivesExportDialog;
};

#endif // TOOLBAR_EXPORTGROUP_H

