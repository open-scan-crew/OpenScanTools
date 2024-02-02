#ifndef MAIN_TOOL_BAR_H_
#define MAIN_TOOL_BAR_H_

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "models/project/ProjectInfos.h"

#include <QtWidgets/QToolBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>

class MainToolBar;
typedef void (MainToolBar::* MainToolBarMethod)(IGuiData*);

class MainToolBar : public QToolBar, public IPanel
{
	Q_OBJECT

public:
    MainToolBar(IDataDispatcher& dataDispatcher, float guiScale);
    ~MainToolBar();

	void informData(IGuiData *keyValue) override;

	void undoRedoUpdate(IGuiData *data);
	void onProjectLoad(IGuiData *data);
    void onAuthorSelection(IGuiData* data);

    void mouseDoubleClickEvent(QMouseEvent *) override;
    //fixme POC dynamic langage
    void changeEvent(QEvent* event) override;

    void changeTitle(const QString& title);
    void setFreeViewerMode();

public slots:
    void slotWindowResized(bool maximized, bool fullScreen);
	void slotOpenRecentSave();

signals:
    void fullScreenPressed();
    void maximizeScreenPressed();
    void minimizeScreenPressed();
    void showSettings();
    void showShortcuts();
    void showAbout();
    void resetLicense();
    void restoreDocks();
    void abort();

private:
    IDataDispatcher& m_dataDispatcher;
    std::unordered_map<guiDType, MainToolBarMethod> methods;

    // Left side buttons
    QPushButton *m_logo_tagline;
	QAction *m_newAction;
	QAction *m_openAction;
	QAction *m_recentProjects;
	QAction *m_saveAction;
    QAction *m_closeAction;
	QAction *m_fullScreenAction;
    QAction* m_restoreDocks;

	QAction *m_importTls;

	QAction *UndoAction;
	QAction *RedoAction;

    // Center
    QWidget* m_centralWidget;
    QLabel* m_mainLabel;
    QString m_OSTInfo;

    // Right side buttons
    QLabel* m_authorName;
    QAction* m_resetingLicenseAction;
    QAction* m_settingsAction;
    QAction* m_shortcutsShowAction;
    QAction* m_aboutAction;
    QAction *m_minimizeAction;
    QAction *m_maximizeAction;
    QAction *m_exitAction;
};

#endif