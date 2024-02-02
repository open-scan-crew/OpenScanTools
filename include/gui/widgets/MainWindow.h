
#include "gui/widgets/AMenu.h"
#include "gui/widgets/VulkanWindow.h"
#include "models/project/ScanProject.h"

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QDockWidget>

#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

class MainWindow : public AMenu, QMainWindow
{   
    Q_OBJECT

public:

    MainWindow(Gui *gui, uint id);
	~MainWindow();

	void updateTree(ScanProject *project);
	void informData(std::pair<std::string, std::vector<std::any>> keyValue) override;
	void initialize() override;

public slots:

    void slotQuit();
	void slotChangeText();

private:
    QLabel* m_showCount;
    QLabel* hello;
    QPushButton* buttonQuit;
	QPushButton* buttonChangeText;

    QDockWidget* m_topDock;
    QWidget* m_topWidget;

    VulkanWindow* m_vkWindow;
};

#endif // _MAIN_WINDOW_H_
