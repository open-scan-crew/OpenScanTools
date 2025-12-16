#include "gui/viewport/VulkanViewport.h"
#include "controller/controls/ControlPicking.h"
#include "controller/controls/ControlViewport.h"

#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataUserOrientation.h"

#include "models/graph/CameraNode.h"
#include "models/graph/PointCloudNode.h"
#include "models/graph/ManipulatorNode.h"
#include "models/ElementType.h"

#include "utils/math/trigo.h"

#include "vulkan/TlFramebuffer_T.h"
#include "vulkan/VulkanManager.h"

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include <QtGui/qevent.h>
#include <QApplication.h>


double calcTranslationSpeedFactor(NavigationParameters navParam)
{
    double slowestFactor = 0.01;
    return (slowestFactor + std::pow(navParam.cameraTranslationSpeedFactor / 100., 3) * (1 - slowestFactor));
}

double calcRotationExamineSpeedFactor(NavigationParameters navParam)
{
    double slowestFactor = 0.2;
    return (slowestFactor + std::pow(navParam.cameraRotationExamineFactor / 100., 2) * (1 - slowestFactor));
}

VulkanViewport::VulkanViewport(IDataDispatcher& dataDispatcher, float guiScale)
    : m_dataDispatcher(dataDispatcher)
    , m_initialized(false)
    , m_naviMode(NavigationMode::Explore)
    , m_mouseInputEffect(MouseInputEffect::None)
    , m_selectionRect{ {-1, -1}, {-1, -1} }
    , m_hoverRect{ {-1, -1}, {-1, -1} }
    , m_saveImagesAnim(false)
    , m_guiScale(guiScale)
    , m_hoveredId(INVALID_PICKING_ID)
    , m_lastPicking(NAN, NAN, NAN)
    , m_3dMouseUpdate(false)
    , m_actionToPull(Action::None)
{
    setSurfaceType(QSurface::VulkanSurface);

    registerGuiDataFunction(guiDType::renderNavigationParameters, &VulkanViewport::onRenderNavigationParameters);
    registerGuiDataFunction(guiDType::renderViewPointAnimation, &VulkanViewport::onRenderViewPointAnimation);
    registerGuiDataFunction(guiDType::renderAnimationSpeed, &VulkanViewport::onRenderAnimationSpeed);
    registerGuiDataFunction(guiDType::renderAnimationLoop, &VulkanViewport::onRenderAnimationLoop);
    registerGuiDataFunction(guiDType::renderStartAnimation, &VulkanViewport::onRenderStartAnimation);
    registerGuiDataFunction(guiDType::renderStopAnimation, &VulkanViewport::onRenderStopAnimation);
    registerGuiDataFunction(guiDType::renderCleanAnimationList, &VulkanViewport::onRenderCleanAnimationList);
    registerGuiDataFunction(guiDType::userOrientation, &VulkanViewport::onUserOrientation);
    registerGuiDataFunction(guiDType::projectOrientation, &VulkanViewport::onProjectOrientation);
    registerGuiDataFunction(guiDType::quitEvent, &VulkanViewport::onQuitEvent);
    registerGuiDataFunction(guiDType::renderDecimationOptions, &VulkanViewport::onRenderDecimationOptions);
}

VulkanViewport::~VulkanViewport()
{
    Logger::log(LoggerMode::VKLog) << "Destroying Vulkan Viewport..." << Logger::endl;
    m_dataDispatcher.unregisterObserver(this);

    VulkanManager& vkM = VulkanManager::getInstance();
    if (m_framebuffer != TL_NULL_HANDLE)
        vkM.destroyFramebuffer(m_framebuffer);
}

void VulkanViewport::informData(IGuiData *data)
{
    if (m_functions.find(data->getType()) != m_functions.end())
    {
        GuiDataFunction fct = m_functions.at(data->getType());
        (this->*fct)(data);
    }
}

void VulkanViewport::onRenderNavigationParameters(IGuiData* data)
{
    auto castData = static_cast<GuiDataRenderNavigationParameters*>(data);
    m_navParams = castData->m_navParam;
}

void VulkanViewport::onRenderViewPointAnimation(IGuiData* data)
{
    auto keyPointData = static_cast<GuiDataRenderAnimationViewPoint*>(data);
    WritePtr<CameraNode> wCam = m_cam.get();
    if (!wCam)
        return;
    wCam->AddViewPoint(keyPointData->m_keypoint);
}

void VulkanViewport::onRenderAnimationSpeed(IGuiData* data)
{
    auto speedData = static_cast<GuiDataRenderAnimationSpeed*>(data);
    //m_cam->setSpeed(speedData->m_speed);
}

void VulkanViewport::onRenderAnimationLoop(IGuiData* data)
{
    auto loopData = static_cast<GuiDataRenderAnimationLoop*>(data);
    //m_cam->setLoop(loopData->_loop);
}

void VulkanViewport::onRenderStartAnimation(IGuiData* data)
{
    WritePtr<CameraNode> wCam = m_cam.get();
    if (!wCam)
        return;
    wCam->startAnimation(m_saveImagesAnim);
}

void VulkanViewport::onRenderStopAnimation(IGuiData* data)
{
    WritePtr<CameraNode> wCam = m_cam.get();
    if (!wCam)
        return;
    wCam->endAnimation();
}

void VulkanViewport::onRenderCleanAnimationList(IGuiData* data)
{
    WritePtr<CameraNode> wCam = m_cam.get();
    if (!wCam)
        return;
    wCam->cleanAnimation();
}

