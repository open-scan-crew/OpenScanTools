#include "gui/toolBars/MainToolBar.h"

#include "controller/controls/ControlApplication.h"
#include "controller/controls/ControlProject.h"

#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/Dialog/DialogRecentProjects.h"
#include "gui/Texts.hpp"
#include "gui/texts/AboutTexts.hpp"

#include <QtWidgets/qboxlayout.h>
#include <QtGui/qevent.h>

MainToolBar::MainToolBar(IDataDispatcher& dataDispatcher, float guiScale)
	: QToolBar()
	, m_dataDispatcher(dataDispatcher)
{
    //-------------------------- Design -------------------------------

    // Watermark
    m_logo_tagline = new QPushButton(this);
    m_logo_tagline->setIconSize(QSize(117 * guiScale, 24 * guiScale));
    m_logo_tagline->setIcon(QIcon(":/resources/images/OpenScanTools-logo-tagline.png"));
    m_logo_tagline->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_logo_tagline->setFlat(true);
    addWidget(m_logo_tagline);

    // Left Buttons
	m_newAction = addAction(QIcon::fromTheme("new_project"), TEXT_MAIN_TOOLBAR_NEW);
    m_openAction = addAction(QIcon::fromTheme("open_project"), TEXT_MAIN_TOOLBAR_OPEN);
	m_recentProjects = addAction(QIcon::fromTheme("clock"), TEXT_MAIN_TOOLBAR_RECENT_PROJECTS);
	m_saveAction = addAction(QIcon::fromTheme("save"), TEXT_MAIN_TOOLBAR_SAVE);
    m_closeAction = addAction(QIcon::fromTheme("close_project"), TEXT_MAIN_TOOLBAR_CLOSE_PROJECT);

	addSeparator();

	m_importTls = addAction(QIcon::fromTheme("import_pointcloud"), TEXT_MAIN_TOOLBAR_IMPORT_TLS);

	addSeparator();

	UndoAction = addAction(QIcon::fromTheme("undo"), TEXT_MAIN_TOOLBAR_UNDO);
	RedoAction = addAction(QIcon::fromTheme("redo"), TEXT_MAIN_TOOLBAR_REDO);

    addSeparator();

	m_fullScreenAction = addAction(QIcon::fromTheme("full viewport"), TEXT_MAIN_TOOLBAR_FULL_SCREEN);


    // Center
    m_centralWidget = new QWidget(this);
    m_centralWidget->setObjectName(QString("Main Application Name"));
    m_centralWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    QHBoxLayout* layout = new QHBoxLayout(m_centralWidget);
    m_OSTInfo = " - OpenScanTools (";
    m_OSTInfo += TEXT_ABOUT_VERSION;
    m_OSTInfo += ")";

    m_mainLabel = new QLabel(m_OSTInfo, m_centralWidget);
    layout->addWidget(m_mainLabel, Qt::AlignHCenter);
    addWidget(m_centralWidget);

    // Right Buttons  
    m_authorName = new QLabel(m_centralWidget);
    addWidget(m_authorName);
    m_restoreDocks = addAction(QIcon::fromTheme("magnet-100"), TEXT_MAIN_TOOLBAR_RESTORE_DOCKS);
    m_settingsAction = addAction(QIcon::fromTheme("icons8-settings-32"), TEXT_MAIN_TOOLBAR_SETTINGS);
    m_shortcutsShowAction = addAction(QIcon(":/icons/ui/shortcuts.png"), TEXT_MAIN_TOOLBAR_SHORTCUTS);
    m_aboutAction = addAction(QIcon::fromTheme("question"), TEXT_MAIN_TOOLBAR_ABOUT);
    m_minimizeAction = addAction(QIcon::fromTheme("hide_list"), TEXT_MAIN_TOOLBAR_MINIMIZE_SCREEN);
    m_maximizeAction = addAction(QIcon::fromTheme("maximize"), TEXT_MAIN_TOOLBAR_MAXIMIZE_SCREEN);
	m_exitAction = addAction(QIcon::fromTheme("close_window"), TEXT_MAIN_TOOLBAR_EXIT);

	//----------------------- Style ----------------------------------

    //setToolButtonStyle(Qt::ToolButtonIconOnly);
    setMovable(false);

    m_saveAction->setDisabled(true);
    m_closeAction->setDisabled(true);
    UndoAction->setDisabled(true);
    RedoAction->setDisabled(true);
    m_importTls->setDisabled(true);
    m_fullScreenAction->setDisabled(true);
    
    //------------------ Function connect ---------------------

    QObject::connect(m_newAction, &QAction::triggered, [this]() { m_dataDispatcher.sendControl(new control::project::SaveCreate()); });
    QObject::connect(m_openAction, &QAction::triggered, [this]() { m_dataDispatcher.sendControl(new control::project::SaveCloseLoad()); });
	QObject::connect(m_recentProjects, &QAction::triggered, this, &MainToolBar::slotOpenRecentSave);
    QObject::connect(m_saveAction, &QAction::triggered, [this]() { m_dataDispatcher.sendControl(new control::project::StartSave()); });
    QObject::connect(m_closeAction, &QAction::triggered, [this]() { m_dataDispatcher.sendControl(new control::project::SaveClose()); });
    QObject::connect(m_exitAction, &QAction::triggered, [this]() { m_dataDispatcher.sendControl(new control::project::SaveQuit()); });

    QObject::connect(m_importTls, &QAction::triggered, this, [this]() { m_dataDispatcher.sendControl(new control::project::FunctionImportScan()); });

    QObject::connect(UndoAction, &QAction::triggered, this, [this]() { m_dataDispatcher.sendControl(new control::application::Undo()); });
    QObject::connect(RedoAction, &QAction::triggered, this, [this]() { m_dataDispatcher.sendControl(new control::application::Redo()); });

    connect(m_aboutAction, &QAction::triggered, [this]() { emit this->showAbout(); });
    connect(m_settingsAction, &QAction::triggered, [this]() { emit this->showSettings(); });
    connect(m_shortcutsShowAction, &QAction::triggered, [this]() { emit this->showShortcuts(); });
	connect(m_fullScreenAction, &QAction::triggered, [this]() { emit this->fullScreenPressed(); });
    //connect(m_maximizeScreenAction, &QAction::triggered, [this]() { emit this->maximizeViewportPressed(); });
    connect(m_maximizeAction, &QAction::triggered, [this]() { emit this->maximizeScreenPressed(); });
    connect(m_minimizeAction, &QAction::triggered, [this]() { emit this->minimizeScreenPressed(); });
    connect(m_restoreDocks, &QAction::triggered, [this]() { emit this->restoreDocks(); });

    //------------------ Shortcuts -----------------------------

    m_saveAction->setShortcut(QKeySequence("Ctrl+S"));
    m_exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    m_importTls->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));

    UndoAction->setShortcut(QKeySequence("Ctrl+Z"));
    RedoAction->setShortcut(QKeySequence("Ctrl+Y"));

    //------------------ Data Dispatcher ----------------

	m_dataDispatcher.registerObserverOnKey(this, guiDType::undoRedoData);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::authorSelection);

    methods.insert(std::pair<guiDType, MainToolBarMethod>(guiDType::undoRedoData, &MainToolBar::undoRedoUpdate));
    methods.insert(std::pair<guiDType, MainToolBarMethod>(guiDType::projectLoaded, &MainToolBar::onProjectLoad));
    methods.insert(std::pair<guiDType, MainToolBarMethod>(guiDType::authorSelection, &MainToolBar::onAuthorSelection));

}

