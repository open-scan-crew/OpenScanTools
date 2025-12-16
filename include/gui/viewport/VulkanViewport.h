#ifndef VULKAN_VIEWPORT_H
#define VULKAN_VIEWPORT_H

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "gui/viewport/PickingManager.h"
#include "models/3d/NavigationTypes.h"
#include "models/3d/ClickInfo.h"
#include "pointCloudEngine/FrameStats.h"
#include "pointCloudEngine/RenderingTypes.h"

#include "glm/glm.hpp"

#include <mutex>
#include <unordered_map>

#include <QtGui/QWindow.h>

class CameraNode;



struct MouseInput
{
    int lastX = 0;
    int lastY = 0;
    int deltaX = 0;
    int deltaY = 0;
    int wheel = 0;
    bool leftButtonPressed = false;
    bool rightButtonPressed = false;
    bool middleButtonPressed = false;

    void resetDeltas()
    {
        deltaX = 0;
        deltaY = 0;
        wheel = 0;
    }

    void reset()
    {
        resetDeltas();
        leftButtonPressed = false;
        rightButtonPressed = false;
        middleButtonPressed = false;
    }
};

struct KeyboardInput
{
    bool leftKeyPressed = false;
    bool rightKeyPressed = false;
    bool upKeyPressed = false;
    bool downKeyPressed = false;
    bool pageUpKeyPressed = false;
    bool pageDownKeyPressed = false;
    bool shiftKeyPressed = false;
    bool controlKeyPressed = false;
    bool altKeyPressed = false;

    bool plusKeyPressed = false;
    bool minusKeyPressed = false;
    bool slashKeyPressed = false;
    bool asteriskKeyPressed = false;

    bool metaUp = false;
    bool metaLeft = false;
    bool metaDown = false;
    bool metaRight = false;

    void reset()
    {
        leftKeyPressed = false;
        rightKeyPressed = false;
        upKeyPressed = false;
        downKeyPressed = false;
        pageUpKeyPressed = false;
        pageDownKeyPressed = false;
        shiftKeyPressed = false;
        controlKeyPressed = false;
        altKeyPressed = false;

        plusKeyPressed = false;
        minusKeyPressed = false;
        slashKeyPressed = false;
        asteriskKeyPressed = false;
    }
};

class ManipulatorNode;
struct DecimationOptions;

class VulkanViewport : public QWindow, public IPanel
{
    Q_OBJECT

public:
    enum class MouseInputEffect
    {
        None,
        Orientation,
        Selection,
        Pan,
        DeltaY,
        ObjectManipulation
    };
    enum class Action {
        None,
        DoubleClick,
        Click,
        Examine,
        BeginManipulation,
        EndManipulation
    };

public:
    VulkanViewport(IDataDispatcher& dataDispatcher, float guiScale);
    ~VulkanViewport();

    void setCamera(SafePtr<CameraNode> camera);
    SafePtr<CameraNode> getCamera() const;
    NavigationMode getNavigationMode() const;
    TlFramebuffer getFramebuffer() const;
    Pos3D getPickingPos(const CameraNode& cam);
    std::unordered_set<uint32_t> getSelectedIds() const;
    std::unordered_set<uint32_t> getHoveredIds() const;
    void refreshHoveredId(uint32_t textId);
    Rect2D getSelectionRect() const;
    Rect2D getHoverRect() const;
    glm::ivec2 getMousePos() const;
    glm::vec2 getMousePosNormalized() const;
    void setMissingScanPart(bool isMissingScanPart);
    bool mustRenderFullScans();


    bool isInitialized() const;

    void updateInputs(WritePtr<CameraNode>& writeCamera, SafePtr<ManipulatorNode> manipNode, SafePtr<AGraphNode> hoveredNode);
    void updateMouse3DInputs(const glm::dmat4& status);
    void updateRenderingNecessityState(WritePtr<CameraNode>& writeCamera, bool& refreshPCRender);
    bool compareFrameHash(uint64_t newFrameHash);
    float decimatePointSize(bool refreshPCRender);
    void recordFrameStats(const FrameStats& stats);
    uint64_t getPreviousPointDrawnCount() const;
    void setPreviousRenderTime(float totalRenderTime);