void VulkanViewport::onUserOrientation(IGuiData* data)
{
    auto userOrientation = static_cast<GuiDataSetUserOrientation*>(data);
    WritePtr<CameraNode> wCam = m_cam.get();
    if (!wCam)
        return;
    wCam->setApplyUserOrientation(true);
    wCam->setUserOrientation(userOrientation->m_userOrientation);
    Logger::log(LoggerMode::GuiLog) << "Viewport - User orientation name : " << userOrientation->m_userOrientation.getName().toStdString() << Logger::endl;
}

void VulkanViewport::onProjectOrientation(IGuiData* data)
{
    WritePtr<CameraNode> wCam = m_cam.get();
    if (!wCam)
        return;
    wCam->setApplyUserOrientation(false);
}

void VulkanViewport::onQuitEvent(IGuiData* data)
{
    m_initialized = false;
}

void VulkanViewport::onRenderDecimationOptions(IGuiData* data)
{
    m_decimationOptions = static_cast<GuiDataRenderDecimationOptions*>(data)->m_options;
}

void VulkanViewport::initSurface()
{
    VulkanManager& vkM = VulkanManager::getInstance();

    vkM.initFramebuffer(m_framebuffer, QWindow::winId(), width(), height(), true);
}

void VulkanViewport::setCamera(SafePtr<CameraNode> camera)
{
    m_cam = camera;
}

SafePtr<CameraNode> VulkanViewport::getCamera() const
{
    return m_cam;
}

NavigationMode  VulkanViewport::getNavigationMode() const
{
    return m_naviMode;
}

TlFramebuffer VulkanViewport::getFramebuffer() const
{
    return m_framebuffer;
}

bool VulkanViewport::isInitialized() const
{
    return m_initialized;
}

void VulkanViewport::updateInputs(WritePtr<CameraNode>& wCam, SafePtr<ManipulatorNode> manipNode, SafePtr<AGraphNode> hoveredNode)
{
    // TODO - Define an action Select that contain the rect.
    // Reset the selection rectangle
    m_selectionRect = { {-1, -1}, {-1, -1} };

    // Update automatic animation already running
    wCam->updateAnimation();

    updateProjNaviMode(wCam); // here or after ?

    // Skip inputs when in animation mode
    if (!wCam->isAnimated())
    {
        updateMouseInputEffect(wCam, manipNode);
        applyMouseInput(wCam, manipNode);
        applyKeyboardInput(wCam);
    }

    doAction(wCam, manipNode, hoveredNode);

    wCam->refresh();
    glm::dvec3 pos = wCam->getCenter();
    emit cameraPosition(pos.x, pos.y, pos.z, this);
}

void VulkanViewport::slotForceUpdate(int state)
{
    m_forceUpdate = (state == 2) ? true : false;
}

ClickInfo VulkanViewport::generateRaytracingInfos(const WritePtr<CameraNode>& wCam)
{
    assert(wCam);

    glm::dvec4 eyeCoord = wCam->getEyeCoord((double)(m_MI.lastX), (double)(m_MI.lastY), 1.0f, (double)width(), (double)height());
    glm::dvec4 pickPos = wCam->getModelMatrix() * eyeCoord;
    glm::dvec4 eyeCoord0 = wCam->getEyeCoord((double)(m_MI.lastX), (double)(m_MI.lastY), 0.0f, (double)width(), (double)height());
    glm::dvec4 pickPos0 = wCam->getModelMatrix() * eyeCoord0;

    glm::dvec3 rayOrigin = glm::dvec3(pickPos0.x, pickPos0.y, pickPos0.z);

    glm::dvec3 ray = glm::dvec3(pickPos.x - pickPos0.x, pickPos.y - pickPos0.y, pickPos.z - pickPos0.z);

    uint32_t w = m_framebuffer->extent.width;
    uint32_t h = m_framebuffer->extent.height;

    ReadPtr<PointCloudNode> panoramic = wCam->getPanoramicScan().cget();

    return ClickInfo(
        w,
        h,
        QApplication::queryKeyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier),
        wCam->getFovy(),
        wCam->getHeightAt1m(),
        SafePtr<AGraphNode>(),
        nullptr,
        glm::dmat4(),
        m_lastPicking,
        ray,
        rayOrigin,
        panoramic ? panoramic->getScanGuid() : tls::ScanGuid(),
        getCamera(),
        false
    );
}

Pos3D VulkanViewport::getPickingPos(const CameraNode& rCam)
{
    VulkanManager& vkManager = VulkanManager::getInstance();
    float depth = vkManager.sampleDepth(m_framebuffer);

    if (depth < 1.f) {
        glm::dvec4 eyeCoord = rCam.getEyeCoord((double)(m_MI.lastX), (double)(m_MI.lastY),
            (double)depth, (double)width(), (double)height());
        glm::dvec4 pickPos = rCam.getModelMatrix() * eyeCoord;
        emit pickingPosition(pickPos.x, pickPos.y, pickPos.z, this);
        return { pickPos.x, pickPos.y, pickPos.z };
    }
    else {
        return { std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::quiet_NaN() };
    }
}

std::unordered_set<uint32_t> VulkanViewport::getSelectedIds() const
{
    // TODO
    //if (m_mouseInputEffect == Action::Select)
    if (m_selectionRect.c0.x != -1 && m_selectionRect.c1.x != -1)
    {
        return m_pickingManager.getObjects(m_framebuffer, m_selectionRect);
    }
    else
        return {};
}

std::unordered_set<uint32_t> VulkanViewport::getHoveredIds() const
{
    if (m_mouseInputEffect == MouseInputEffect::Selection)
    {
        return m_pickingManager.getObjects(m_framebuffer, m_hoverRect);
    }
    else
        return { m_hoveredId };
}

