#include "gui/viewport/ViewportOrganizer.h"
#include "gui/viewport/VulkanViewport.h"
#include "gui/viewport/QuickBarNavigation.h"
#include "gui/IDataDispatcher.h"
#include "gui/ShortcutSystem.h"
#include "pointCloudEngine/IRenderingEngine.h"
#include "models/graph/CameraNode.h"
#include "gui/viewport/EventManagerViewport.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataContextRequest.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "controller/controls/ControlFunction.h"
#include "controller/controls/ControlViewport.h"
#include "controller/controls/ControlIO.h"
#include "controller/messages/CameraMessage.h"
#include "utils/Logger.h"

#include "gui/style/CustomCursors.h"

#include <QtWidgets/QVBoxLayout>

ViewportOrganizer::ViewportOrganizer(QWidget* parent, IDataDispatcher& dataDispatcher, IRenderingEngine& engine, ShortcutSystem& shortSys, float guiScale)
    : QWidget(parent)
    , m_dataDispatcher(dataDispatcher)
    , m_renderingEngine(engine)
    , m_guiScale(guiScale)
    , m_activeViewport()
    , m_fullScreenViewport()
    , m_shortSys(shortSys)
{
    m_layout = new QGridLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(2);
    setLayout(m_layout);

    // First viewport
    addViewport(0, 0, 1, 1);

    m_mouse.setViewportOrganizer(this);

    registerGuiDataFunction(guiDType::contextRequestCameraPosition, &ViewportOrganizer::onContextRequestActiveCamera);
    registerGuiDataFunction(guiDType::cursorChange, &ViewportOrganizer::onCursorChange);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::disableFullScreen);

    addShortcut(Qt::Key_F11, this, Qt::ShortcutContext::WindowShortcut, &ViewportOrganizer::onToggleFullScreen);

    addShortcut(Qt::Key_0, this, Qt::ShortcutContext::WindowShortcut, [this]() { onAdjustZoomToScene(); });
    addShortcut(Qt::Key_2, this, Qt::ShortcutContext::WindowShortcut, [this]() {onAlignView(AlignView::Bottom); });
    addShortcut(Qt::Key_3, this, Qt::ShortcutContext::WindowShortcut, [this]() {onAlignView(AlignView::Front); });
    addShortcut(Qt::Key_4, this, Qt::ShortcutContext::WindowShortcut, [this]() {onAlignView(AlignView::Left); });
    addShortcut(Qt::Key_6, this, Qt::ShortcutContext::WindowShortcut, [this]() {onAlignView(AlignView::Right); });
    addShortcut(Qt::Key_8, this, Qt::ShortcutContext::WindowShortcut, [this]() {onAlignView(AlignView::Top); });
    addShortcut(Qt::Key_9, this, Qt::ShortcutContext::WindowShortcut, [this]() {onAlignView(AlignView::Back); });

    addShortcut(Qt::Key_5, this, Qt::ShortcutContext::WindowShortcut, &ViewportOrganizer::onRotation90);

    addShortcut(Qt::Key_O, this, Qt::ShortcutContext::WindowShortcut, [this]() {onProjectionMode(ProjectionMode::Orthographic); });
    addShortcut(Qt::Key_P, this, Qt::ShortcutContext::WindowShortcut, [this]() {onProjectionMode(ProjectionMode::Perspective); });

    addShortcut(Qt::Key_X, this, Qt::ShortcutContext::WindowShortcut, [this]() {onMousePointAction(true); });
    addShortcut(Qt::Key_Space, this, Qt::ShortcutContext::WindowShortcut, [this]() {onMousePointAction(false); });

     addShortcut(Qt::Key_Enter, this, Qt::ShortcutContext::WindowShortcut, &ViewportOrganizer::onCancel);
     addShortcut(Qt::Key_Return, this, Qt::ShortcutContext::WindowShortcut, &ViewportOrganizer::onCancel);


    m_dataDispatcher.updateInformation(new GuiDataChangeCursor());
    //connect(new QShortcut(Qt::Key_Escape, this), &QShortcut::activated, this, &ViewportOrganizer::onDisableFullScreen);

}