    void mousePointAction(bool examineAction);

signals:
    void projectionModeChanged(ProjectionMode mode);
    void activeViewport(const VulkanViewport* viewport);
    void initializedCamera(CameraNode* camera, const VulkanViewport* viewport);
    void cameraPosition(double x, double y, double z, const VulkanViewport* viewport);
    void pickingPosition(double x, double y, double z, const VulkanViewport* viewport);
    void showOverlay();

public slots:
    // To test the rendering
    void slotForceUpdate(int state);

protected:
    // Override from APanel
    void informData(IGuiData* data) override;
    typedef void (VulkanViewport::* GuiDataFunction)(IGuiData*);
    inline void registerGuiDataFunction(guiDType type, GuiDataFunction fct)
    {
        m_dataDispatcher.registerObserverOnKey(this, type);
        m_functions.insert({ type, fct });
    };

    void onRenderNavigationParameters(IGuiData* data);
    void onRenderViewPointAnimation(IGuiData* data);  // FIXME - Move to CameraNode
    void onRenderAnimationSpeed(IGuiData* data); // FIXME - Move to CameraNode
    void onRenderAnimationLoop(IGuiData* data);  // FIXME - Move to CameraNode
    void onRenderStartAnimation(IGuiData* data);  // FIXME - Move to CameraNode
    void onRenderStopAnimation(IGuiData* data);  // FIXME - Move to CameraNode
    void onRenderCleanAnimationList(IGuiData* data);  // FIXME - Move to CameraNode
    void onUserOrientation(IGuiData* data); // FIXME - Move to CameraNode
    void onProjectOrientation(IGuiData* data); // FIXME - Move to CameraNode
    void onQuitEvent(IGuiData* data);
    void onRenderDecimationOptions(IGuiData* data);

private:
    void initSurface();

    void quitPanoramic(WritePtr<CameraNode>& wCam);
    void doAction(const WritePtr<CameraNode>& wCam, SafePtr<ManipulatorNode>& manipNode, SafePtr<AGraphNode>& hoverNode);

    // *** Events Override from QWindow ***
    bool event(QEvent *) override;

    // Window Event
    void resizeEvent(QResizeEvent *) override;
    void exposeEvent(QExposeEvent *) override;
    void focusOutEvent(QFocusEvent *) override;

    // Input Event
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
    void keyReleaseEvent(QKeyEvent *) override;
    void wheelEvent(QWheelEvent *) override;

    void updateMouseInputEffect(WritePtr<CameraNode>& wCam, SafePtr<ManipulatorNode>& manipNode);
    void applyMouseInput(WritePtr<CameraNode>& wCam, SafePtr<ManipulatorNode>& manipNode);
    void applyKeyboardInput(WritePtr<CameraNode>& wCam);

    void updateProjNaviMode(WritePtr<CameraNode>& wCam);

    ClickInfo generateRaytracingInfos(const WritePtr<CameraNode>& rCam, bool useObjectCenter = false);

public:
    // ******* Statistics ******* //
    bool m_displayRenderStats = false;
    uint32_t m_frameStackIndex = 0;
    std::array<FrameStats, 120> m_frameStack;
    float m_lastAdaptiveDecimation = 1.f;

private:
    IDataDispatcher& m_dataDispatcher;
    std::unordered_map<guiDType, GuiDataFunction> m_functions;

    //To maange cylcing and area "gaussian" selection
    PickingManager m_pickingManager;
    Action m_actionToPull;
    bool m_examineUseObjectCenter;

    // Window state
    bool m_initialized;
    bool m_isFullscreen;

    // Synchronization mutexes
    std::mutex m_inputMutex;
    std::mutex m_renderParamMutex;
    std::mutex m_dataMutex;

    // Navigation
    NavigationMode m_naviMode;
    NavigationParameters m_navParams = NavigationParameters();
    MouseInputEffect m_mouseInputEffect;

    // Inuput structure
    MouseInput m_MI;
    KeyboardInput m_KI;
    bool m_3dMouseUpdate;

    float m_guiScale;
    double m_mouseSensibility = 0.005; // rad/pixel for zoom = 1
    float m_wheelSensibility = 120.f;  // 
    float m_zoomRatio = 0.15f;

    // Multiselection
    Rect2D m_selectionRect;
    Rect2D m_hoverRect;
    Pos3D m_lastPicking;

    // Only used in the render loop --> synchronized
    DecimationOptions m_decimationOptions;
    bool m_refreshViewport = false;
    bool m_updateScansFullRender = false;
    bool m_previousRenderDecimated = false; // NEW //
    bool m_updateMissingScanPart = false;
    bool m_forceUpdate = false;
    uint64_t m_frameHash = 0;

    SafePtr<CameraNode> m_cam;

    TlFramebuffer m_framebuffer = TL_NULL_HANDLE;

    // Markers & Object hovered
    uint32_t m_hoveredId = INVALID_PICKING_ID;

    // Animation
    bool m_saveImagesAnim;
};

#endif // VULKAN_VIEWPORT_H_