void VulkanViewport::refreshHoveredId(uint32_t textId)
{
    // Priorités des hover : Manipulateurs, textes, objets
    
    m_hoveredId = VulkanManager::getInstance().sampleIndex(m_framebuffer, (uint32_t)m_MI.lastX, (uint32_t)m_MI.lastY);

    if ((m_hoveredId == INVALID_PICKING_ID || (m_hoveredId & RESERVED_DATA_ID_MASK) == 0) &&
        textId != INVALID_PICKING_ID)
    {
        m_hoveredId = textId;
    }
}

Rect2D VulkanViewport::getSelectionRect() const
{
    return m_selectionRect;
}

Rect2D VulkanViewport::getHoverRect() const
{
    return m_hoverRect;
}

glm::ivec2 VulkanViewport::getMousePos() const
{
    return glm::ivec2(m_MI.lastX, m_MI.lastY);
}

glm::vec2 VulkanViewport::getMousePosNormalized() const
{
    return glm::vec2((float)m_MI.lastX / width(), (float)m_MI.lastY / height());
}

void VulkanViewport::setMissingScanPart(bool isMissingScanPart)
{
    m_updateMissingScanPart = isMissingScanPart;
}

bool VulkanViewport::mustRenderFullScans()
{
    return m_updateScansFullRender;
}

void VulkanViewport::quitPanoramic(WritePtr<CameraNode>& wCam)
{
    if (m_naviMode == NavigationMode::Panoramic)
    {
        m_naviMode = NavigationMode::Explore;
        wCam->resetPerspectiveZoom();
        wCam->resetPanoramicScan();
        wCam->resetExaminePoint();
    }
}

void VulkanViewport::doAction(const WritePtr<CameraNode>& wCam, SafePtr<ManipulatorNode>& manipNode, SafePtr<AGraphNode>& hoverObject)
{
    if (m_actionToPull != VulkanViewport::Action::None)
    {
        ClickInfo click = generateRaytracingInfos(wCam);
        click.hover = hoverObject;
        click.forceObjectCenter = m_forceObjectCenterOnExamine;

        switch (m_actionToPull)
        {
        case VulkanViewport::Action::DoubleClick:
            m_dataDispatcher.updateInformation(new GuiDataMoveToData(click.hover));
            break;
        case VulkanViewport::Action::Click:
            m_dataDispatcher.sendControl(new control::picking::Click(click));
            break;
        case VulkanViewport::Action::Examine:
        {
            ElementType hoverType = ElementType::None;
            if (click.hover)
            {
                ReadPtr<AGraphNode> rHover = click.hover.cget();
                if (rHover)
                    hoverType = rHover->getType();
            }

            if (hoverType == ElementType::Scan || hoverType == ElementType::ViewPoint)
                m_dataDispatcher.updateInformation(new GuiDataMoveToData(click.hover));
            else
                m_dataDispatcher.sendControl(new control::viewport::Examine(click));
            break;
        }
        //case VulkanViewport::Action::BeginManipulation:
        //    ManipulatorNode::setCurrentSelection(m_hoveredId & RESERVED_DATA_ID_MASK, glm::ivec2(m_MI.lastX, m_MI.lastY), manipNode);
        //    break;
        case VulkanViewport::Action::EndManipulation:
            ManipulatorNode::setCurrentSelection(Selection::None, glm::ivec2(m_MI.lastX, m_MI.lastY), manipNode);
            break;
        }
    }
    m_forceObjectCenterOnExamine = false;
    m_actionToPull = Action::None;
}

void VulkanViewport::updateProjNaviMode(WritePtr<CameraNode>& wCam)
{
    bool examineActive = wCam->isExamineActive();

    if (wCam->getProjectionMode() == ProjectionMode::Perspective)
    {
        if (wCam->getPanoramicScan())
            m_naviMode = NavigationMode::Panoramic;
        else if (examineActive)
            m_naviMode = NavigationMode::Examine;
        else
            m_naviMode = NavigationMode::Explore;
    }
    else // orthographic
    {
        if (examineActive)
            m_naviMode = NavigationMode::OrthoExamine;
        else
            m_naviMode = NavigationMode::Orthographic;
    }
}

//---------------------------------------------------
//                    Events
//---------------------------------------------------

bool VulkanViewport::event(QEvent *_event)
{
    switch (_event->type()) {
    case QEvent::UpdateRequest:
        m_refreshViewport = true;
        break;

    case QEvent::PlatformSurface:
        if (static_cast<QPlatformSurfaceEvent*>(_event)->surfaceEventType() == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed)
        {
            if (m_framebuffer != TL_NULL_HANDLE)
                VulkanManager::getInstance().destroyFramebuffer(m_framebuffer);
        }
        break;

    default:
        break;
    }

    return QWindow::event(_event);
}

void VulkanViewport::resizeEvent(QResizeEvent *_event)
{
    if (m_framebuffer != TL_NULL_HANDLE)
    {
        VulkanManager::resizeFramebuffer(m_framebuffer, width(), height());
    }

    m_dataDispatcher.updateInformation(new GuiDataFocusViewport(m_cam, width(), height(), false));
    m_pickingManager.resetPickingStored();
    m_refreshViewport = true;
}

