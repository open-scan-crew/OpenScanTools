#ifndef TOOLBAR_IMPORTOBJECTS_H
#define TOOLBAR_IMPORTOBJECTS_H

#include <QtWidgets/qwidget.h>
#include "ui_toolbar_importObjects.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

class ToolBarImportObjects : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarImportObjects(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale);

	void informData(IGuiData *data) override;

private:
    ~ToolBarImportObjects();
	void onProjectLoad(IGuiData* data);

public slots:
	void slotImportObjects();
	void slotLinkObjects();

private:
	Ui::ToolBarImportObjects m_ui;
    IDataDispatcher &m_dataDispatcher;
	QString m_openPath;
};

#endif // TOOLBAR_IMPORTOBJECTS_H

