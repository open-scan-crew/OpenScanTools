#ifndef GUI_H
#define GUI_H

#include <stdlib.h>
#include <string>
#include <iostream>
#include <map>
#include <unordered_set>
#include <mutex>

#include "gui/IDataDispatcher.h"
#include "gui/IPanel.h"

#include "gui/ribbon/ribbon.h"
#include "gui/Dialog/DialogSettings.h"
#include "gui/Dialog/DialogAbout.h"
#include "gui/Dialog/DialogShortcuts.h"
#include "gui/Dialog/DialogDeleteScanTypeSelect.h"
#include "gui/Dialog/MessageSplashScreen.h"
#include "gui/Dialog/DialogImportAsciiPC.h"
#include "gui/Dialog/DialogProjectCreation.h"
#include "gui/Dialog/DialogImportMeshObject.h"
#include "gui/Dialog/DialogImportFileObject.h"
#include "gui/Dialog/DialogOpenProjectCentral.h"
#include "gui/Dialog/ProjectTemplateListDialog.h"
#include "gui/widgets/PropertyClippingSettings.h"
#include "gui/widgets/SplashScreen.h"

#include "controller/controls/IControl.h"

#include "pointCloudEngine/IRenderingEngine.h"

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QStatusBar>
#include <QGuiApplication>

#define GUILOG Logger::log(LoggerMode::GuiLog)

class StatusPanel;
class ViewportOrganizer;
class MainToolBar;
class SplashScreen;
class Translator;
class LicenseWindowsManager;
class ToolBarProjectGroup;
class Controller;
class APropertyGeneral;

/*! \class Gui
 * \brief Class that controll all the interaction and all the definition of the UI
 *
 * the job of this class is double. first, draw all the UI elements, like the images, the buttons
 * the project tree ect. the second job of this class is to detect and send the "events/actions"
 * to the controller.
 */
class Gui : public QMainWindow, IPanel
{
    Q_OBJECT

public:
	Gui(Controller& controller, Translator* translator);
	~Gui();

    // from Gui
    void launch();

    // from IPanel
	void informData(IGuiData *keyValue) override;
	void onEditing(const bool& isEditing);

    void initGeneralSettings();
	void initRegisterGuiDataDoubleEdit();
	void initDockWidgetsPosition();

	void showAuthorManager();

private slots:
    void toggleMaximized();
    //void toggleFullScreen();
	void showSettings();
	void showShortcuts();
	void showAbout();
	void onRestoreDocks();
	void onRescale(qreal dpi);
	void onScreenChanged(QScreen* screen);
    void onPropertyPanelName(QString name);

signals:
	void sigIsEditing(const bool& isEditing);
	void sigDoNotFocusOut();

private:
    bool event(QEvent* event) override;
    void closeEvent(QCloseEvent *event) override;
	void keyPressEvent(QKeyEvent* _event) override;
	void keyReleaseEvent(QKeyEvent* _event) override;
	void changeEvent(QEvent* event) override;

    typedef void (Gui::*GuiMethod)(IGuiData*);
    inline void registerGuiDataFunction(guiDType type, GuiMethod fct)
    {
        m_dataDispatcher.registerObserverOnKey(this, type);
        m_methods.insert({ type, fct });
    };

	void showWarningMBox(IGuiData *data);
	void showInfoMBox(IGuiData *data);
	void showModalData(IGuiData *data);
	void onProjectLoaded(IGuiData *data);
	void onHidePropertyPanels(IGuiData *data);
	void onQuitEvent(IGuiData *data);
	void onNewProject(IGuiData *data);
	void onOpenProject(IGuiData *data);

	//new
	//void onOpenProjectCentral(IGuiData* data);

	void onImportScans(IGuiData *data);
	void onSplashScreenStart(IGuiData* data);
	void onSplashScreenEnd(IGuiData* data);
	void onProjectPath(IGuiData *data);
	void onDeleteScanDialog(IGuiData* data);
	void onImportFileObject(IGuiData* data);
	void onExportFileObject(IGuiData* data);
	void onImportMeshObject(IGuiData* data);
	void onProjectTemplateDialog(IGuiData* data);
	void onObject3DPropertySettings(IGuiData* data);
	void onOpenInExplorer(IGuiData* data);
	void onOpenCentralProject(IGuiData* data);

	void removeGroupInTab(const QString& tabName, const QString& widgetName);
	void disableGroupInTab(const QString& tabName, const QString& widgetName);

private:
    IDataDispatcher& m_dataDispatcher;

	bool m_isSaved = true;
	bool m_isFullScreen;
    bool m_maximizedFrameless;
	bool m_isActivePopup;
    float m_guiScale = 1.f;
	bool m_isEditing;

	std::unordered_map<guiDType, GuiMethod> m_methods;
	std::unordered_map<guiDType, QWidget*> m_properties;
	std::unordered_map<ElementType, APropertyGeneral*> m_objectProperties;
	std::unordered_set<QDialog*> m_dialogs;
	APropertyGeneral* m_lastShowProperties = nullptr;

	QString m_openPath;

    MainToolBar* m_mainToolBar;
	ToolBarProjectGroup* m_projectGroup;
	QDockWidget* m_workspaceDock;
	QDockWidget* m_propertiesDock;
	QDockWidget* m_tagMenu;
	QDockWidget* m_FunctionsBarDock;
	Ribbon	*m_ribbon;

    StatusPanel* m_statusPanel;
	ViewportOrganizer* m_centralWrapper;
	Translator* m_translator;
	DialogSettings m_dSettings;
	DialogShortcuts m_dShortcuts;
	DialogAbout m_dAbout;
	MessageSplashScreen m_messageScreen;
	SplashScreen m_splashScreen;
	DialogImportMeshObject m_importMeshObject;
	DialogImportFileObject m_importFileObject;
	DialogOpenProjectCentral m_openCentralProject;
	ProjectTemplateListDialog m_projectTemplatesDialog;
	PropertyClippingSettings m_object3DPropertySettings;

    IRenderingEngine* m_renderingEngine;
};

#endif // !_GUI_H_
