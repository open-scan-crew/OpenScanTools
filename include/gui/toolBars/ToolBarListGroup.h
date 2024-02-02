#ifndef TOOLBAR_LISTGROUP_H
#define TOOLBAR_LISTGROUP_H

#include <QtWidgets/QWidget>
#include "ui_toolbar_listGroup.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

class ToolBarListGroup : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarListGroup(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale);

	void informData(IGuiData *data) override;

private:
    ~ToolBarListGroup();
	void onProjectLoad(IGuiData* data);

public slots:
	void manageLists();
	void manageTemplate();

private:
	Ui::ToolBarListGroup m_ui;
    IDataDispatcher &m_dataDispatcher;
	QString m_openPath;
};

#endif // TOOLBAR_LISTGROUP_H