ViewportOrganizer::~ViewportOrganizer()
{
    Logger::log(LoggerMode::VKLog) << "Destroying Viewport Organizer..." << Logger::endl;
}

void ViewportOrganizer::addViewport(int row, int column, int rowSpan, int columnSpan)
{
    MetaViewport meta;
    meta.mainWidget = new QWidget(this);
    meta.vulkanViewport = new VulkanViewport(m_dataDispatcher, m_guiScale);

    // Le viewports sont des QWindow, il leur faut un Widget container pour pouvoir les organiser comme des widgets.
    QWidget* wndWrapper = QWidget::createWindowContainer(meta.vulkanViewport, this);
    EventManagerViewport* eventManager = new EventManagerViewport(wndWrapper, m_dataDispatcher);
    wndWrapper->installEventFilter(eventManager);
    wndWrapper->setFocusPolicy(Qt::StrongFocus);
    wndWrapper->setFocus();

    // Quick Bar
    meta.quickBar = new QuickBarNavigation(this, m_dataDispatcher, m_guiScale);
    meta.quickBar->connectViewport(meta.vulkanViewport);

    QVBoxLayout* layout = new QVBoxLayout(meta.mainWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);
    layout->addWidget(meta.quickBar);
    layout->addWidget(wndWrapper);

    //connect picking & camera
    connect(meta.vulkanViewport, &VulkanViewport::cameraPosition, this, &ViewportOrganizer::onUpdateCameraPosition, Qt::QueuedConnection);
    connect(meta.vulkanViewport, &VulkanViewport::pickingPosition, this, &ViewportOrganizer::onUpdatePickingPosition, Qt::QueuedConnection);
    connect(meta.vulkanViewport, &VulkanViewport::activeViewport, this, &ViewportOrganizer::onActiveViewport, Qt::QueuedConnection);
    connect(meta.vulkanViewport, &VulkanViewport::initializedCamera, this, &ViewportOrganizer::onInitializedCamera, Qt::QueuedConnection);

    m_layout->addWidget(meta.mainWidget, row, column, rowSpan, columnSpan);

    // Register the IViewport to the main drawing Engine
    m_renderingEngine.registerViewport(meta.vulkanViewport);
    meta.quickBar->connectCamera(meta.vulkanViewport->getCamera());

    m_viewports.insert({ meta.vulkanViewport->getCamera(), meta }); 
    
    if (!m_activeViewport)
        m_activeViewport = meta.vulkanViewport->getCamera();
}

void ViewportOrganizer::addShortcut(QKeySequence key, QWidget* parent, Qt::ShortcutContext context, std::function<void()> action)
{
    QShortcut* shortcut = createShortcut(key, parent, context);
    connect(shortcut, &QShortcut::activated, this, action);
    m_shortSys.addShortcut(shortcut);
}

void ViewportOrganizer::addShortcut(QKeySequence key, QWidget* parent, Qt::ShortcutContext context, ViewportOrganiserFunction action)
{
    QShortcut* shortcut = createShortcut(key, parent, context);
    connect(shortcut, &QShortcut::activated, this, action);
    m_shortSys.addShortcut(shortcut);
}

QShortcut* ViewportOrganizer::createShortcut(QKeySequence key, QWidget* parent, Qt::ShortcutContext context)
{
    QShortcut* shortcut = new QShortcut(parent);
    shortcut->setKey(key);
    shortcut->setContext(context);
    return shortcut;
}

void ViewportOrganizer::informData(IGuiData* data)
{
    if (m_functions.find(data->getType()) != m_functions.end())
    {
        GuiDataFunction fct = m_functions.at(data->getType());
        (this->*fct)(data);
    }
    else if (data->getType() == guiDType::disableFullScreen)
        onDisableFullScreen();
}