void VulkanViewport::exposeEvent(QExposeEvent *_event)
{
    WritePtr<CameraNode> wCam = m_cam.get();
    if (!wCam)
    {
        return;
    }

    if (isExposed()) {
        if (!m_initialized) {
            double screenRatio = (double)width() / (double)height();
            wCam->initOrthographic(10.0, -512.0, 512.0, screenRatio);
            wCam->initPerspective(glm::radians(60.0), 0.125, 1024.0, screenRatio);

            initSurface();

            emit initializedCamera(&wCam, this);

            m_initialized = true;
            m_refreshViewport = true;
        }
        else {
            m_refreshViewport = true;
        }
    }
}

void VulkanViewport::focusOutEvent(QFocusEvent *_event)
{
    std::lock_guard<std::mutex> lockInput(m_inputMutex);
    // We cannot observe the key release so we reset them
    m_KI.reset();
    m_MI.reset();
    if (m_mouseInputEffect == MouseInputEffect::ObjectManipulation)
        m_actionToPull = Action::EndManipulation;
}

void VulkanViewport::mousePressEvent(QMouseEvent *_event)
{
    std::lock_guard<std::mutex> lock(m_inputMutex);

    emit activeViewport(this);
    switch (_event->button())
    {
    case Qt::LeftButton:
        m_MI.leftButtonPressed = true;
        break;
    case Qt::RightButton:
        m_MI.rightButtonPressed = true;
        break;
    case Qt::MiddleButton:
        m_MI.middleButtonPressed = true;
        break;
    default:
        break;
    }
}

void VulkanViewport::mouseReleaseEvent(QMouseEvent *_event)
{
    std::lock_guard<std::mutex> lock(m_inputMutex);
    if (!(m_MI.leftButtonPressed || m_MI.rightButtonPressed || m_MI.middleButtonPressed))
        return;

    // NOTE - Qt nous donne les modifiers via cette fonction.
    //Qt::KeyboardModifiers modif = _event->modifiers();

    switch (_event->button())
    {
    case Qt::LeftButton:
    {
        m_MI.leftButtonPressed = false;
        if (m_mouseInputEffect == MouseInputEffect::None)
        {
            m_actionToPull = Action::Click;
        }
        break;
    }
    case Qt::RightButton:
        m_MI.rightButtonPressed = false;
        break;
    case Qt::MiddleButton:
        m_MI.middleButtonPressed = false;
        break;
    default:
        break;
    }
}

void VulkanViewport::mouseMoveEvent(QMouseEvent *_event)
{
    std::lock_guard<std::mutex> lock(m_inputMutex);

    m_MI.deltaX += _event->pos().x() - m_MI.lastX;
    m_MI.deltaY += _event->pos().y() - m_MI.lastY;

    m_MI.lastX = _event->pos().x();
    m_MI.lastY = _event->pos().y();
}

void VulkanViewport::mouseDoubleClickEvent(QMouseEvent* _event)
{
    std::lock_guard<std::mutex> lock(m_inputMutex);

    m_MI.lastX = _event->pos().x();
    m_MI.lastY = _event->pos().y();

    if (_event->button() == Qt::LeftButton)
    {
        m_forceObjectCenterOnExamine = true;
        m_actionToPull = Action::Examine;
    }
    else
        m_actionToPull = Action::DoubleClick;
}

void VulkanViewport::keyPressEvent(QKeyEvent *_event)
{
    std::lock_guard<std::mutex> lock(m_inputMutex);

    // ### Camera controls ###
    // Take in account only the initial key press and discard the repeating key event.
    if (_event->isAutoRepeat())
        return;

    switch (_event->key()) {
    case Qt::Key_Up:
        m_KI.upKeyPressed = true;
        break;
    case Qt::Key_Down:
        m_KI.downKeyPressed = true;
        break;
    case Qt::Key_Left:
        m_KI.leftKeyPressed = true;
        break;
    case Qt::Key_Right:
        m_KI.rightKeyPressed = true;
        break;
    case Qt::Key_PageUp:
        m_KI.pageUpKeyPressed = true;
        break;
    case Qt::Key_PageDown:
        m_KI.pageDownKeyPressed = true;
        break;
    case Qt::Key_Shift:
        m_KI.shiftKeyPressed = true;
        break;
    case Qt::Key_Control:
        m_KI.controlKeyPressed = true;
        break;
    case Qt::Key_AltGr:
    case Qt::Key_Alt:
        m_KI.altKeyPressed = true;
        break;
    case Qt::Key_Plus:
        m_KI.plusKeyPressed = true;
        break;
    case Qt::Key_Minus:
        m_KI.minusKeyPressed = true;
        break;
    case Qt::Key_Slash:
        m_KI.slashKeyPressed = true;
        break;
    case Qt::Key_Asterisk:
        m_KI.asteriskKeyPressed = true;
        break;
    case Qt::Key_C:
    {
        ReadPtr<CameraNode> rCam = m_cam.cget(true);
        if (!rCam)
            break;

        const DisplayParameters& param = rCam->getDisplayParameters();
        m_dataDispatcher.updateInformation(new GuiDataRenderTransparency(BlendMode::Transparent, param.m_transparency, m_cam), this);
        break;
    }

    default:
        break;
    }
}

