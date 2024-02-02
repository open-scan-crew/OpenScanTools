#include "gui/viewport/QuickBarNavigation.h"
#include "gui/viewport/VulkanViewport.h"
#include "gui/Texts.hpp"

#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"

#include "models/3d/ManipulationTypes.h"
#include "gui/GuiData/GuiData3dObjects.h"

#include "models/3d/Graph/CameraNode.h"

#include "controller/controls/ControlViewport.h"

#include "controller/controls/ControlViewport.h"
#include "controller/controls/ControlFunction.h"

QuickBarNavigation::QuickBarNavigation(QWidget* parent, IDataDispatcher& dataDispatcher, float guiScale)
    : QWidget(parent)
    , m_dataDispatcher(dataDispatcher)
    , m_camera()
    , m_fullScreenActive(false)
{
    m_ui.setupUi(this);
    setEnabled(false);

    // Set the right size for the icons
    m_ui.toolButton_explore->setIconSize(QSize(20, 20) * guiScale);
    m_ui.toolButton_examine->setIconSize(QSize(20, 20) * guiScale);
    m_ui.toolButton_orthographic->setIconSize(QSize(20, 20) * guiScale);
    m_ui.toolButton_perspective->setIconSize(QSize(20, 20) * guiScale);
    m_ui.toolButton_rotate90->setIconSize(QSize(20, 20) * guiScale);
    m_ui.toolButton_zoomExtent->setIconSize(QSize(20, 20) * guiScale);
    m_ui.comboBox_views->setIconSize(QSize(20, 20) * guiScale);
    m_ui.toolButton_align2Points->setIconSize(QSize(20, 20) * guiScale);
    m_ui.toolButton_align3Points->setIconSize(QSize(20, 20) * guiScale);
    m_ui.toolButton_alignBox->setIconSize(QSize(20, 20) * guiScale);
    m_ui.toolButton_hideBar->setIconSize(QSize(20, 20) * guiScale);
    m_ui.toolButton_gizmo->setIconSize(QSize(20, 20) * guiScale);

    m_ui.toolButton_translation->setIconSize(QSize(20, 20) * guiScale);
    m_ui.toolButton_rotation->setIconSize(QSize(20, 20) * guiScale);
    m_ui.toolButton_extrusion->setIconSize(QSize(20, 20) * guiScale);
    m_ui.toolButton_moveManip->setIconSize(QSize(20, 20) * guiScale);

    m_ui.toolButton_hideBar->hide();

#ifndef _DEBUG_
    m_ui.checkBox_refresh->hide();
#endif

    m_ui.comboBox_views->addItem(QIcon(":icons/100x100/top_view.png"), TEXT_VIEW_TOP, QVariant((int)AlignView::Top));
    m_ui.comboBox_views->addItem(QIcon(":icons/100x100/bottom_view.png"), TEXT_VIEW_BOTTOM, QVariant((int)AlignView::Bottom));
    m_ui.comboBox_views->addItem(QIcon(":icons/100x100/left_view.png"), TEXT_VIEW_LEFT, QVariant((int)AlignView::Left));
    m_ui.comboBox_views->addItem(QIcon(":icons/100x100/right_view.png"), TEXT_VIEW_RIGHT, QVariant((int)AlignView::Right));
    m_ui.comboBox_views->addItem(QIcon(":icons/100x100/front_view.png"), TEXT_VIEW_FRONT, QVariant((int)AlignView::Front));
    m_ui.comboBox_views->addItem(QIcon(":icons/100x100/rear_view.png"), TEXT_VIEW_BACK, QVariant((int)AlignView::Back));;
    m_ui.comboBox_views->addItem(QIcon(":icons/100x100/iso_view.png"), TEXT_VIEW_ISO, QVariant((int)AlignView::Iso));;

    connect(m_ui.toolButton_examine, &QToolButton::released, [this]() { m_dataDispatcher.sendControl(new control::viewport::Examine(m_camera)); });
    connect(m_ui.toolButton_hideBar, &QToolButton::released, this, &QWidget::hide);

    connect(m_ui.toolButton_translation, &QToolButton::released, this, [this]() { this->onManipulationMode(ManipulationMode::Translation); });
    connect(m_ui.toolButton_extrusion, &QToolButton::released, this, [this]() { this->onManipulationMode(ManipulationMode::Extrusion); });
    connect(m_ui.toolButton_rotation, &QToolButton::released, this, [this]() { this->onManipulationMode(ManipulationMode::Rotation); });

    connect(m_ui.toolButton_moveManip, &QToolButton::released, this, &QuickBarNavigation::onMoveManip);

    registerGuiDataFunction(guiDType::projectLoaded, &QuickBarNavigation::onProjectLoaded);
    registerGuiDataFunction(guiDType::activatedFunctions, &QuickBarNavigation::onActivateFunction);
    registerGuiDataFunction(guiDType::renderGuizmo, &QuickBarNavigation::onGuizmoDisplay);
    registerGuiDataFunction(guiDType::renderActiveCamera, &QuickBarNavigation::onCameraToViewpoint);
    registerGuiDataFunction(guiDType::manipulatorModeCallBack, &QuickBarNavigation::onUpdateManipulationMode);
}

