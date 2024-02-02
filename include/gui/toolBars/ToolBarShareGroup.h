#ifndef TOOLBAR_SHAREGROUP_H
#define TOOLBAR_SHAREGROUP_H

#include <QtWidgets/QWidget>
#include "ui_toolbar_sharegroup.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

#include "gui/Dialog/DialogExportPrimitives.h"
#include "gui/Dialog/DialogExportSubProject.h"

class ToolBarShareGroup;

typedef void (ToolBarShareGroup::* ShareGroupMethod)(IGuiData*);

class ToolBarShareGroup : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarShareGroup(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale);

	void informData(IGuiData *keyValue) override;

private:
    ~ToolBarShareGroup();
	void onProjectLoad(IGuiData *data);

public slots:
	void slotExportToShareObjects();
	void slotExportToSubProject();

private:
	std::unordered_map<guiDType, ShareGroupMethod> m_methods;
    IDataDispatcher &m_dataDispatcher;
	Ui::ToolBarShareGroup m_ui;
	QString m_openPath;

	DialogExportPrimitives* m_primitivesExportDialog;
	DialogExportSubProject* m_subProjectExportDialog;
};

#endif // TOOLBAR_EXPORTGROUP_H