void ViewportOrganizer::onContextRequestActiveCamera(IGuiData* data)
{
    GuiDataContextRequestActiveCamera* dataCast(static_cast<GuiDataContextRequestActiveCamera*>(data));

    VulkanViewport* vp = nullptr;
    if (m_viewports.find(m_activeViewport) != m_viewports.end())
        vp = m_viewports.at(m_activeViewport).vulkanViewport;
    else
        return;

    CameraMessage* message = new CameraMessage(vp->getCamera());
    m_dataDispatcher.sendControl(new control::function::ForwardMessage(message, dataCast->m_contextId));
}

void ViewportOrganizer::onCursorChange(IGuiData* data)
{
    QCursor c = cursor();
    Qt::CursorShape shape = static_cast<GuiDataChangeCursor*>(data)->m_shape;
    if (shape == Qt::CursorShape::CrossCursor)
    {
        QBitmap B, M;
        scs::giveCrossCursorBitmaps(B, M);
        QCursor newCursor(B, M);
        c.swap(newCursor);
    }
    else
        c.setShape(static_cast<GuiDataChangeCursor*>(data)->m_shape);

    for (auto viewports : m_viewports)
        viewports.second.vulkanViewport->setCursor(c);
}

void ViewportOrganizer::onAdjustZoomToScene()
{
    if (m_viewports.find(m_activeViewport) != m_viewports.end())
        m_dataDispatcher.sendControl(new control::viewport::AdjustZoomToScene(m_viewports[m_activeViewport].vulkanViewport->getCamera()));
}

void ViewportOrganizer::onAlignView(AlignView align)
{
    if (m_viewports.find(m_activeViewport) != m_viewports.end())
    {
        WritePtr<CameraNode> wCam = m_viewports[m_activeViewport].vulkanViewport->getCamera().get();
        wCam->alignView(align);
    }
}

void ViewportOrganizer::onRotation90()
{
    if (m_viewports.find(m_activeViewport) != m_viewports.end())
    {
        WritePtr<CameraNode> wCam = m_viewports[m_activeViewport].vulkanViewport->getCamera().get();
        if (!wCam)
            return;
        wCam->rotate90degrees();
    }
}

void ViewportOrganizer::onProjectionMode(ProjectionMode proj)
{
    if (m_viewports.find(m_activeViewport) != m_viewports.end())
    {
        WritePtr<CameraNode> wCam = m_viewports.at(m_activeViewport).vulkanViewport->getCamera().get();
        if (!wCam)
            return;
        wCam->setProjectionMode(proj);
    }
}

void ViewportOrganizer::onMousePointAction(bool examineAction)
{
    if (m_viewports.find(m_activeViewport) != m_viewports.end())
        m_viewports[m_activeViewport].vulkanViewport->mousePointAction(examineAction);
}

void ViewportOrganizer::onActiveViewport(const VulkanViewport* viewport)
{
    if (viewport->getCamera() != m_activeViewport)
    {
        m_activeViewport = viewport->getCamera();
        if (viewport->isInitialized())
        {
            m_dataDispatcher.updateInformation(new GuiDataFocusViewport(viewport->getCamera(), viewport->width(), viewport->height(), true));

            // ---- TODO ----
            // * Distinguer l’initialisation de la camera (chargement du projet) 
            //   de la mise à jour des paramètres UI dépendant de la camera/viewpoint (changement de camera active)
            // * GuiDataLoadViewPoint(camId, ViewPoint) OU directement un CameraNode::setViewPoint()
            // * GuiDataChangeActiveCamera(camId, DisplayParameters)
            //     L-> PAS de projection, panoramic, 3dObject, ... dont n’a pas besoin la GUI

            m_dataDispatcher.updateInformation(new GuiDataCameraInfo(viewport->getCamera()));
        }
    }
}