QuickBarNavigation::~QuickBarNavigation()
{
}

void QuickBarNavigation::informData(IGuiData *data)
{
    if (m_functions.find(data->getType()) != m_functions.end())
    {
        GuiDataFunction fct = m_functions.at(data->getType());
        (this->*fct)(data);
    }
}

void QuickBarNavigation::connectViewport(VulkanViewport* viewport)
{
    connect(viewport, &VulkanViewport::showOverlay, this, &QuickBarNavigation::onToggleVisible);

    // TEMPORAIRE - Pour test de rendu
    connect(m_ui.checkBox_refresh, &QCheckBox::stateChanged, viewport, &VulkanViewport::slotForceUpdate);
}

void QuickBarNavigation::connectCamera(SafePtr<CameraNode> camera)
{
    m_camera = camera;
    // TODO - connect the camera if needed
    //connect(viewport, &VulkanViewport::examineActive, this, &QuickBarNavigation::onActiveExamine);
    connect(m_ui.toolButton_rotate90, &QToolButton::released, [this]() {
        WritePtr<CameraNode> wCam = m_camera.get();
        if (!wCam)
            return;
        wCam->rotate90degrees();
    });

    connect(m_ui.toolButton_explore, &QToolButton::released, [this]() {
        WritePtr<CameraNode> wCam = m_camera.get();
        if (!wCam)
            return;
        wCam->resetExaminePoint();
    });

    connect(m_ui.toolButton_perspective, &QToolButton::released,
        [this]() {
        WritePtr<CameraNode> wCam = m_camera.get();
        if (!wCam)
            return;
        wCam->setProjectionMode(ProjectionMode::Perspective);
        m_ui.toolButton_orthographic->setChecked(false);
    });
    connect(m_ui.toolButton_orthographic, &QToolButton::released,
        [this]() {
        WritePtr<CameraNode> wCam = m_camera.get();
        if (!wCam)
            return;
        wCam->setProjectionMode(ProjectionMode::Orthographic);
        m_ui.toolButton_perspective->setChecked(false);
    });

    connect(m_ui.toolButton_zoomExtent, &QToolButton::released, [this]() { this->m_dataDispatcher.sendControl(new control::viewport::AlignViewSide(AlignView::Reset, m_camera)); });

    connect(m_ui.comboBox_views, QOverload<int>::of(&QComboBox::activated), [this](int i) {
        AlignView align = (AlignView)m_ui.comboBox_views->currentData().toInt();
        m_dataDispatcher.sendControl(new control::viewport::AlignViewSide(align, m_camera));
    });

    connect(m_ui.toolButton_align2Points, &QToolButton::released, [this]() {
        m_dataDispatcher.sendControl(new control::viewport::AlignView2PointsFunction());
    });
    connect(m_ui.toolButton_align3Points, &QToolButton::released, [this]() {
        m_dataDispatcher.sendControl(new control::viewport::AlignView3PointsFunction());
    });

    connect(m_ui.toolButton_alignBox, &QToolButton::released, [this]() {
        m_dataDispatcher.sendControl(new control::viewport::AlignViewBoxFunction());
    });

    connect(m_ui.toolButton_gizmo, &QToolButton::released, [this]() {
        m_dataDispatcher.updateInformation(new GuiDataDisplayGuizmo(m_camera, m_ui.toolButton_gizmo->isChecked()), this); });

}


void QuickBarNavigation::onProjectLoaded(IGuiData* data)
{
    auto* guiData = static_cast<GuiDataProjectLoaded*>(data);
    setEnabled(guiData->m_isProjectLoad);
}

