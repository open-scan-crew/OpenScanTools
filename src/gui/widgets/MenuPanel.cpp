#include "gui/widgets/MenuPanel.h"
#include "gui/gui.h"
#include "io/SaveLoadSystem.h"
#include "gui/widgets/NewProjectDialog.h"

#include "controls/ProjectControls.h"

#include <QtWidgets/QShortCut>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QFileDialog>

MenuPanel::MenuPanel(Gui *gui) : APanel(gui), QMenuBar()
{
    //************
    // File
    //************
	m_fileMenu = new QMenu(tr("&File"), this);
	m_newAction = m_fileMenu->addAction(tr("New"));
	m_openAction = m_fileMenu->addAction(tr("Open"));
	m_saveAction = m_fileMenu->addAction(tr("Save"));
    m_saveAction->setShortcut(QKeySequence("Ctrl+S"));
    m_fileMenu->addSeparator();
	m_quitAction = m_fileMenu->addAction(tr("Quit"));
    m_quitAction->setShortcut(QKeySequence("Ctrl+Q"));
	this->addMenu(m_fileMenu);

	QObject::connect(m_newAction, &QAction::triggered, this, &MenuPanel::slotNew);
	QObject::connect(m_openAction, &QAction::triggered, this, &MenuPanel::slotOpen);
	QObject::connect(m_saveAction, &QAction::triggered, this, &MenuPanel::slotSave);
	QObject::connect(m_quitAction, &QAction::triggered, this, &MenuPanel::slotQuit);
    
    //************
    // View
    //************
    m_viewMenu = new QMenu(tr("&View"), this);
    this->addMenu(m_viewMenu);

    m_overlayView = m_viewMenu->addAction(tr("Overlay"));
    m_projectDetailsView = m_viewMenu->addAction(tr("Project Details"));
    m_viewMenu->addSeparator();
    m_tagEditView = m_viewMenu->addAction(tr("Tag Edit"));

    m_overlayView->setCheckable(true);
    m_projectDetailsView->setCheckable(true);
    m_tagEditView->setCheckable(true);

    QObject::connect(m_overlayView, &QAction::triggered, _gui, &Gui::onToggleOverlay);
    QObject::connect(m_projectDetailsView, &QAction::triggered, _gui, &Gui::onToggleProjectDetails);
    QObject::connect(m_tagEditView, &QAction::triggered, _gui, &Gui::onToggleTagEdit);
    QObject::connect(_gui, &Gui::viewChanged, this, &MenuPanel::onViewChanged);

    //************
    // Project
    //************
    m_projectMenu = new QMenu(tr("&Project"), this);
    m_importTls = m_projectMenu->addAction("Import tls");

    m_importTls->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
    this->addMenu(m_projectMenu);

    QObject::connect(m_importTls, &QAction::triggered, this, &MenuPanel::slotImportTls);

}


MenuPanel::~MenuPanel()
{
    std::cout << "Destroying Menu panel..." << std::endl;
}

void MenuPanel::informData(std::pair<uiDataKey, IGuiData*> keyValue)
{

}

void MenuPanel::slotNew()
{
    NewProjectDialog* projectDialog = new NewProjectDialog(this, _gui);
}

void MenuPanel::slotOpen()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"), QString::fromStdString(getDocumentPath()), tr("Project File (*.SCS)"));
	_gui->sendControl(new ControlLoad(fileName.toStdString()));
}

void MenuPanel::slotSave()
{
	_gui->sendControl(new ControlSave());
}

void MenuPanel::slotQuit()
{
	_gui->sendControl(new ControlQuitApplication());
}

void MenuPanel::slotImportTls()
{
	QStringList paths = QFileDialog::getOpenFileNames(this, tr("Import Scans"), QString::fromStdString(getDocumentPath()), tr("TagLabs Laser Scan (*.tls)"));
	std::vector<std::string> filePaths;

	for (QStringList::iterator it = paths.begin(); it != paths.end(); it++)
		filePaths.push_back((*it).toStdString());
	_gui->sendControl(new ControlImportScan(filePaths));
}

void MenuPanel::onViewChanged(ViewState viewState)
{
    m_overlayView->setChecked(viewState.overlayVisible);
    m_projectDetailsView->setChecked(viewState.projectDetailsVisible);
    m_tagEditView->setChecked(viewState.tagEditVisible);
}