#include "gui/Gui.h"
#include "gui/Dialog/DialogProjectCreation.h"
#include "gui/Dialog/DialogImportAsciiPC.h"
#include "gui/Dialog/DialogDeleteScanTypeSelect.h"
#include "gui/Texts.hpp"
#include "gui/texts/MainGuiTexts.hpp"
#include "gui/texts/FileTypeTexts.hpp"
#include "gui/texts/TreePanelTexts.hpp"
#include "gui/widgets/StatusPanel.h"
#include "gui/widgets/ProjectTreePanel.h"
#include "gui/widgets/PropertiesProjectPanel.h"
#include "gui/widgets/PropertiesClusterPanel.h"
#include "gui/widgets/PropertyPointCloud.h"
#include "gui/widgets/PropertySimpleMeasure.h"
#include "gui/widgets/PropertyPolylineMeasure.h"
#include "gui/widgets/PropertyBox.h"
#include "gui/widgets/PropertyPipe.h"
#include "gui/widgets/PropertyElbow.h"
#include "gui/widgets/PropertySphere.h"
#include "gui/widgets/PropertyPoint.h"
#include "gui/widgets/PropertyMeshObject.h"
#include "gui/widgets/PropertyBeamBendingMeasure.h"
#include "gui/widgets/PropertyColumnTiltMeasure.h"
#include "gui/widgets/PropertyPointToPlaneMeasure.h"
#include "gui/widgets/PropertyPipeToPipeMeasure.h"
#include "gui/widgets/PropertyPointToPipeMeasure.h"
#include "gui/widgets/PropertyPipeToPlaneMeasure.h"
#include "gui/widgets/PropertyDuplicationSettings.h"
#include "gui/widgets/PropertyUserOrientation.h"
#include "gui/widgets/PropertyViewpoint.h"
#include "gui/widgets/MultiProperty.h"
#include "gui/widgets/QPFields/TemplatePropertiesPanel.h"

#include "gui/viewport/ViewportOrganizer.h"
#include "gui/viewport/EventManagerViewport.h"
#include "gui/toolBars/MainToolBar.h"
#include "gui/style/CustomQtThemes.h"

// List of ribbon menu contents
#include "gui/ribbon/ribbon.h"
#include "gui/toolBars/ToolBarAttributesGroup.h"
#include "gui/toolBars/ToolBarProjectGroup.h"
#include "gui/toolBars/ToolBarTagGroup.h"
#include "gui/toolBars/ToolBarTemplateGroup.h"
#include "gui/toolBars/ToolBarFilterGroup.h"
#include "gui/toolBars/ToolBarFindScan.h"
#include "gui/toolBars/ToolBarExportGroup.h"
#include "gui/toolBars/ToolBarShareGroup.h"
#include "gui/toolBars/ToolBarExportPointCloud.h"
#include "gui/toolBars/ToolBarListGroup.h"
#include "gui/toolBars/ToolBarImportObjects.h"
#include "gui/toolBars/ToolBarImportScantra.h"
#include "gui/toolBars/ToolBarImageGroup.h"
#include "gui/toolBars/ToolBarMeasureGroup.h"
#include "gui/toolBars/ToolBarMeasureSimple.h"
#include "gui/toolBars/ToolBarShowHideGroup.h"
#include "gui/toolBars/ToolBarRenderSettings.h"
#include "gui/toolBars/ToolBarRenderNormals.h"
#include "gui/toolBars/ToolBarRenderRampGroup.h"
#include "gui/toolBars/ToolBarRenderTransparency.h"
#include "gui/toolBars/ToolBarClippingGroup.h"
#include "gui/toolBars/ToolBarMeasureShowOptions.h"
#include "gui/toolBars/ToolBarStructureAnalysis.h"
#include "gui/toolBars/ToolBarOthersModelsGroup.h"
#include "gui/toolBars/ToolBarPointCloudObjectGroup.h"
#include "gui/toolBars/ToolBarMeshObjectGroup.h"
#include "gui/toolBars/ToolBarPipeGroup.h"
#include "gui/toolBars/ToolBarLucasExperimentGroup.h"
#include "gui/toolBars/ToolBarNavigationConstraint.h"
#include "gui/toolBars/ToolBarFOV.h"
#include "gui/toolBars/ToolBarClippingParameters.h"
#include "gui/toolBars/ToolBarRampDefaultValues.h"
#include "gui/toolBars/ToolBarTextDisplay.h"
#include "gui/toolBars/ToolBarUserOrientations.h"
#include "gui/toolBars/ToolBarMarkerDisplayOptions.h"
#include "gui/toolBars/ToolBarPointEdition.h"
#include "gui/toolBars/ToolBarSphereGroup.h"
#include "gui/toolBars/ToolBarConnectPipeGroup.h"
#include "gui/toolBars/ToolBarBeamDetection.h"
#include "gui/toolBars/ToolBarViewPoint.h"
#include "gui/toolBars/ToolBarSlabGroup.h"
#include "gui/toolBars/ToolBarOrthoGrid.h"
#include "gui/toolBars/ToolBarAutoSeeding.h"
#include "gui/toolBars/ToolBarExportVideo.h"
#include "gui/toolBars/ToolBarManipulateObjects.h"
#include "gui/toolBars/ToolBarConvertImage.h"

#include "gui/Dialog/DialogExportFileObject.h"
#include "gui/dialog/DialogExportPointCloud.h"
#include "gui/dialog/DialogDeletePoints.h"
#include "gui/dialog/DialogPointCloudObjectCreation.h"
#include "gui/Dialog/ProcessingSplashScreen.h"

