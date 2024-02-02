#include "gui/widgets/ProjectPanel.h"
#include "gui/gui.h"

#include "controls/ProjectControls.h"
#include "io/SaveLoadSystem.h"

#include <QtWidgets/QShortCut>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QFileDialog>

ProjectPanel::ProjectPanel(Gui *gui, uint id) : APanel(gui, id), QMenuBar()
{
	menu = new QMenu("File", this);
	quitShortcut = new QShortcut(QKeySequence("Ctrl+Cancel"), this);
	newAction = menu->addAction("New");
	openAction = menu->addAction("Open");
	saveAction = menu->addAction("Save");
	quitAction = menu->addAction("Quit");
	this->addMenu(menu);

	//saveAction->setDisabled(true);
	QObject::connect(newAction, &QAction::triggered, this, &ProjectPanel::slotNew);
	QObject::connect(openAction, &QAction::triggered, this, &ProjectPanel::slotOpen);
	QObject::connect(saveAction, &QAction::triggered, this, &ProjectPanel::slotSave);
	QObject::connect(quitAction, &QAction::triggered, this, &ProjectPanel::slotQuit);
	QObject::connect(quitShortcut, &QShortcut::activated, this, &ProjectPanel::slotQuit);

	_gui->registerMenuOnId(id, "projectLoad");
    initialize();
}


ProjectPanel::~ProjectPanel()
{
}

void ProjectPanel::informData(std::pair<std::string, std::vector<std::any>> keyValue)
{
	std::cout << "informData " << keyValue.first << std::endl;
	if (keyValue.first == "projectLoad")
		receiveProjectConfirm(std::any_cast<ScanProject*>(keyValue.second[0]));
	else if (keyValue.first == "projectSave")
		receiveProjectSave();
}

void ProjectPanel::receiveProjectConfirm(ScanProject *project)
{
	std::cout << "receive" << std::endl;
	if (project != NULL)
	{
		projectLoaded = true;
		projectPath = project->getFilePath();
	}
	else
	{
		std::cout << "no project" << std::endl;
	}
	//FIXME problème thread
	//saveAction->setDisabled(true);
	std::cout << "receiveok" << std::endl;
}

void ProjectPanel::receiveProjectSave()
{

}

void ProjectPanel::initialize()
{
}

QWidget * ProjectPanel::getQWidget()
{
	return (this);
}

void ProjectPanel::slotNew()
{
	std::cout << "new" << std::endl;
	_gui->sendControl(new ControlNewProject());
}

void ProjectPanel::slotOpen()
{
	std::cout << "open" << std::endl;
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"), QString::fromStdString(getDocumentPath()), tr("Project File (*.SCS)"));
	_gui->sendControl(new ControlLoad(fileName.toStdString()));
}

void ProjectPanel::slotSave()
{
	if (projectLoaded == false)
		return;
	std::cout << "save" << std::endl;
	QString fileName = "";
	if (projectPath == "")
		fileName = QFileDialog::getSaveFileName(this, tr("New Project"), QString::fromStdString(getDocumentPath()), tr("Project Folder"));
	else
		fileName = QString::fromStdString(projectPath);
	_gui->sendControl(new ControlSave(fileName.toStdString()));
}

void ProjectPanel::slotQuit()
{
	_gui->sendControl(new ControlQuitApplication());
}
