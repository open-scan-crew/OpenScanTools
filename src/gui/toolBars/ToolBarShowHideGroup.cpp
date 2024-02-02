#include <QtWidgets/QWidget>

#include "gui/toolBars/ToolBarShowHideGroup.h"
#include "controller/controls/ControlSpecial.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "models/3d/Graph/CameraNode.h"

ToolBarShowHideGroup::ToolBarShowHideGroup(IDataDispatcher &dataDispatcher, QWidget *parent, float guiScale)
    : QWidget(parent)
    , m_dataDispatcher(dataDispatcher)
{
    m_ui.setupUi(this);
    setEnabled(false);

    // Set the right size for the icons
    m_ui.ShowHideScanButton->setIconSize(QSize(20, 20) * guiScale);
    m_ui.ShowHideTagButton->setIconSize(QSize(20, 20) * guiScale);
    m_ui.ShowHideViewpointButton->setIconSize(QSize(20, 20) * guiScale);
    m_ui.ShowHideSelectedButton->setIconSize(QSize(20, 20) * guiScale);
    m_ui.ShowHideUnselectedButton->setIconSize(QSize(20, 20) * guiScale);
    m_ui.ShowHideMarkersTextButton->setIconSize(QSize(20, 20) * guiScale);
    m_ui.ShowHideMeasurementsButton->setIconSize(QSize(20, 20) * guiScale);
    m_ui.ShowClippingsButton->setIconSize(QSize(20, 20) * guiScale);
    m_ui.ShowHideAll->setIconSize(QSize(20, 20) * guiScale);
    m_ui.ShowHidePipesButton->setIconSize(QSize(20, 20) * guiScale);
    m_ui.ShowHidePointsButton->setIconSize(QSize(20, 20) * guiScale);

    connect(m_ui.ShowHideScanButton, &QToolButton::clicked, this, &ToolBarShowHideGroup::slotToogleShowScanMarkers);
    connect(m_ui.ShowHideTagButton, &QToolButton::clicked, this, &ToolBarShowHideGroup::slotToogleShowTagMarkers);
    connect(m_ui.ShowHideViewpointButton, &QToolButton::clicked, this, &ToolBarShowHideGroup::slotToogleShowViewpointMarkers);
    connect(m_ui.ShowHideSelectedButton, &QToolButton::clicked, this, &ToolBarShowHideGroup::slotToogleShowCurrentObjects);
    connect(m_ui.ShowHideUnselectedButton, &QToolButton::clicked, this, &ToolBarShowHideGroup::slotToogleShowUncurrentObjects);
    connect(m_ui.ShowHideMarkersTextButton, &QToolButton::clicked, this, &ToolBarShowHideGroup::slotToogleShowMarkersText);
    connect(m_ui.ShowHideMeasurementsButton, &QToolButton::clicked, this, &ToolBarShowHideGroup::slotToogleShowMeasures);
    connect(m_ui.ShowClippingsButton, &QToolButton::clicked, this, &ToolBarShowHideGroup::slotToogleShowClippings);
    connect(m_ui.ShowHideAll, &QToolButton::clicked, this, &ToolBarShowHideGroup::slotToogleShowAll);
    connect(m_ui.ShowHidePipesButton, &QToolButton::clicked, this, &ToolBarShowHideGroup::slotToogleShowPipes);
    connect(m_ui.ShowHidePointsButton, &QToolButton::clicked, this, &ToolBarShowHideGroup::slotToogleShowPoints);

    registerGuiDataFunction(guiDType::projectLoaded, &ToolBarShowHideGroup::onProjectLoad);
    registerGuiDataFunction(guiDType::focusViewport, &ToolBarShowHideGroup::onFocusViewport);
    registerGuiDataFunction(guiDType::renderActiveCamera, &ToolBarShowHideGroup::onActiveCamera);

}