void VulkanViewport::keyReleaseEvent(QKeyEvent* _event)
{
    if (_event->isAutoRepeat())
        return;

    QWindow::keyReleaseEvent(_event);

    std::lock_guard<std::mutex> lock(m_inputMutex);


    // ### Camera controls ###
    switch (_event->key()) {
    case Qt::Key_Up:
        m_KI.upKeyPressed = false;
        break;
    case Qt::Key_Down:
        m_KI.downKeyPressed = false;
        break;
    case Qt::Key_Left:
        m_KI.leftKeyPressed = false;
        break;
    case Qt::Key_Right:
        m_KI.rightKeyPressed = false;
        break;
    case Qt::Key_PageUp:
        m_KI.pageUpKeyPressed = false;
        break;
    case Qt::Key_PageDown:
        m_KI.pageDownKeyPressed = false;
        break;
    case Qt::Key_Shift:
        m_KI.shiftKeyPressed = false;
        break;
    case Qt::Key_AltGr:
    case Qt::Key_Alt:
        m_KI.altKeyPressed = false;
        break;
    case Qt::Key_Control:
        m_KI.controlKeyPressed = false;
        break;
    case Qt::Key_Plus:
        m_KI.plusKeyPressed = false;
        break;
    case Qt::Key_Minus:
        m_KI.minusKeyPressed = false;
        break;
    case Qt::Key_Slash:
        m_KI.slashKeyPressed = false;
        break;
    case Qt::Key_Asterisk:
        m_KI.asteriskKeyPressed = false;
        break;
    case Qt::Key_C:
    {
        ReadPtr<CameraNode> rCam = m_cam.cget(true);
        if (!rCam)
            break;

        const DisplayParameters& param = rCam->getDisplayParameters();
        m_dataDispatcher.updateInformation(new GuiDataRenderTransparency(BlendMode::Opaque, param.m_transparency, m_cam), this);
        break;
    }
    }

    if (m_KI.altKeyPressed || m_KI.controlKeyPressed || m_KI.shiftKeyPressed)
        return;
    switch(_event->key())
    {
    case Qt::Key_N:
        emit showOverlay();
        break;
    case Qt::Key_F3:
        m_displayRenderStats = !m_displayRenderStats;
        break;
    default:
        break;
    }
}

void VulkanViewport::wheelEvent(QWheelEvent* _event)
{
    m_MI.wheel += m_navParams.wheelInverted ? _event->angleDelta().y() : -_event->angleDelta().y();
}

void VulkanViewport::updateMouseInputEffect(WritePtr<CameraNode>& wCam, SafePtr<ManipulatorNode>& manipNode)
{
    std::lock_guard<std::mutex> lock(m_inputMutex);

    bool hasMoved = (m_MI.deltaX != 0) || (m_MI.deltaY != 0);

    // Check if we quit the current input effect
    switch (m_mouseInputEffect)
    {
    case MouseInputEffect::Orientation:
    {
        if (!m_MI.leftButtonPressed)
            m_mouseInputEffect = MouseInputEffect::None;
        break;
    }
    case MouseInputEffect::Selection:
    {
        // Selection is done by the RenderingEngine when `m_selectionRect` is passed to the graph
        if (!m_MI.rightButtonPressed)
        {
            m_mouseInputEffect = MouseInputEffect::None;
            m_selectionRect = m_hoverRect;
            m_hoverRect = { {-1, -1}, {-1, -1} };
        }
        break;
    }
    case MouseInputEffect::Pan:
    {
        if (!m_MI.middleButtonPressed)
            m_mouseInputEffect = MouseInputEffect::None;
        break;
    }
    case MouseInputEffect::DeltaY:
    {
        if (!m_MI.leftButtonPressed && !m_MI.rightButtonPressed)
            m_mouseInputEffect = MouseInputEffect::None;
        break;
    }
    case MouseInputEffect::ObjectManipulation:
        if (!m_MI.leftButtonPressed)
        {
            m_mouseInputEffect = MouseInputEffect::None;
            m_actionToPull = Action::EndManipulation;
        }
        break;
    default:
        break;
    }

    bool hoverManip = ((m_hoveredId & RESERVED_DATA_ID_MASK) > 0) && m_hoveredId != INVALID_PICKING_ID;

    // Test if we must enter an input effect
    if (m_mouseInputEffect == MouseInputEffect::None)
    {
        if ((m_MI.leftButtonPressed && m_MI.rightButtonPressed && hasMoved))
        {
            quitPanoramic(wCam);
            m_mouseInputEffect = MouseInputEffect::DeltaY;
            return;
        }

        if (m_MI.leftButtonPressed && hoverManip)
        {
            m_mouseInputEffect = MouseInputEffect::ObjectManipulation;
            ManipulatorNode::setCurrentSelection(Selection(m_hoveredId & VALUE_DATA_ID_MASK), glm::ivec2(m_MI.lastX, m_MI.lastY), manipNode);
            return;
        }

        if (m_MI.leftButtonPressed && hasMoved)
        {
            m_mouseInputEffect = MouseInputEffect::Orientation;
            return;
        }

        if (m_MI.rightButtonPressed && hasMoved)
        {
            m_mouseInputEffect = MouseInputEffect::Selection;
            m_hoverRect = { { m_MI.lastX - m_MI.deltaX, m_MI.lastY - m_MI.deltaY }, { m_MI.lastX, m_MI.lastY } };
            return;
        }

        if (m_MI.middleButtonPressed && hasMoved)
        {
            quitPanoramic(wCam);
            // NOTES
            // - C'est la camera contient qui peut reset son point d'examun en fonction de ses options internes lors de moveLocal().
            // - La camera pourrait aussi reset le scan panoramic elle-même.
            m_mouseInputEffect = MouseInputEffect::Pan;
            return;
        }
    }
}