#include "utils/Config.h"
#include "utils/Logger.h"
#include "utils/System.h"
#include "utils/OpenScanToolsVersion.h"
#include "gui/GuiData/IGuiData.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataIO.h"
#include "gui/GuiData/GuiDataClipping.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/ShortcutSystem.h"
#include "gui/ribbon/ribbontabcontent.h"
#include "controller/Controller.h"
#include "controller/controls/ControlProject.h"
#include "controller/controls/ControlApplication.h"
#include "controller/controls/ControlModal.h"
#include "controller/controls/ControlFunction.h"
#include "controller/controls/ControlIO.h"

#include "models/project/ProjectTypes.h"
#include "models/graph/CameraNode.h"

#include <QtWidgets/qwidget.h>
#include <QtWidgets/qdockwidget.h>
#include <QtWidgets/qfiledialog.h>
#include <QtCore/qstandardpaths.h>
#include <QtWidgets/qmessagebox.h>
#include <QtGui/qscreen.h>
#include <QtGui/qevent.h>
#include <QtCore/qprocess.h>
#include <QtCore/qsettings.h>

#include "gui/UrlHandler.h"
#include "qdesktopservices.h"

#ifndef PORTABLE
#include "gui/widgets/ConvertionOptionsBox.h"
#endif

#define MAINWINDOW_INI L"gui.ini"
//toberemoved

static FileUrlHandler* s_fileHandler = new FileUrlHandler();