MainToolBar::~MainToolBar()
{
    m_dataDispatcher.unregisterObserver(this);
}

void MainToolBar::changeTitle(const QString& title)
{
    m_OSTInfo = " - OpenScanTools (";
    m_OSTInfo += title;
    m_OSTInfo += ")";
    m_mainLabel->setText(m_OSTInfo);
}

void MainToolBar::setFreeViewerMode()
{
    m_importTls->setEnabled(false);
    m_importTls->setVisible(false);
    m_newAction->setEnabled(false);
    m_newAction->setVisible(false);
}

void MainToolBar::changeEvent(QEvent* event)
{
    //fixme POC dynamic langage
   // switch (event->type())
   // {
   //         // this event is send if a translator is loaded
   //     case QEvent::LanguageChange:
   //         m_newAction->setToolTip(TEXT_MAIN_TOOLBAR_NEW);
   //         m_openAction->setToolTip(TEXT_MAIN_TOOLBAR_NEW);
   //         m_saveAction->setToolTip(TEXT_MAIN_TOOLBAR_NEW);
   //         m_closeAction->setToolTip(TEXT_MAIN_TOOLBAR_NEW);
   //         m_exitAction->setToolTip(TEXT_MAIN_TOOLBAR_NEW);
   //         break;
   // }
}