void VulkanViewport::applyMouseInput(WritePtr<CameraNode>& wCam, SafePtr<ManipulatorNode>& manipNode)
{
    std::lock_guard<std::mutex> lock(m_inputMutex);

    if (!wCam)
    {
        assert(false);
        return;
    }

    double speedRotationMult = m_mouseSensibility * calcRotationExamineSpeedFactor(m_navParams);
    double speedTranslationMult = calcTranslationSpeedFactor(m_navParams);

    switch (m_mouseInputEffect)
    {
    case MouseInputEffect::None:
    {
        switch (m_naviMode)
        {
        case NavigationMode::Explore:
        {
            double speed = m_KI.shiftKeyPressed ? 3. : m_KI.controlKeyPressed ? 0.01 : 0.5;
            double moveForward = (float)(-m_MI.wheel) / m_wheelSensibility * speed * speedTranslationMult;

            glm::dvec4 eyeCoord = wCam->getEyeCoord((double)(m_MI.lastX), (double)(m_MI.lastY),
                1., (double)width(), (double)height());
            glm::dvec4 pickPos = wCam->getModelMatrix() * eyeCoord;
            wCam->moveForwardToPoint(moveForward, { pickPos.x, pickPos.y, pickPos.z });

            break;
        }
        case NavigationMode::Examine:
        {
            wCam->moveAroundExamine(m_MI.wheel / m_wheelSensibility / 15., 0., 0.);
            break;
        }
        case NavigationMode::Orthographic:
        case NavigationMode::OrthoExamine:
        {
            double ratio = std::exp((double)m_MI.wheel / m_wheelSensibility * m_zoomRatio * speedTranslationMult);
            wCam->zoomOnPointOfInterest(getMousePosNormalized(), ratio, 0.3);
            break;
        }
        case NavigationMode::Panoramic:
        {
            double ratio = std::exp((double)m_MI.wheel / m_wheelSensibility * m_zoomRatio * speedTranslationMult);
            wCam->zoom(ratio, 0.3);
            break;
        }
        default:
            break;
        }
        break;
    }
    case MouseInputEffect::Orientation:
    {
        if ((wCam->getProjectionMode() == ProjectionMode::Perspective) &&
            (m_naviMode == NavigationMode::Explore || m_naviMode == NavigationMode::Panoramic))
        {
            double dTheta = m_MI.deltaX * speedRotationMult;
            if (m_navParams.mouseDragInverted) dTheta *= -1;
            wCam->yaw(dTheta);

            double dPhi = m_MI.deltaY * speedRotationMult;
            if (m_navParams.mouseDragInverted) dPhi *= -1;
            wCam->pitch(dPhi);
        }
        else if ((m_naviMode == NavigationMode::Examine || m_naviMode == NavigationMode::OrthoExamine))
        {
            double dTheta = -m_MI.deltaX * speedRotationMult;
            double dPhi = -m_MI.deltaY * speedRotationMult;
            wCam->moveAroundExamine(0., dTheta, dPhi);
        }
        break;
    }
    case MouseInputEffect::Selection:
    {
        m_hoverRect.c1 = { m_MI.lastX, m_MI.lastY };
        break;
    }
    case MouseInputEffect::Pan:
    {
        // TODO - save a "pan point" as a reference to pan

        double panSpeed = m_KI.shiftKeyPressed ? 0.06 : m_KI.controlKeyPressed ? 0.006 : 0.01;

        double height = wCam->getHeightAt1m();
        if ((m_naviMode == NavigationMode::OrthoExamine || m_naviMode == NavigationMode::Orthographic))
            panSpeed *= height * 0.125f;
        else
            panSpeed *= speedTranslationMult;

        double moveRight = -(double)(m_MI.deltaX) * panSpeed;
        double moveUp = (double)(m_MI.deltaY) * panSpeed;
        // NEW - the moveLocal() can reset automatically the examine point.
        //     - It depends on the internal camera option "keep examine on pan".
        wCam->moveLocal(0., moveRight, moveUp);
        break;
    }
    case MouseInputEffect::DeltaY:
    {
        switch (m_naviMode)
        {
        case NavigationMode::Explore:
        case NavigationMode::Panoramic:
        {
            double speed = m_KI.shiftKeyPressed ? 0.12 : m_KI.controlKeyPressed ? 0.001 : 0.01;
            double moveForward = -m_MI.deltaY * speed * speedTranslationMult;
            wCam->moveLocal(moveForward, 0., 0.);
            break;
        }
        case NavigationMode::Examine:
        {
            double moveForward = m_MI.deltaY / (60.0 * m_guiScale);
            // NOTE - Maintenant que le point d'examun est dans la camera, on peut utiliser moveLocal() sans distinction exterieure.
            wCam->moveAroundExamine(moveForward * speedTranslationMult, 0., 0.);
            break;
        }
        case NavigationMode::OrthoExamine:
        {
            double ratio = std::exp(m_MI.deltaY / (60.0 * m_guiScale));
            wCam->zoomOnPointOfInterest(getMousePosNormalized(), ratio, 0.3);
            break;
        }
        case NavigationMode::Orthographic:
        {
            double ratio = std::exp(m_MI.deltaY / (60.0 * m_guiScale));
            wCam->zoom(ratio, 0.1);
            break;
        }
        default:
            break;
        }
        break;
    }
    case MouseInputEffect::ObjectManipulation:
    {
        ManipulatorNode::updateEvent(manipNode, glm::ivec2(m_MI.lastX, m_MI.lastY), glm::ivec2(width(), height()), *&wCam);
        break;
    }
    default:
        break;
    }

    m_MI.resetDeltas();
}

