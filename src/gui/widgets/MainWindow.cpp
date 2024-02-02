#include "gui/widgets/MainWindow.h"
#include "controls/ProjectControls.h"
#include "gui/Gui.h"

#include "gui/widgets/VulkanWindow.h"

#include <iostream>

#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QApplication>

MainWindow::MainWindow(Gui *gui, uint id) : AMenu(gui, id)
{
    hello = new QLabel("Hello World!", (QMainWindow*)this);
	hello->setFixedSize(QSize(300, 100));
	buttonQuit = new QPushButton("Quit", this);
	buttonChangeText = new QPushButton("Load project add +1", this);

    QObject::connect(buttonQuit, &QPushButton::clicked, this, &MainWindow::slotQuit);
	QObject::connect(buttonChangeText, &QPushButton::clicked, this, &MainWindow::slotChangeText);

    m_showCount = new QLabel("--", this);
    m_showCount->setText(QString::number(17));

    m_topDock = new QDockWidget(this);
    m_topWidget = new QWidget(this);
    QGridLayout * layout = new QGridLayout(m_topWidget);

    layout->addWidget(hello, 0, 0);
    layout->addWidget(buttonQuit, 0, 10);
    layout->addWidget(m_showCount, 1, 1);
	layout->addWidget(buttonChangeText, 1, 0);

    m_topWidget->setLayout(layout);
    m_topDock->setWidget(m_topWidget);
    this->addDockWidget(Qt::TopDockWidgetArea, m_topDock);

    m_vkWindow = new VulkanWindow();

    QWidget* centralWrapper = QWidget::createWindowContainer(m_vkWindow);
    centralWrapper->setFocusPolicy(Qt::StrongFocus);

    this->setCentralWidget(centralWrapper);
    centralWrapper->setFocus();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initialize()
{
	_gui->registerMenuOnId(PanelId, "test");
}

void MainWindow::updateTree(ScanProject * project)
{
	if (project == NULL)
	{
		hello->setText(QString::fromStdString("Project not loaded"));
	}
	else
	{
		hello->setText(QString::fromStdString("Project loaded with name : " + project->getProjectName()));
	}
}

void MainWindow::slotChangeText()
{
	m_showCount->setText(QString::number(42));
	_gui->sendControl(new ControlLoad());
}

void MainWindow::slotQuit()
{
	_gui->sendControl(new ControlQuitApplication());
}

void MainWindow::informData(std::pair<std::string, std::vector<std::any>> keyValue)
{
	if (keyValue.first == "test")
	{
		std::cout << std::any_cast<int>(keyValue.second[0]) << std::endl;
		std::cout << std::any_cast<ScanProject*>(keyValue.second[1])->getAuthor() << std::endl;
	}
}