ToolBarShowHideGroup::~ToolBarShowHideGroup()
{
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarShowHideGroup::informData(IGuiData *data)
{
    if (m_methods.find(data->getType()) != m_methods.end())
    {
        GuiDataFunction method = m_methods.at(data->getType());
        (this->*method)(data);
    }
}

void ToolBarShowHideGroup::onProjectLoad(IGuiData * data)
{
    GuiDataProjectLoaded *projectLoaded = static_cast<GuiDataProjectLoaded*>(data);
    setEnabled(projectLoaded->m_isProjectLoad);
}

void ToolBarShowHideGroup::onFocusViewport(IGuiData* data)
{
    GuiDataFocusViewport* castData = static_cast<GuiDataFocusViewport*>(data);
    if (castData->m_forceFocus && castData->m_camera)
        m_focusCamera = castData->m_camera;
}

void ToolBarShowHideGroup::onActiveCamera(IGuiData* data)
{
    GuiDataCameraInfo* castData = static_cast<GuiDataCameraInfo*>(data);
    if (castData->m_camera && m_focusCamera != castData->m_camera)
        return;

    ReadPtr<CameraNode> cam = m_focusCamera.cget();
    if (!cam)
        return;

    DisplayParameters displayParam = cam->getDisplayParameters();

    toggleShowHideIcon(m_ui.ShowHideScanButton, displayParam.m_markerMask & SHOW_SCAN_MARKER);
    toggleShowHideIcon(m_ui.ShowHideTagButton, displayParam.m_markerMask & SHOW_TAG_MARKER);
}

void ToolBarShowHideGroup::toggleShowHideIcon(QToolButton* button, bool show)
{
    button->setIcon(show ? QIcon(":icons/100x100/show.png") : QIcon(":icons/100x100/hide.png"));
}

void ToolBarShowHideGroup::slotToogleShowScanMarkers()
{
    WritePtr<CameraNode> cam = m_focusCamera.get();
    if (!cam)
        return;

    MarkerShowMask new_mask = cam->getDisplayParameters().m_markerMask ^ SHOW_SCAN_MARKER;
    cam->setMarkerShowMask(new_mask);

    toggleShowHideIcon(m_ui.ShowHideScanButton, new_mask & SHOW_SCAN_MARKER);
}

void ToolBarShowHideGroup::slotToogleShowTagMarkers()
{
    /*
    WritePtr<CameraNode> cam = m_focusCamera.get();
    if (!cam)
        return;

    MarkerShowMask new_mask = cam->getDisplayParameters().m_markerMask ^ SHOW_TAG_MARKER;
    cam->setMarkerShowMask(new_mask);

    toggleShowHideIcon(m_ui.ShowHideTagButton, new_mask & SHOW_TAG_MARKER);
    */

    m_showTagMarkers = !m_showTagMarkers;

    toggleShowHideIcon(m_ui.ShowHideTagButton, m_showTagMarkers);

    m_dataDispatcher.sendControl(new control::special::ShowHideObjects({ ElementType::Tag }, m_showTagMarkers));
}

void ToolBarShowHideGroup::slotToogleShowViewpointMarkers()
{
    m_showViewpointMarkers = !m_showViewpointMarkers;

    toggleShowHideIcon(m_ui.ShowHideViewpointButton, m_showViewpointMarkers);

    m_dataDispatcher.sendControl(new control::special::ShowHideObjects({ ElementType::ViewPoint }, m_showViewpointMarkers));
}

void ToolBarShowHideGroup::slotToogleShowClippings()
{
    m_showClippings = !m_showClippings;

    toggleShowHideIcon(m_ui.ShowClippingsButton, m_showClippings);

    m_dataDispatcher.sendControl(new control::special::ShowHideObjects({ ElementType::Box, ElementType::Grid }, m_showClippings));
}

void ToolBarShowHideGroup::slotToogleShowAll()
{
    m_showAll = !m_showAll;

    toggleShowHideIcon(m_ui.ShowHideAll, m_showAll);

    m_dataDispatcher.sendControl(new control::special::ShowAll(m_showAll));
}

void ToolBarShowHideGroup::slotToogleShowPipes()
{
    m_showPipes = !m_showPipes;

    toggleShowHideIcon(m_ui.ShowHidePipesButton, m_showPipes);

    m_dataDispatcher.sendControl(new control::special::ShowHideObjects({ ElementType::Cylinder, ElementType::Torus }, m_showPipes));
}

void ToolBarShowHideGroup::slotToogleShowPoints()
{
    m_showPoints = !m_showPoints;

    toggleShowHideIcon(m_ui.ShowHidePointsButton, m_showPoints);

    m_dataDispatcher.sendControl(new control::special::ShowHideObjects({ ElementType::Point }, m_showPoints));
}

void ToolBarShowHideGroup::slotToogleShowMarkersText()
{
    m_showObjectTexts = !m_showObjectTexts;

    toggleShowHideIcon(m_ui.ShowHideMarkersTextButton, m_showObjectTexts);

    m_dataDispatcher.updateInformation(new GuiDataRenderDisplayObjectTexts(m_showObjectTexts, SafePtr<CameraNode>()));
}

void ToolBarShowHideGroup::slotToogleShowMeasures()
{
    m_showMeasure = !m_showMeasure;

    toggleShowHideIcon(m_ui.ShowHideMeasurementsButton, m_showMeasure);

    m_dataDispatcher.sendControl(new control::special::ShowHideObjects({
        ElementType::BeamBendingMeasure,
        ElementType::ColumnTiltMeasure,
        ElementType::PipeToPipeMeasure,
        ElementType::PipeToPlaneMeasure,
        ElementType::PointToPipeMeasure,
        ElementType::PointToPlaneMeasure,
        ElementType::SimpleMeasure,
        ElementType::PolylineMeasure,
        ElementType::PolylineMeasure }, m_showMeasure));
}

void ToolBarShowHideGroup::slotToogleShowCurrentObjects()
{
    m_showSelected = !m_showSelected;

    toggleShowHideIcon(m_ui.ShowHideSelectedButton, m_showSelected);

    m_dataDispatcher.sendControl(new control::special::ShowHideCurrentObjects(m_showSelected));
}

void ToolBarShowHideGroup::slotToogleShowUncurrentObjects()
{
    m_showUnselected = !m_showUnselected;

    toggleShowHideIcon(m_ui.ShowHideUnselectedButton, m_showUnselected);

    m_dataDispatcher.sendControl(new control::special::ShowHideUncurrentObjects(m_showUnselected));
}