void VulkanViewport::applyKeyboardInput(WritePtr<CameraNode>& wCam)
{
    std::lock_guard<std::mutex> lock(m_inputMutex);

    double leftRightSum(0.);
    double upDownSum(0.);
    double pageUpDownSum(0.);
    double speedTranslation(0.);

    double minusPlusSum(0.);
    double slashAsteriskSum(0.);
    double speedOrientation(0.);

    if (m_KI.altKeyPressed && m_naviMode == NavigationMode::Orthographic)
    {
        speedTranslation = 1.;
        leftRightSum -= (m_KI.metaLeft && !m_KI.leftKeyPressed ? 1.0 : 0.0);
        leftRightSum += (m_KI.metaRight && !m_KI.rightKeyPressed ? 1.0 : 0.0);
        leftRightSum *= wCam->getWidthAt1m();
        upDownSum -= (m_KI.metaDown && !m_KI.downKeyPressed ? 1.0 : 0.0);
        upDownSum += (m_KI.metaUp && !m_KI.upKeyPressed ? 1.0 : 0.0);
        upDownSum *= wCam->getHeightAt1m();
    }
    else
    {
        speedOrientation = (m_KI.shiftKeyPressed ? 2.f : m_KI.controlKeyPressed ? 0.33f : 1.f) * (M_PI / 300.f);
        speedOrientation *= calcRotationExamineSpeedFactor(m_navParams);
        speedTranslation = m_KI.shiftKeyPressed ? 20.f : m_KI.controlKeyPressed ? 0.5f : 2.f;
        speedTranslation *= calcTranslationSpeedFactor(m_navParams);

        leftRightSum = (double)m_KI.rightKeyPressed - (double)m_KI.leftKeyPressed;
        upDownSum = (double)m_KI.upKeyPressed - (double)m_KI.downKeyPressed;
        pageUpDownSum = (double)m_KI.pageUpKeyPressed - (double)m_KI.pageDownKeyPressed;

        minusPlusSum = (double)m_KI.minusKeyPressed - (double)m_KI.plusKeyPressed;
        slashAsteriskSum = (double)m_KI.slashKeyPressed - (double)m_KI.asteriskKeyPressed;
    }

    switch (m_naviMode)
    {
    case NavigationMode::Panoramic:
    {
        if (leftRightSum != 0 || upDownSum != 0 || pageUpDownSum != 0)
            quitPanoramic(wCam);
        // no break, continue the navigation as 'Explore'
    }
    case NavigationMode::Explore:
    {
        wCam->setSpeedRight(leftRightSum * speedTranslation);
        wCam->setSpeedForward(upDownSum * speedTranslation);
        wCam->setSpeedUp(pageUpDownSum * speedTranslation);

        wCam->yaw(slashAsteriskSum * speedOrientation);
        wCam->pitch(minusPlusSum * speedOrientation);
        break;
    }
    case NavigationMode::OrthoExamine:
    case NavigationMode::Examine:
    {
        double dRadius = -0.02 * pageUpDownSum;
        double dTheta = speedOrientation * leftRightSum;
        double dPhi = -speedOrientation * upDownSum;
        wCam->moveAroundExamine(dRadius, dTheta, dPhi);
        break;
    }
    case NavigationMode::Orthographic:
    {
        wCam->moveLocal(0.0, leftRightSum * speedTranslation, upDownSum * speedTranslation);

        wCam->yaw(slashAsteriskSum * speedOrientation);
        wCam->pitch(minusPlusSum * speedOrientation);
        break;
    }
    default:
        break;
    }
    m_KI.metaDown = m_KI.altKeyPressed && m_KI.downKeyPressed;
    m_KI.metaUp = m_KI.altKeyPressed && m_KI.upKeyPressed;
    m_KI.metaLeft = m_KI.altKeyPressed && m_KI.leftKeyPressed;
    m_KI.metaRight = m_KI.altKeyPressed && m_KI.rightKeyPressed;
}

void VulkanViewport::updateMouse3DInputs(const glm::dmat4& status)
{
    //position
    glm::dvec3 newPosition;
    for (uint16_t iterator(0); iterator < 3; iterator++)
        newPosition[iterator] = status[iterator][3];
    bool doRot(true);

    WritePtr<CameraNode> wCam = m_cam.get();
    if (!wCam)
        return;

    switch (m_naviMode)
    {
        case NavigationMode::Panoramic:
        {
            if (glm::distance2(newPosition, glm::dvec3(0.0)) > 0.0)
                quitPanoramic(wCam);
            // no break, continue the navigation as 'Explore'
        }
        case NavigationMode::Explore:
        {
            wCam->moveLocal(-newPosition.z, newPosition.x, newPosition.y);
            break;
        }
        case NavigationMode::OrthoExamine:
        case NavigationMode::Examine:
        {
            float dRadius = -newPosition.y;
            float dTheta = newPosition.z;
            float dPhi = newPosition.x;

            wCam->moveAroundExamine(dRadius, dTheta, dPhi);
            doRot = false;
            break;
        }
        case NavigationMode::Orthographic:
        {
            wCam->moveLocal(0.0, newPosition.x, newPosition.z);
            break;
        }
        default:
            break;
    }
    //rotation
    if (doRot)
    {
        // --- TODO - Tester de remplacer le code ci-dessous
        TransformationModule newTransfo(status);
        glm::dvec3 eulers(tls::math::quat_to_euler_zyx_rad(newTransfo.getOrientation()));
        eulers *= 0.5;
        if (!std::isnan(eulers.y))
            wCam->yaw(-eulers.y);
        if (!std::isnan(eulers.x))
            wCam->pitch(eulers.x);
        // --- par celui-ci
        //glm::dquat q = glm::quat_cast(status);
        //wCam->setRotation(q);
        // ---
    }
    m_3dMouseUpdate = true;
}