void MainToolBar::informData(IGuiData *data)
{
    if (methods.find(data->getType()) != methods.end())
    {
        MainToolBarMethod method = methods.at(data->getType());
        (this->*method)(data);
    }
}

void MainToolBar::undoRedoUpdate(IGuiData * data)
{
	GuiDataUndoRedoAble *URAdata = static_cast<GuiDataUndoRedoAble*>(data);

	UndoAction->setEnabled(URAdata->_undoAble);
	RedoAction->setEnabled(URAdata->_redoAble);
}

void MainToolBar::onProjectLoad(IGuiData * data)
{
	GuiDataProjectLoaded *PLdata = static_cast<GuiDataProjectLoaded*>(data);

	if (PLdata->m_isProjectLoad == false)
	{
		m_closeAction->setEnabled(false);
		m_importTls->setEnabled(false);
        m_saveAction->setEnabled(false);
        m_fullScreenAction->setEnabled(false);

        m_mainLabel->setText(m_OSTInfo);
	}
	else
	{
		m_closeAction->setEnabled(true);
		m_importTls->setEnabled(true);
        m_saveAction->setEnabled(true);
        m_fullScreenAction->setEnabled(true);
        m_mainLabel->setText(QString::fromStdWString(PLdata->m_projectName) + m_OSTInfo);
	}
}

void MainToolBar::onAuthorSelection(IGuiData* data)
{
    GuiDataAuthorNameSelection* author = static_cast<GuiDataAuthorNameSelection*>(data);
    m_authorName->setText(QString::fromStdWString(author->m_author));
}

void MainToolBar::mouseDoubleClickEvent(QMouseEvent *_event)
{
    if (m_maximizeAction->isVisible())
        emit maximizeScreenPressed();
}

void MainToolBar::slotWindowResized(bool maximized, bool fullScreen)
{
    m_minimizeAction->setVisible(fullScreen);
    m_maximizeAction->setVisible(fullScreen);
    m_exitAction->setVisible(fullScreen);
    if (maximized)
    {
        // change the text & icon accordingly
        m_maximizeAction->setText(TEXT_MAIN_TOOLBAR_RESTORE_SCREEN);
        m_maximizeAction->setIcon(QIcon::fromTheme("restore"));
    }
    else
    {
        // change the text & icon accordingly
        m_maximizeAction->setText(TEXT_MAIN_TOOLBAR_MINIMIZE_SCREEN);
        m_maximizeAction->setIcon(QIcon::fromTheme("maximize"));
    }
}

void MainToolBar::slotOpenRecentSave()
{
	DialogRecentProjects *dialog = new DialogRecentProjects(m_dataDispatcher, this->parentWidget());
	m_dataDispatcher.sendControl(new control::application::SendRecentProjects());
	dialog->show();
}