#ifndef TOOLBAR_EXPORTVIDEO_H
#define TOOLBAR_EXPORTVIDEO_H

#include <QtWidgets/qwidget.h>
#include "ui_toolbar_exportVideo.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

#include "gui/Dialog/DialogExportVideo.h"

class ToolBarExportVideo : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarExportVideo(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale);

	void informData(IGuiData *data) override;

private:
    ~ToolBarExportVideo();
	void onProjectLoad(IGuiData* data);

public slots:
	void generateVideo();

private:
	Ui::ToolBarExportVideo m_ui;
    IDataDispatcher& m_dataDispatcher;
	DialogExportVideo* m_dialog;
};

#endif // TOOLBAR_LISTGROUP_H