void VulkanViewport::updateRenderingNecessityState(WritePtr<CameraNode>& wCam, bool& refreshPCRender)
{
    if (!wCam)
    {
        assert(false);
        return;
    }

    refreshPCRender |= m_refreshViewport;
    refreshPCRender |= m_updateMissingScanPart;
    bool isNavigationOngoing = false;
    {
        isNavigationOngoing |= (m_mouseInputEffect == MouseInputEffect::Orientation);
        isNavigationOngoing |= (m_mouseInputEffect == MouseInputEffect::Pan);
        isNavigationOngoing |= (m_mouseInputEffect == MouseInputEffect::DeltaY);
        isNavigationOngoing |= (m_mouseInputEffect == MouseInputEffect::ObjectManipulation);
        isNavigationOngoing |= m_KI.leftKeyPressed;
        isNavigationOngoing |= m_KI.rightKeyPressed;
        isNavigationOngoing |= m_KI.upKeyPressed;
        isNavigationOngoing |= m_KI.downKeyPressed;
        isNavigationOngoing |= m_KI.pageUpKeyPressed;
        isNavigationOngoing |= m_KI.pageDownKeyPressed;
        isNavigationOngoing |= m_KI.asteriskKeyPressed;
        isNavigationOngoing |= m_KI.slashKeyPressed;
        isNavigationOngoing |= m_KI.plusKeyPressed;
        isNavigationOngoing |= m_KI.minusKeyPressed;
        isNavigationOngoing |= m_3dMouseUpdate;
        isNavigationOngoing |= m_MI.wheel != 0;
        isNavigationOngoing |= m_MI.rightButtonPressed;
    }

    // Keep the full render if some scan were missing
    m_updateScansFullRender = (m_previousRenderDecimated || m_updateMissingScanPart) && !isNavigationOngoing;
    refreshPCRender |= m_updateScansFullRender;

    // Reset local variables
    m_refreshViewport = false;
    m_updateMissingScanPart = false;
    m_3dMouseUpdate = false;

    // SPECIAL - Force Update
    refreshPCRender |= m_forceUpdate;
    m_updateScansFullRender |= m_forceUpdate;
}

bool VulkanViewport::compareFrameHash(uint64_t newFrameHash)
{
    uint64_t oldFrameHash = m_frameHash;
    m_frameHash = newFrameHash;
    return (m_frameHash != oldFrameHash);
}

// TODO - Replace the VulkanViewport by a meta{viewport + update variables}
//      - The update variables will be the timers, booleans, frame stats, etc
float VulkanViewport::decimatePointSize(bool refreshPCRender)
{
    // Do not decimate
    if (m_updateScansFullRender)
    {
        m_frameStack[m_frameStackIndex].decimation = 1.f;
        m_previousRenderDecimated = false;
        return 1.f;
    }
    else if (refreshPCRender)
    {
        float decimation_n0 = 1.f;
        switch (m_decimationOptions.mode)
        {
        case DecimationMode::None:
            decimation_n0 = 1.f;
            break;
        case DecimationMode::Constant:
            decimation_n0 = m_decimationOptions.constantValue;
            break;
        case DecimationMode::Adaptive:
        {
            int frame_n2 = (m_frameStackIndex - 2) % m_frameStack.size();
            int frame_n1 = (m_frameStackIndex - 1) % m_frameStack.size();
            float decimation_n2 = m_frameStack[frame_n2].decimation;
            float decimation_n1 = m_frameStack[frame_n1].decimation;
            float prevPrepT = m_frameStack[frame_n1].prepareScansTime + m_frameStack[frame_n1].prepareObjectsTime;
            float prevRenderT = m_frameStack[frame_n2].renderTime;


            // Detect which timer is limiting
            if (prevPrepT > 16.6f && prevPrepT > prevRenderT)
                decimation_n0 = decimation_n1 / (prevPrepT / 16.6f);
            else if (prevRenderT > 0.f)
                decimation_n0 = decimation_n2 / (prevRenderT / 16.6f);
            else
                decimation_n0 = m_lastAdaptiveDecimation;

            // Clamp the decimation value
            if (decimation_n0 < m_decimationOptions.dynamicMin)
                decimation_n0 = m_decimationOptions.dynamicMin;
            // Special condition to handle nan and infinity values
            if (!(decimation_n0 < 1.f))
                decimation_n0 = 1.f;
            break;
        }
        }

        m_lastAdaptiveDecimation = decimation_n0;
        m_previousRenderDecimated = (decimation_n0 != 1.f);
        m_frameStack[m_frameStackIndex].decimation = decimation_n0;
        return (decimation_n0);
    }
    return 1.f;
}

void VulkanViewport::recordFrameStats(const FrameStats& _stats)
{
    m_frameStackIndex = (m_frameStackIndex + 1) % m_frameStack.size();
    m_frameStack[m_frameStackIndex] = _stats;
}

uint64_t VulkanViewport::getPreviousPointDrawnCount() const
{
    int fi = (m_frameStackIndex - 1) % m_frameStack.size();
    return m_frameStack[fi].pointCount;
}

void VulkanViewport::setPreviousRenderTime(float totalRenderTime)
{
    int fi = (m_frameStackIndex - 1) % m_frameStack.size();
    m_frameStack[fi].renderTime = totalRenderTime;
}

void VulkanViewport::mousePointAction(bool examineAction)
{
    if (examineAction)
    {
        m_forceObjectCenterOnExamine = false;
        m_actionToPull = Action::Examine;
    }
    else
        m_actionToPull = Action::Click;
}