void QuickBarNavigation::onActivateFunction(IGuiData* data)
{
    auto* function = static_cast<GuiDataActivatedFunctions*>(data);

    blockSignals_functions(true);

    switch (function->type)
    {
    case ContextType::alignView2P:
    case ContextType::alignView3P:
    case ContextType::alignViewBox:
        break;
    default:
        m_ui.toolButton_align2Points->setChecked(false);
        m_ui.toolButton_align3Points->setChecked(false);
        m_ui.toolButton_alignBox->setChecked(false);
        m_ui.toolButton_moveManip->setChecked(false);
    }

    blockSignals_functions(false);
}

void QuickBarNavigation::onCameraToViewpoint(IGuiData* data)
{
    auto gui = static_cast<GuiDataCameraInfo*>(data);
    if (gui->m_camera && m_camera != gui->m_camera)
        return;

    ReadPtr<CameraNode> rCam = m_camera.cget();
    if (!rCam)
        return;

    blockSignals_camera(true);

    bool isExamineActive = rCam->isExamineActive();
    m_ui.toolButton_explore->setChecked(!isExamineActive);
    m_ui.toolButton_examine->setChecked(isExamineActive);
    m_ui.toolButton_orthographic->setChecked(rCam->getProjectionMode() == ProjectionMode::Orthographic);
    m_ui.toolButton_perspective->setChecked(rCam->getProjectionMode() == ProjectionMode::Perspective);

    blockSignals_camera(false);
}

void QuickBarNavigation::onUpdateManipulationMode(IGuiData* data)
{
    GuiDataCallBackManipulatorMode* castData = static_cast<GuiDataCallBackManipulatorMode*>(data);
    
    m_ui.toolButton_extrusion->setChecked(castData->m_mode == ManipulationMode::Extrusion);
    m_ui.toolButton_translation->setChecked(castData->m_mode == ManipulationMode::Translation);
    m_ui.toolButton_rotation->setChecked(castData->m_mode == ManipulationMode::Rotation);

}

void QuickBarNavigation::onActiveExamine(bool isActive)
{
    m_ui.toolButton_examine->blockSignals(true);
    m_ui.toolButton_explore->blockSignals(true);

    m_ui.toolButton_examine->setChecked(isActive);
    m_ui.toolButton_explore->setChecked(!isActive);

    m_ui.toolButton_examine->blockSignals(false);
    m_ui.toolButton_explore->blockSignals(false);
}

void QuickBarNavigation::onGuizmoDisplay(IGuiData* data)
{
    auto castData = static_cast<GuiDataDisplayGuizmo*>(data);
    if (!castData->m_camera || castData->m_camera == m_camera)
        m_ui.toolButton_gizmo->setChecked(castData->m_isDisplayed);
}

void QuickBarNavigation::onFullScreenMode(bool fullScreenActivated)
{
    m_fullScreenActive = fullScreenActivated;
    m_ui.toolButton_hideBar->setVisible(fullScreenActivated);
    if (fullScreenActivated == false)
        this->show();
}

void QuickBarNavigation::onToggleVisible()
{
    if (m_fullScreenActive)
        this->setVisible(!isVisible());
}

void QuickBarNavigation::onManipulationMode(ManipulationMode mode)
{
    m_ui.toolButton_extrusion->setChecked(mode == ManipulationMode::Extrusion ? false : m_ui.toolButton_extrusion->isChecked());
    m_ui.toolButton_translation->setChecked(mode == ManipulationMode::Translation ? false : m_ui.toolButton_translation->isChecked());
    m_ui.toolButton_rotation->setChecked(mode == ManipulationMode::Rotation ? false : m_ui.toolButton_rotation->isChecked());

    m_dataDispatcher.updateInformation(new GuiDataManipulatorMode(mode));
}

void QuickBarNavigation::onMoveManip()
{
    m_dataDispatcher.sendControl(new control::viewport::MoveManipFunction());
}

void QuickBarNavigation::blockSignals_camera(bool block)
{
    m_ui.toolButton_explore->blockSignals(block);
    m_ui.toolButton_examine->blockSignals(block);
    m_ui.toolButton_orthographic->blockSignals(block);
    m_ui.toolButton_perspective->blockSignals(block);
}

void QuickBarNavigation::blockSignals_functions(bool block)
{
    m_ui.toolButton_align2Points->blockSignals(block);
    m_ui.toolButton_align3Points->blockSignals(block);
    m_ui.toolButton_alignBox->blockSignals(block);
}