Gui::Gui(Controller& controller)
    : QMainWindow(nullptr/*, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint*/)
    , m_dataDispatcher(controller.getDataDispatcher())
	, m_isActivePopup(false)
	, m_isFullScreen(false)
	, m_isEditing(false)
    , m_maximizedFrameless(true)
	, m_centralWrapper(nullptr)
	, m_dSettings(m_dataDispatcher, this)
	, m_dShortcuts(this)
	, m_dAbout(m_dataDispatcher, this)
	, m_messageScreen(this)
	, m_importFileObject(m_dataDispatcher, this)
	, m_projectTemplatesDialog(m_dataDispatcher, this)
	, m_object3DPropertySettings(m_dataDispatcher, this)
{
    QCoreApplication::setOrganizationName("OpenScanTools");
	QCoreApplication::setApplicationName("OpenScanTools");

	QList<QScreen*> screenList = QGuiApplication::screens();
	QPoint screenLocation = this->mapToGlobal({ width() / 2, 0 });
	QScreen* screen = QGuiApplication::screenAt(screenLocation);
    if (screen != nullptr)
    {
        m_guiScale = screen->logicalDotsPerInch() / 96.f;
        GUILOG << "Initialized the GUI with screen ratio: " << m_guiScale << Logger::endl;
    }

    scs::ApplyDarkTheme(m_guiScale);
	m_dAbout.setDialog(m_guiScale);
    m_mainToolBar = new MainToolBar(m_dataDispatcher, m_guiScale);
    this->addToolBar(Qt::TopToolBarArea, m_mainToolBar);

	m_object3DPropertySettings.hide();

    // Docks
	m_workspaceDock = new QDockWidget(TEXT_WORKSPACE, this);
	m_propertiesDock = new QDockWidget(TEXT_PROPERTIES, this);
    m_FunctionsBarDock = new QDockWidget(TEXT_TOOLBAR, this);
	m_propertiesDock->setObjectName(TEXT_PROPERTIES);
	m_propertiesDock->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
	m_FunctionsBarDock->setObjectName(TEXT_TOOLBAR);
	m_workspaceDock->setObjectName(TEXT_WORKSPACE);
	
    TemplatePropertiesPanel* panel = new TemplatePropertiesPanel(controller, this);
    connect(panel, &TemplatePropertiesPanel::nameChanged, this, &Gui::onPropertyPanelName, Qt::QueuedConnection);

	m_objectProperties.insert({ ElementType::Tag, panel });
	m_objectProperties.insert({ ElementType::Cluster, new PropertiesClusterPanel(controller, this) });
	PropertyPointCloud* pointCloudPropertyPanel = new PropertyPointCloud(controller, this);
	m_objectProperties.insert({ ElementType::Scan, pointCloudPropertyPanel });
	m_objectProperties.insert({ ElementType::PCO, pointCloudPropertyPanel });
	m_objectProperties.insert({ ElementType::SimpleMeasure , new PropertySimpleMeasure(controller, this) });
	m_objectProperties.insert({ ElementType::PolylineMeasure, new PropertyPolylineMeasure(controller, this) });
	m_objectProperties.insert({ ElementType::BeamBendingMeasure, new PropertyBeamBendingMeasure(controller, this) });
	m_objectProperties.insert({ ElementType::ColumnTiltMeasure, new PropertyColumnTiltMeasure(controller, this) });
	m_objectProperties.insert({ ElementType::PointToPlaneMeasure, new PropertyPointToPlanMeasure(controller, this) });
	m_objectProperties.insert({ ElementType::PipeToPipeMeasure, new PropertyPipeToPipeMeasure(controller, this) });
	m_objectProperties.insert({ ElementType::PointToPipeMeasure, new PropertyPointToPipeMeasure(controller, this) });
	m_objectProperties.insert({ ElementType::PipeToPlaneMeasure, new PropertyPipeToPlaneMeasure(controller, this) });

	m_objectProperties.insert({ ElementType::ViewPoint, new PropertyViewpoint(controller, this) });
	m_objectProperties.insert({ ElementType::MeshObject, new PropertyMeshObject(controller, this) });
	m_objectProperties.insert({ ElementType::Box, new PropertyBox(controller, this) });
	m_objectProperties.insert({ ElementType::Cylinder, new PropertyPipe(controller, this) });
	m_objectProperties.insert({ ElementType::Torus, new PropertyElbow(controller, this) });
	m_objectProperties.insert({ ElementType::Sphere, new PropertySphere(controller, this) });
	m_objectProperties.insert({ ElementType::Point, new PropertyPoint(controller, this) });

	m_dialogs.insert(new ProcessingSplashScreen(m_dataDispatcher, this) );
#ifndef PORTABLE
	m_dialogs.insert(new ConvertionOptionsBox(m_dataDispatcher, this));
#endif
	m_dialogs.insert(new DialogExportPointCloud(m_dataDispatcher, this) );
	m_dialogs.insert(new DialogDeletePoints(m_dataDispatcher, this) );
	m_dialogs.insert(new DialogPointCloudObjectCreation(m_dataDispatcher, this) );

	m_properties.insert({ guiDType::projectDataProperties, new PropertiesProjectPanel(m_dataDispatcher, this) });
	m_properties.insert({ guiDType::userOrientationProperties, new PropertyUserOrientation(m_dataDispatcher, this) });
	m_properties.insert({ guiDType::multiObjectProperties, new MultiProperty(controller, this) });
	

	for (QDialog* it : m_dialogs)
		it->close();

	for (std::pair<const guiDType, QWidget*>& it : m_properties)
		it.second->close();

	for (std::pair<const ElementType, APropertyGeneral*>& it : m_objectProperties)
		it.second->close();

	m_ribbon = new Ribbon();
	m_FunctionsBarDock->setWidget(m_ribbon);
	
	// Add groups to the Home Tab
	RibbonTabContent *ribbonTabContent = new RibbonTabContent();
	m_projectGroup = new ToolBarProjectGroup(m_dataDispatcher, this, m_guiScale);
	ribbonTabContent->addWidget(TEXT_PROJECT, m_projectGroup);
	ribbonTabContent->addWidget(TEXT_ATTRIBUTE, new ToolBarAttributesGroup(controller, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_TAG, new ToolBarTemplateGroup(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_TAG, new ToolBarTagGroup(m_dataDispatcher, this, m_guiScale));
	ToolBarMeasureSimple* measureSimple = new ToolBarMeasureSimple(m_dataDispatcher, this, m_guiScale);
	measureSimple->setPolyligneOptions(false);
	ribbonTabContent->addWidget(TEXT_MEASURE, measureSimple);
	ToolBarRenderSettings* rendering = new ToolBarRenderSettings(m_dataDispatcher, this, m_guiScale);
	ribbonTabContent->addWidget(TEXT_POINT_CLOUD, rendering);
	m_ribbon->addTab(TEXT_HOME, ribbonTabContent);
	rendering->switchRenderMode();

	// Add groups to the Renderings Tab
	ribbonTabContent = new RibbonTabContent();
	ribbonTabContent->addWidget(TEXT_NORMALS_OPTIONS, new ToolBarRenderNormals(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_TAB_TRANSPARENCY, new ToolBarRenderTransparency(m_dataDispatcher, this, m_guiScale));
	m_ribbon->addTab(TEXT_RENDERINGS, ribbonTabContent);


	// Add groups to the View Tab
	ribbonTabContent = new RibbonTabContent();
	ribbonTabContent->addWidget(TEXT_SHOW_HIDE, new ToolBarShowHideGroup(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_TEXT_DISPLAY, new ToolBarTextDisplay(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_MARKER_DISPLAY_OPTIONS, new ToolBarMarkerDisplayOptions(m_dataDispatcher, this, m_guiScale));	
	ribbonTabContent->addWidget(TEXT_ORTHO_GRID, new ToolBarOrthoGrid(m_dataDispatcher, this, m_guiScale));

	m_ribbon->addTab(TEXT_VIEW, ribbonTabContent);
	
//#363 Porposal Viewpoint
	// Add groups to the ViewPoint Tab
	ribbonTabContent = new RibbonTabContent();
	ribbonTabContent->addWidget(TEXT_ATTRIBUTE, new ToolBarAttributesGroup(controller, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_VIEWPOINT, new ToolBarViewPoint(m_dataDispatcher, this, m_guiScale));
	m_ribbon->addTab(TEXT_VIEWPOINT, ribbonTabContent);

	// Add groups to the Navigate Tab
	ribbonTabContent = new RibbonTabContent();
	ribbonTabContent->addWidget(TEXT_VIEWING_MODE, new ToolBarNavigationConstraint(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_VIEWING_MODE, new ToolBarFOV(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_ORIENTATIONS, new ToolBarUserOrientation(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_MANIPULATE, new ToolBarManipulateObjects(m_dataDispatcher, this, m_guiScale));
	m_ribbon->addTab(TEXT_NAVIGATE, ribbonTabContent);


	// Add groups to the Tag Tab
	ribbonTabContent = new RibbonTabContent();
	ribbonTabContent->addWidget(TEXT_ATTRIBUTE, new ToolBarAttributesGroup(controller, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_ATTRIBUTE, new ToolBarTemplateGroup(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_TAG, new ToolBarTagGroup(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_MANAGE, new ToolBarListGroup(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_OTHER, new ToolBarOthersModelsGroup(m_dataDispatcher, this, m_guiScale));
	m_ribbon->addTab(TEXT_TAG, ribbonTabContent);


	// Add groups to the Measure Tab
	ribbonTabContent = new RibbonTabContent();
	ToolBarAttributesGroup* attriMeasure = new ToolBarAttributesGroup(controller, this, m_guiScale);
	attriMeasure->hideColorPicker();
	ribbonTabContent->addWidget(TEXT_ATTRIBUTE, attriMeasure);
	ribbonTabContent->addWidget(TEXT_MEASURE_DISPLAY, new ToolBarMeasureShowOptions(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_SIMPLE_MEASURE, new ToolBarMeasureSimple(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_MEASURE, new ToolBarMeasureGroup(m_dataDispatcher, this, m_guiScale));
	//#ifdef _DEBUG
	ribbonTabContent->addWidget(TEXT_EXPERIMENT, new ToolBarLucasExperimentGroup(m_dataDispatcher, this, m_guiScale));
	//#endif // _DEBUG
	m_ribbon->addTab(TEXT_MEASURE, ribbonTabContent);

	// Add groups to the Analysis Tab
	ribbonTabContent = new RibbonTabContent();
	ribbonTabContent->addWidget(TEXT_RAMPS, new ToolBarRampDefaultValues(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_RAMPS, new ToolBarRenderRampGroup(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_STRUCTURE_ANALYSIS, new ToolBarStructureAnalysis(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_AUTO_SOWING, new ToolBarAutoSeeding(m_dataDispatcher, this, m_guiScale));
	m_ribbon->addTab(TEXT_ANALYSIS, ribbonTabContent);


	// Add groups to the Model Tab
	ribbonTabContent = new RibbonTabContent();
	ribbonTabContent->addWidget(TEXT_ATTRIBUTE, new ToolBarAttributesGroup(controller, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_PIPES, new ToolBarPipeGroup(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_CONNECTION, new ToolBarConnectPipeGroup(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_SLAB, new ToolBarSlabGroup(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_TAB_SPHERES, new ToolBarSphereGroup(m_dataDispatcher, this, m_guiScale));
	//ribbonTabContent->addWidget(TEXT_BEAM_DETECTION, new ToolBarBeamDetection(m_dataDispatcher, this, m_guiScale));
	m_ribbon->addTab(TEXT_MODEL, ribbonTabContent);

    // Add groups to the Clipping Tab
	ribbonTabContent = new RibbonTabContent();
	ribbonTabContent->addWidget(TEXT_ATTRIBUTE, new ToolBarAttributesGroup(controller, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_BOX, new ToolBarClippingGroup(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_CLIPPING_GROUP_NAME, new ToolBarClippingParameters(m_dataDispatcher, this, m_guiScale));
    ribbonTabContent->addWidget(TEXT_POINT_EDITION, new ToolBarPointEdition(m_dataDispatcher, this, m_guiScale));

	m_ribbon->addTab(TEXT_CLIPPING, ribbonTabContent);

	// Add groups to the Import (previously Point Cloud Object) Tab 
	ribbonTabContent = new RibbonTabContent();
	ribbonTabContent->addWidget(TEXT_ATTRIBUTE, new ToolBarAttributesGroup(controller, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_POINT_CLOUD_OBJECT, new ToolBarPointCloudObjectGroup(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_MESHOBJECT, new ToolBarMeshObjectGroup(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_OPENSCANTOOLS, new ToolBarImportObjects(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_SCANTRA, new ToolBarImportScantra(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_IMAGE, new ToolBarConvertImage(m_dataDispatcher, this, m_guiScale));
	m_ribbon->addTab(TEXT_IMPORT, ribbonTabContent);

	// Add groups to the Export Tab
	ribbonTabContent = new RibbonTabContent();
	ribbonTabContent->addWidget(TEXT_EXPORT, new ToolBarExportGroup(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_SHARE, new ToolBarShareGroup(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_POINT_CLOUD, new ToolBarExportPointCloud(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_IMAGE, new ToolBarImageGroup(m_dataDispatcher, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_VIDEO, new ToolBarExportVideo(m_dataDispatcher, this, m_guiScale));
	m_ribbon->addTab(TEXT_EXPORT, ribbonTabContent);

	// Add groups to the Filter Tab
	ribbonTabContent = new RibbonTabContent();
	ribbonTabContent->addWidget(TEXT_FILTER, new ToolBarFilterGroup(m_dataDispatcher, controller, this, m_guiScale));
	ribbonTabContent->addWidget(TEXT_FIND_SCAN, new ToolBarFindScan(m_dataDispatcher, this, m_guiScale));
	m_ribbon->addTab(TEXT_FILTER, ribbonTabContent);

	//m_ribbon->adjustSize();

	ShortcutSystem* shortSys = new ShortcutSystem(m_dataDispatcher, this);
	connect(this, &Gui::sigIsEditing, shortSys, &ShortcutSystem::slotEdition);

	m_workspaceDock->setWidget(new ProjectTreePanel(m_dataDispatcher, controller.getGraphManager(), m_guiScale));
	
    this->addDockWidget(Qt::TopDockWidgetArea, m_FunctionsBarDock);
	this->addDockWidget(Qt::LeftDockWidgetArea, m_workspaceDock);
	this->addDockWidget(Qt::LeftDockWidgetArea, m_propertiesDock);

    // Style
    // Possible DockOptions : AnimatedDocks | AllowTabbedDocks | AllowNestedDocks
    setDockOptions(AllowTabbedDocks);
    setDockNestingEnabled(false);  // NOTE(robin) - the nesting is enabled anyway (bug Qt ?)!
    // ---------- Functions Bar Dock ----------
    m_FunctionsBarDock->setTitleBarWidget(new QWidget());
    m_FunctionsBarDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    // ---------- Workspace Dock --------------
    m_workspaceDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);//setFeatures(QDockWidget::NoDockWidgetFeatures);
	m_workspaceDock->setAllowedAreas(Qt::DockWidgetArea::RightDockWidgetArea | Qt::DockWidgetArea::LeftDockWidgetArea);
    // ---------- Properties Dock -------------
    m_propertiesDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
	m_propertiesDock->setAllowedAreas(Qt::DockWidgetArea::RightDockWidgetArea | Qt::DockWidgetArea::LeftDockWidgetArea);

    // Rendering Engine - NOTE(robin) can be created in the main.cpp
    m_renderingEngine = scs::createRenderingEngine(controller.getGraphManager(), m_dataDispatcher, m_guiScale);

    // Central Viewport - Test multiple viewport
	m_centralWrapper = new ViewportOrganizer(this, m_dataDispatcher, *m_renderingEngine, *shortSys, m_guiScale);
    this->setCentralWidget(m_centralWrapper);
    m_centralWrapper->setFocus();
    m_centralWrapper->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    m_renderingEngine->start();

    // Status Bar
    m_statusPanel = new StatusPanel(m_dataDispatcher);
    this->setStatusBar(m_statusPanel);
    // TODO - Create a GuiData to pass the camera position and picking to the status bar
    connect(m_centralWrapper, &ViewportOrganizer::cameraPos, m_statusPanel, &StatusPanel::onCameraPos, Qt::QueuedConnection);
    connect(m_centralWrapper, &ViewportOrganizer::picking, m_statusPanel, &StatusPanel::onPicking, Qt::QueuedConnection);

    // Init the gui variables for the Toolbar and the Viewport
    m_dataDispatcher.updateInformation(new GuiDataMarkerDisplayOptions({ true, 30.0, 3.0, 20.0, 50.0, 12.0 }, SafePtr<CameraNode>()), this);

    registerGuiDataFunction(guiDType::popupMsgData, &Gui::showInfoMBox);
    registerGuiDataFunction(guiDType::popupWrnData, &Gui::showWarningMBox);
    registerGuiDataFunction(guiDType::popupModalData, &Gui::showModalData);
    registerGuiDataFunction(guiDType::hidePropertyPanels, &Gui::onHidePropertyPanels);
    registerGuiDataFunction(guiDType::projectLoaded, &Gui::onProjectLoaded);
    registerGuiDataFunction(guiDType::quitEvent, &Gui::onQuitEvent);
    registerGuiDataFunction(guiDType::newProject, &Gui::onNewProject);
	registerGuiDataFunction(guiDType::openProject, &Gui::onOpenProject);
    registerGuiDataFunction(guiDType::importScans, &Gui::onImportScans);
    registerGuiDataFunction(guiDType::splashScreenStart, &Gui::onSplashScreenStart);
    registerGuiDataFunction(guiDType::splashScreenEnd, &Gui::onSplashScreenEnd);
    registerGuiDataFunction(guiDType::projectPath, &Gui::onProjectPath);
	registerGuiDataFunction(guiDType::deleteScanDialog, &Gui::onDeleteScanDialog);
	registerGuiDataFunction(guiDType::exportFileObjectDialog, &Gui::onExportFileObject);
	registerGuiDataFunction(guiDType::importFileObjectDialog, &Gui::onImportFileObject);
	registerGuiDataFunction(guiDType::projectTemplateDialog, &Gui::onProjectTemplateDialog);
	registerGuiDataFunction(guiDType::clippingSettingsProperties, &Gui::onObject3DPropertySettings);
	registerGuiDataFunction(guiDType::openInExplorer, &Gui::onOpenInExplorer);
    // register all properties type
    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectDataProperties);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::clippingSettingsProperties);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::userOrientationProperties);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::objectSelected);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::multiObjectProperties);

	connect(m_mainToolBar, &MainToolBar::showAbout, this, &Gui::showAbout);
	connect(m_mainToolBar, &MainToolBar::showSettings, this, &Gui::showSettings);
	connect(m_mainToolBar, &MainToolBar::showShortcuts, this, &Gui::showShortcuts);
    connect(m_mainToolBar, &MainToolBar::maximizeScreenPressed, this, &Gui::toggleMaximized);
    connect(m_mainToolBar, &MainToolBar::minimizeScreenPressed, [this]() { this->showMinimized(); });
    connect(m_mainToolBar, &MainToolBar::fullScreenPressed, m_centralWrapper, &ViewportOrganizer::onEnableFullScreen);
	connect(m_mainToolBar, &MainToolBar::restoreDocks, this, &Gui::onRestoreDocks);

    // Resize before maximized for when we exit the full size
    this->resize(1200 * m_guiScale, 800 * m_guiScale);
    // FIX AQ_531 - On ne peut pas démmarer l'application en mode plein écran, ça reste un problème.
    //toggleMaximized();

	m_openPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory);
	m_messageScreen.hide();

    initGeneralSettings();
	initRegisterGuiDataDoubleEdit();
	initDockWidgetsPosition();

	m_propertiesDock->hide();
	QDesktopServices::setUrlHandler("file", s_fileHandler, "handleFile");

	GUILOG << "Init Gui done" << LOGENDL;
}

Gui::~Gui()
{
	GUILOG << "Destroy Gui...\n" << LOGENDL;
	std::wstring path = Utils::System::getOSTProgramDataPath().wstring() + L"\\" + MAINWINDOW_INI;
	QSettings settings(QString::fromStdWString(path), QSettings::Format::IniFormat, this);
	settings.setValue("geometry", saveGeometry());
	settings.setValue("windowState", saveState());

    m_dataDispatcher.unregisterObserver(this);

    delete m_renderingEngine;

	for (auto propertyPanel : m_properties)
		delete (propertyPanel.second);
	m_properties.clear();

}

void Gui::informData(IGuiData *data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		GuiMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}

    if (m_properties.find(data->getType()) != m_properties.end())
    {
		if (m_propertiesDock->widget() != m_properties.at(data->getType()))
		{
			QWidget* propertyElem = m_properties.at(data->getType());
			m_propertiesDock->setWidget(propertyElem);
			m_propertiesDock->setWindowTitle(propertyElem->windowTitle());
		}
        m_propertiesDock->show();
    }

	if (data->getType() == guiDType::objectSelected)
	{
		GuiDataObjectSelected* objProp = static_cast<GuiDataObjectSelected*>(data);
		if (objProp->m_object && m_objectProperties.find(objProp->m_type) != m_objectProperties.end())
		{
			{
				ReadPtr<AGraphNode> rObj = objProp->m_object.cget();
				if (!rObj || rObj->isDead())
					return;
			}

			m_lastShowProperties = m_objectProperties.at(objProp->m_type);
			m_lastShowProperties->actualizeProperty(objProp->m_object);
			m_propertiesDock->setWidget(m_lastShowProperties);
			m_propertiesDock->setWindowTitle(m_lastShowProperties->windowTitle());
			m_propertiesDock->show();
		}
		else
		{
			if (m_lastShowProperties && m_lastShowProperties->isVisible())
				if (!m_lastShowProperties->actualizeProperty(SafePtr<AGraphNode>()))
					m_propertiesDock->hide();
		}

	}
}

void Gui::launch()
{
	show();
	m_projectGroup->manageAuthors(false);
}

void Gui::showWarningMBox(IGuiData * data)
{
	if (m_isActivePopup == false)
	{
		m_isActivePopup = true;
		m_isActivePopup = QMessageBox::warning(nullptr, TEXT_WARNING, static_cast<GuiDataWarning*>(data)->m_message) == 0;
	}
}

void Gui::showInfoMBox(IGuiData * data)
{
	GuiDataInfo* info = static_cast<GuiDataInfo*>(data);
	
	if (info->m_isModal)
	{
		if (m_isActivePopup)
			return;
		m_isActivePopup = true;
		m_isActivePopup = QMessageBox::information(nullptr, TEXT_INFORMATION, info->m_message) == 0;
	}
	else
	{
		static QMessageBox* infoBox = new QMessageBox(QMessageBox::Icon::Information, TEXT_INFORMATION, QString(), QMessageBox::StandardButton::Ok, this);
		infoBox->setText(info->m_message);
		infoBox->setModal(false);
		infoBox->show();
	}
}

void Gui::showModalData(IGuiData * data)
{
	QMessageBox modal(QMessageBox::Icon::Question, TEXT_QUESTION, static_cast<GuiDataModal*>(data)->m_message, QFlags<QMessageBox::StandardButton>(static_cast<GuiDataModal*>(data)->m_flags));
	m_dataDispatcher.sendControl(new control::modal::ModalReturnValue(modal.exec()));
}

void Gui::onProjectLoaded(IGuiData * data)
{
    GuiDataProjectLoaded* pData = static_cast<GuiDataProjectLoaded*>(data);
    if (pData->m_isProjectLoad == false)
        m_propertiesDock->hide();
}

void Gui::onHidePropertyPanels(IGuiData * data)
{
	m_propertiesDock->hide();
	emit sigIsEditing(false);
}

void Gui::onQuitEvent(IGuiData * data)
{
	QCoreApplication::quit();
}

void Gui::onNewProject(IGuiData *data)
{
    auto* idata = static_cast<GuiDataNewProject*>(data);
    std::unordered_map<LanguageType, ProjectTemplate> projectTemplates;

    DialogProjectCreation* projectDialog = new DialogProjectCreation(m_dataDispatcher, this);
    projectDialog->setDefaultValues(idata->default_folder_, idata->default_name_, idata->default_company_);
    projectDialog->setAdditionalTemplatesPath(idata->templates_);
    projectDialog->show();
}

void Gui::onProjectTemplateDialog(IGuiData* data)
{
	m_projectTemplatesDialog.show();
}

void Gui::onObject3DPropertySettings(IGuiData* data)
{
	GuiDataClippingSettingsProperties* idata = static_cast<GuiDataClippingSettingsProperties*>(data);
	m_object3DPropertySettings.showHideCreationSettings(idata->m_onlyDuplication);
	m_propertiesDock->setWidget(&m_object3DPropertySettings);
	m_propertiesDock->setWindowTitle(idata->m_windowsName);
	m_propertiesDock->show();

}

void Gui::onOpenInExplorer(IGuiData* data)
{
	GuiDataOpenInExplorer* gd = static_cast<GuiDataOpenInExplorer*>(data);
	std::filesystem::path path = gd->m_path;
	if(!std::filesystem::is_directory(path))
		path = path.parent_path();


	QString qpath = QString::fromStdWString(path.wstring());

	QStringList args;

	args << "/root," << QDir::toNativeSeparators(qpath);

	QProcess* process = new QProcess(this);
	process->start("explorer.exe", args);
}

void Gui::onOpenProject(IGuiData *data)
{
	auto* idata = static_cast<GuiDataOpenProject*>(data);
    QString fileName = QFileDialog::getOpenFileName(this, TEXT_OPEN_PROJECT, QString::fromStdWString(idata->m_folder.wstring()), TEXT_FILE_TYPE_TLP, nullptr);

	GUILOG << "Open project : " << fileName.toStdString() << Logger::endl;
	if (fileName != "")
		m_dataDispatcher.sendControl(new control::modal::ModalReturnFiles({ fileName.toStdWString() }));
	else
		m_dataDispatcher.sendControl(new control::function::Validate());
}

void Gui::onImportScans(IGuiData *data)
{
    QStringList paths = QFileDialog::getOpenFileNames(this, TEXT_IMPORT_SCANS, m_openPath, TEXT_FILE_TYPE_ALL_SCANS_OPEN, nullptr);

	if (paths.empty())
		return;

	m_openPath = paths.back();

    std::vector<std::filesystem::path> filePaths;

    for (const QString& it : paths)
        filePaths.push_back(it.toStdWString());

	DialogImportAsciiPC importAsciiDialog(this);
	if(importAsciiDialog.setInfoAsciiPC(filePaths))
		importAsciiDialog.exec();

	std::map<std::filesystem::path, Import::AsciiInfo> mapAsciiInfo;
	if (importAsciiDialog.isFinished())
		mapAsciiInfo = importAsciiDialog.getFileImportInfo();

	Import::ScanInfo info;
	info.asObject = false;
	info.paths = filePaths;
	info.mapAsciiInfo = mapAsciiInfo;

    m_dataDispatcher.sendControl(new control::project::ImportScan(info));
}

void Gui::onSplashScreenStart(IGuiData* data)
{
	auto guiData = static_cast<GuiDataSplashScreenStart*>(data);
	switch (guiData->m_type)
	{
		case GuiDataSplashScreenStart::SplashScreenType::Display:
			m_splashScreen.showStatusMessage(guiData->m_text);
			m_splashScreen.show();
			break;
		case GuiDataSplashScreenStart::SplashScreenType::Message:
			m_messageScreen.setShowMessage(guiData->m_text);
			m_messageScreen.show();
			break;
	}
}

void Gui::onSplashScreenEnd(IGuiData* data)
{
	auto guiData = static_cast<GuiDataSplashScreenEnd*>(data);
	switch (guiData->m_type)
	{
	case GuiDataSplashScreenEnd::SplashScreenType::Display:
		m_splashScreen.hide();
		break;
	case GuiDataSplashScreenEnd::SplashScreenType::Message:
		m_messageScreen.hide();
		break;
	default:
		m_splashScreen.hide();
		m_messageScreen.hide();
	}
}

bool Gui::event(QEvent* _event)
{
    if (_event->type() == QEvent::WindowStateChange)
    {
        // Force fullScreenMode when maximized
        if (isMaximized() && !isFullScreen() && m_maximizedFrameless)
            setWindowState(windowState() | Qt::WindowFullScreen);
		bool forceMaximizeBtn(windowState() == Qt::WindowFullScreen);
		
        m_mainToolBar->slotWindowResized(forceMaximizeBtn | isMaximized(), isFullScreen());
		if (forceMaximizeBtn)
			toggleMaximized();
    }

    return QMainWindow::event(_event);
}

void Gui::closeEvent(QCloseEvent * event)
{
    m_dataDispatcher.sendControl(new control::project::SaveQuit());
    event->ignore();
}

void Gui::toggleMaximized()
{
    if (isMaximized())
        setWindowState(Qt::WindowNoState);
    else
    {
        if (m_maximizedFrameless)
            setWindowState(Qt::WindowMaximized | Qt::WindowFullScreen);
        else
            setWindowState(Qt::WindowMaximized);
    }
}

void Gui::keyPressEvent(QKeyEvent* _event)
{
	if (m_isEditing || _event->isAutoRepeat())
		return;

	if (_event->key() == Qt::Key_C)
	{
		ReadPtr<CameraNode> rCam = m_centralWrapper->getActiveCameraNode().cget();
		if(rCam)
			m_dataDispatcher.updateInformation(new GuiDataRenderTransparency(BlendMode::Transparent, rCam->getDisplayParameters().m_transparency, m_centralWrapper->getActiveCameraNode()), this);
	}
}

void Gui::keyReleaseEvent(QKeyEvent* _event)
{
	if (m_isEditing || _event->isAutoRepeat())
		return;
	if (_event->key() == Qt::Key_ScreenSaver)
		m_dataDispatcher.sendControl(new control::io::QuickScreenshot(ImageFormat::MAX_ENUM));

	if (_event->key() == Qt::Key_C)
	{
		ReadPtr<CameraNode> rCam = m_centralWrapper->getActiveCameraNode().cget();
		if (rCam)
			m_dataDispatcher.updateInformation(new GuiDataRenderTransparency(BlendMode::Opaque, rCam->getDisplayParameters().m_transparency, m_centralWrapper->getActiveCameraNode()), this);
	}
}

void Gui::onEditing(const bool& isEditing)
{
	m_isEditing = isEditing;
	emit sigIsEditing(isEditing);
}

void  Gui::changeEvent(QEvent* event)
{//fixme POC dynamic Language
	//	switch (event->type()) 
	//	{
	//		// this event is send if a translator is loaded
	//	case QEvent::LanguageChange:
	//		
	//
	//		break;

	//		// this event is send, if the system, language changes
	//	case QEvent::LocaleChange:
	//	{
	//	}
	//	break;
	//	}

	//QMainWindow::changeEvent(event);
}

void Gui::onProjectPath(IGuiData* data)
{
	auto dataType = static_cast<GuiDataProjectPath*>(data);
	m_openPath = QString::fromStdWString(dataType->m_path.wstring() + L'/');

	QDir::setSearchPaths(QString("file"), QStringList({ m_openPath }));
	s_fileHandler->m_projectPath = m_openPath;
}

void Gui::onPropertyPanelName(QString name)
{
    m_propertiesDock->setWindowTitle(name);
}

void Gui::initGeneralSettings()
{
    m_dataDispatcher.sendControl(new control::application::SetExamineOptions(Config::getCenteringConfiguration(), Config::getKeepingExamineConfiguration(), false));
    m_dataDispatcher.sendControl(new control::application::SetValueSettingsDisplay(Config::getUnitUsageConfiguration(), false));
    m_dataDispatcher.sendControl(new control::application::SetTemporaryFolder(Config::getTemporaryPath(), false));
    m_dataDispatcher.sendControl(new control::application::SetProjectsFolder(Config::getProjectsPath(), false));
    m_dataDispatcher.sendControl(new control::application::SetUserColor(Config::getUserColor(), 3, false, false));
    m_dataDispatcher.sendControl(new control::application::SetDecimationOptions(Config::getDecimationOptions(), true, false));
	m_dataDispatcher.sendControl(new control::application::SetExamineDisplayMode(Config::getExamineDisplayMode(), false));
	m_dataDispatcher.sendControl(new control::application::SetRecentProjects(Config::getRecentProjects(), false));
	m_dataDispatcher.sendControl(new control::application::SetAutoSaveParameters(Config::getIsAutoSaveActive(), Config::getAutoSaveTiming(), false));
	m_dataDispatcher.sendControl(new control::application::SetIndexationMethod(Config::getIndexationMethod(), false));
	float* param = Config::getGizmoParameters();
	m_dataDispatcher.sendControl(new control::application::SetGizmoParameters(true, { param[0],param[1] ,param[2] }, false));
	delete param;
	m_dataDispatcher.sendControl(new control::application::SetManipulatorSize(Config::getManipulatorSize(), false));

	m_dataDispatcher.sendControl(new control::application::SetNavigationParameters(Config::getNavigationParameters(), false));
	m_dataDispatcher.sendControl(new control::application::SetPerspectiveZBounds(Config::getPerspectiveZBounds(), false));
	m_dataDispatcher.sendControl(new control::application::SetOrthographicZBounds(Config::getOrthographicZBounds(), false));
	m_dataDispatcher.sendControl(new control::application::UnlockScanManipulation(Config::isUnlockScanManipulation(), false));

    m_maximizedFrameless = Config::getMaximizedFrameless();
    connect(m_dSettings.getFramelessCheckBox(), &QCheckBox::stateChanged, [this](int value) { this->m_maximizedFrameless = (value == Qt::CheckState::Checked); });
}

void Gui::initRegisterGuiDataDoubleEdit()
{
	for (ACustomLineEdit* widg : this->findChildren<ACustomLineEdit*>())
		if (widg->getType() == LineEditType::DOUBLE)
		{
			QDoubleEdit* doubleEdit = static_cast<QDoubleEdit*>(widg);
			doubleEdit->registerDataDispatcher(&m_dataDispatcher);
		}

}

void Gui::initDockWidgetsPosition()
{
	std::wstring path = Utils::System::getOSTProgramDataPath().wstring() + L"\\" + MAINWINDOW_INI;
	QSettings settings(QString::fromStdWString(path), QSettings::Format::IniFormat, this);
	restoreState(settings.value("windowState").toByteArray());
	restoreGeometry(settings.value("geometry").toByteArray());
}

void Gui::showSettings()
{
	m_dSettings.setModal(true);
	m_dSettings.exec();
}

void Gui::showShortcuts()
{
	m_dShortcuts.setModal(true);
	m_dShortcuts.exec();
}

void Gui::removeGroupInTab(const QString& tabName, const QString& groupName)
{
	RibbonTabContent* tab(m_ribbon->getTab(tabName));
	assert(tab != nullptr);
	if (tab)
		tab->removeGroup(groupName);
}

void Gui::disableGroupInTab(const QString& tabName, const QString& groupName)
{
	RibbonTabContent* tab(m_ribbon->getTab(tabName));
	if (tab)
		tab->disableGroup(groupName);
}

void Gui::showAbout()
{
	m_dAbout.setModal(true);
	m_dAbout.exec();
}

void Gui::onRescale(qreal dpi)
{
	m_guiScale = dpi / 96.f;
	GUILOG << "Change the GUI scale: " << m_guiScale << Logger::endl;
	scs::ApplyDarkTheme(m_guiScale);
}

void Gui::onRestoreDocks()
{
	if (m_propertiesDock->isFloating())
	{
		m_propertiesDock->setFloating(false);
		this->addDockWidget(Qt::LeftDockWidgetArea, m_propertiesDock);
	}
	if (m_workspaceDock->isFloating())
	{
		m_workspaceDock->setFloating(false);
		this->addDockWidget(Qt::LeftDockWidgetArea, m_workspaceDock);
	}
}

void Gui::onScreenChanged(QScreen* screen)
{
	GUILOG << "Change the screen | new ratio: " << screen->logicalDotsPerInch() << Logger::endl;
}

void Gui::onDeleteScanDialog(IGuiData * data)
{
	GuiDataDeleteFileDependantObjectDialog* deleteScanDialog = static_cast<GuiDataDeleteFileDependantObjectDialog*>(data);
	DialogDeleteScanTypeSelect* dialog = new DialogDeleteScanTypeSelect(m_dataDispatcher, deleteScanDialog->m_importantData, deleteScanDialog->m_otherData, nullptr);
	if (dialog->getWaitUser() == true)
		dialog->show();
	else
	{
		//Le dialog a l'attribut DeleteOnClose
		dialog->close();
	}
}

void Gui::onExportFileObject(IGuiData* data)
{
	DialogExportFileObject* dialog = new DialogExportFileObject(m_dataDispatcher, static_cast<GuiDataExportFileObjectDialog*>(data)->m_infoExport, this);
	dialog->show();

}

void Gui::onImportFileObject(IGuiData* data)
{
	GuiDataImportFileObjectDialog* importData = static_cast<GuiDataImportFileObjectDialog*>(data);
	m_importFileObject.setInfoFileObjects(importData->m_notFoundFileObjects, importData->m_isOnlyLink);
	m_importFileObject.show();
}