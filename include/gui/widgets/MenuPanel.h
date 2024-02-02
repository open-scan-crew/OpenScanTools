#ifndef _PROJECT_PANEL_H_
#define _PROJECT_PANEL_H_

#include "gui/widgets/APanel.h"
#include "gui/GuiStates.h"

#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMenu>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

class MenuPanel : public QMenuBar, public APanel
{
    Q_OBJECT

public:

    MenuPanel(Gui *gui);
    ~MenuPanel();

    void informData(std::pair<uiDataKey, IGuiData*> keyValue) override;

private:/*
	void receiveProjectConfirm(ScanProject *project);
	void receiveProjectSave();*/

public slots:
    void slotQuit();
	void slotNew();
	void slotOpen();
	void slotSave();

	void slotImportTls();

    void onViewChanged(ViewState viewState);

private:
	std::string projectPath = "";
	bool projectLoaded = false;

	QMenu *m_fileMenu;
    QAction *m_newAction;
    QAction *m_openAction;
    QAction *m_saveAction;
    QAction *m_quitAction;

    QMenu *m_viewMenu;
    QAction *m_overlayView;
    QAction *m_projectDetailsView;
    QAction *m_tagEditView;

    QMenu *m_projectMenu;
	QAction *m_importTls;


};

#endif // _PROJECT_PANEL_H_