void ViewportOrganizer::onInitializedCamera(CameraNode* camera, const VulkanViewport* viewport)
{
    if (viewport->getCamera() == m_activeViewport)
    {
        m_dataDispatcher.updateInformation(new GuiDataFocusViewport(viewport->getCamera(), viewport->width(), viewport->height(), true));
        m_dataDispatcher.updateInformation(new GuiDataCameraInfo(viewport->getCamera()), camera);
    }
}

void ViewportOrganizer::onUpdateCameraPosition(double x, double y, double z, const VulkanViewport* viewport)
{
    if(viewport->getCamera() == m_activeViewport)
        emit cameraPos(x, y, z);
}

void ViewportOrganizer::onUpdatePickingPosition(double x, double y, double z, const VulkanViewport* viewport)
{
    if (viewport->getCamera() == m_activeViewport)
        emit picking(x, y, z);
}

void ViewportOrganizer::onToggleFullScreen()
{
    if (m_fullScreenViewport)
        onDisableFullScreen();
    else
        onEnableFullScreen();
}

void ViewportOrganizer::onEnableFullScreen()
{
    if (m_viewports.find(m_activeViewport) != m_viewports.end())
    {
        // On cherche le MetaViewport qui contient le VulkanViewport actif.
        MetaViewport& metaVP(m_viewports.at(m_activeViewport));

        // On cherche la position du widget dans le layout principal.
        // On sauvegarde cette position pour pouvoir le replacer en sortie du mode fullscreen.
        bool layoutPosFound = false;
        for (int i = 0; i < m_layout->count(); ++i)
        {
            QLayoutItem* item = m_layout->itemAt(i);
            QWidget* widget = item->widget();
            if (widget == metaVP.mainWidget)
            {
                m_layout->getItemPosition(i, &row, &col, &rowSpan, &colSpan);
                layoutPosFound = true;
                break;
            }
        }

        // On détache le méta viewport
        if (metaVP.mainWidget != nullptr && layoutPosFound)
        {
            metaVP.mainWidget->setParent(nullptr);
            metaVP.mainWidget->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
            metaVP.mainWidget->showFullScreen();
            metaVP.quickBar->onFullScreenMode(true);
            m_shortSys.changeParent(metaVP.mainWidget);
        }
        m_fullScreenViewport = m_activeViewport;
    }
}

void ViewportOrganizer::onDisableFullScreen()
{
    if (m_viewports.find(m_fullScreenViewport) != m_viewports.end())
    {
        MetaViewport& metaVP = m_viewports[m_fullScreenViewport];
        metaVP.mainWidget->setParent(this);
        metaVP.quickBar->onFullScreenMode(false);
        // On replace le widget détaché à l'endroit où il a était détaché du layout.
        m_layout->addWidget(metaVP.mainWidget, row, col, rowSpan, colSpan);
        m_fullScreenViewport.reset();
        m_shortSys.changeParent(this);
    }
}

void ViewportOrganizer::onCancel()
{
    GUI_LOG << "shortcut cancel" << LOGENDL;
    m_dataDispatcher.sendControl(new control::function::Validate());
}

void ViewportOrganizer::mouse3DUpdate(const glm::dmat4& status) const
{
    if (m_viewports.find(m_activeViewport) != m_viewports.end())
        m_viewports.at(m_activeViewport).vulkanViewport->updateMouse3DInputs(status);
}

glm::dmat4 ViewportOrganizer::getCurrentViewMatrix() const
{
    if (m_viewports.find(m_activeViewport) != m_viewports.end())
        return glm::dmat4(1.0);

    ReadPtr<CameraNode> rCam = m_viewports.at(m_activeViewport).vulkanViewport->getCamera().cget();
    return rCam->getViewMatrix();
}

SafePtr<CameraNode> ViewportOrganizer::getActiveCameraNode()
{
    if (m_viewports.find(m_activeViewport) != m_viewports.end())
    {
        if (!m_viewports.empty())
            return m_viewports.begin()->second.vulkanViewport->getCamera();
        else
            return SafePtr<CameraNode>();
    }
    return m_viewports.at(m_activeViewport).vulkanViewport->getCamera();
}
