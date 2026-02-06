#ifndef CAMERA_NODE_H
#define CAMERA_NODE_H

#include "models/graph/AGraphNode.h"
#include "models/3d/NavigationTypes.h"
#include "models/3d/BoundingBox.h"
#include "models/application/UserOrientation.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "vulkan/VkUniform.h"

#include "models/3d/RenderingParameters.h"
#include "models/3d/ColorimetricFilterUniform.h"

#include <deque>

class ViewPointNode;
class PointCloudNode;
struct UniformClippingData;

struct KeyPoint {
    glm::dvec3 point;
    double theta;
    double phi;
    // relative time in the trajectory
    double dtime_arrival;
};

// NEW struct for the KeyPoint with quaternion
//struct 

struct SimpleAnimation {
    KeyPoint start;
    KeyPoint end;
};

enum class InterpolationValueWithPreviousAnimation { NONE, BEZIER/*, LINEAR*/ };

class CameraNode : public AGraphNode, public IPanel, public RenderingParameters
{
private:
    enum class AnimationMode { Simple, Complex/*, HorizontalExamine*/ };

public:
    CameraNode(const std::wstring& name, IDataDispatcher& dataDispatcher);
    CameraNode(const CameraNode& camera);
    //CameraNode(const UIViewPoint& viewPoint, ...); // ??
    ~CameraNode();

    Type getGraphType() const;
    SafePtr<PointCloudNode> getPanoramicScan() const;
    void resetPanoramicScan();

    void informData(IGuiData* guiData) override;

    // Rendering matrices
    void prepareUniforms(uint32_t swapIndex);
    void uploadClippingUniform(const std::vector<UniformClippingData>& clippingTransfos, uint32_t swapIndex) const;
    VkUniformOffset getViewProjUniform(uint32_t swapIndex) const;
    VkUniformOffset getViewUniform(uint32_t swapIndex) const;
    VkUniformOffset getInversedViewUniform(uint32_t swapIndex) const;
    VkUniformOffset getClippingUniform(uint32_t swapIndex) const;
    VkUniformOffset getColorimetricFilterUniform(uint32_t swapIndex) const;

    //LockHorizontal animation
    void checkAndHorizontalAnimation();

    glm::dmat4 getInverseLocalViewMatrix() const;

    void refresh();

    bool updateAnimation();
    bool isAnimated() const;

    glm::dvec3 getViewAxis() const; // or front axis ?
    void adjustToScene(const BoundingBoxD& sceneBBox);
    void alignView(AlignView align);
    //Simple animation
    void lookAt(glm::dvec3 lookPoint, double dt_sec);
    void lookAt(double endTheta, double endPhi, double dt_sec);
    void translateTo(glm::dvec3 endPoint, double dt_sec);

    //Complex animation

    void AddViewPoint(SafePtr<ViewPointNode> vp, const InterpolationValueWithPreviousAnimation& interpolation = InterpolationValueWithPreviousAnimation::NONE);
    void AddViewPoint1(SafePtr<ViewPointNode> vp, const InterpolationValueWithPreviousAnimation& interpolation = InterpolationValueWithPreviousAnimation::NONE);
    bool startAnimation(const bool& isOffline = false, const uint64_t& step = 1000);
    bool endAnimation();
    void cleanAnimation();
    void setSpeed(const int& speed);
    void setLoop(const bool& loop);

    // Orientation as Euler angles
    void yaw(double _amount);
    void pitch(double _amount);
    double getTheta() const;
    double getPhi() const;

    // Constraints and User orientation
    bool getApplyConstraint() const;
    NaviConstraint getNaviConstraint() const;

    bool getApplyUserOrientation() const;
    const UserOrientation& getUserOrientation() const;
    const glm::dvec3& getLargeCoordinatesCorrection() const;

    // Display Modifiers
    void setMarkerShowMask(MarkerShowMask mask);

    // Modifiers
    void setThetaAndPhi(const double& theta, const double& phi);

    void setApplyUserOrientation(bool useUo);
    void setUserOrientation(const UserOrientation& orientation);

    void setSpeedForward(double amount);
    void setSpeedRight(double amount);
    void setSpeedUp(double amount);

    // Instant modifiers
    void moveLocal(double forward, double right, double up);
    void moveForwardToPoint(double forward, glm::dvec3 pickingPos);
    void moveGlobal(double x, double y, double z);
    void moveAroundExamine(double dRadius, double dTheta, double dPhi);
    void rotate90degrees();


    // Examine Mode
    bool isExamineActive() const;
    void setExaminePoint(const glm::dvec3& position);
    void resetExaminePoint();

    glm::dmat4 getViewMatrix() const;
    glm::dmat4 getGlobalViewMatrix() const;
    glm::dmat4 getModelMatrix() const;

    glm::dvec3 getScreenProjection(glm::dvec3 point, glm::ivec2 screenSize) const;
    void getScreenToViewDirection(glm::dvec2 screenPos, glm::ivec2 screenSize, glm::dvec3& linePoint, glm::dvec3& lineDir) const;

    // Helper function
    static void modulo2Pi(double ref_angle, double& modified_angle);
    static void clampPhi(double& phi);
    static glm::dvec3 sphericToCartesian(double r, double theta, double phi);
    static glm::dvec3 sphericToCartesian_withDeport(double r, double theta, double phi, double deportTheat, double deportPhi);

    //FROM Camera
    void initPerspective(double fovy, double nearZ, double farZ, double screenRatio);
    void initOrthographic(double realH, double nearZ, double farZ, double screenRatio);

    void setScreenRatio(int width, int height);
    void setProjectionMode(ProjectionMode _mode);
    void setProjectionFrustum(double left, double right, double bottom, double top, double near, double far);
    void zoom(double amount, double dtime);
    void zoomOnPointOfInterest(glm::dvec2 mouse_position, double amount, double dtime);
    void setOrthoZoom(double width, double height, const glm::dvec3& endPos, double duration);
    void resetPerspectiveZoom();
    void setFovy(float fovy, bool saveBackup = true);
    void setOrthoHeight(double height);

    void moveToData(const SafePtr<AGraphNode>& data);

    // Draw camera
    glm::vec3 getExamineTargetPosition() const;

private:
    bool integrityCheck();
    // Vulkan resources
    void allocAllUniforms();
    void freeAllUniforms();

    void startPlayTrajectory(const uint64_t& animationStep);
    bool animateSimpleTrajectory();
    bool animateComplexTrajectory();

    void applyProjection(const ProjectionData& projectionData);

    void computeDeportAngles(const glm::dvec3& centralPoint, double theta, double phi, double& depTheta, double& depPhi);

    void moveTo(glm::dvec3 _endPoint, double _stopDist, double _dt_sec);
    void moveTo(glm::dvec3 _endPoint, double _dt_sec, glm::dvec3 _lookDir, glm::dvec3 _upDir);
    void moveTo(const glm::dvec3& endPoint, double endTheta, double endPhi, double dt_sec);

    // Compute the the trajectory from linear interpolation of the path points.
    // The path points come from an Bezier/Polynomial interpolation.
    // REWORK - The speed must not be constant but each control point should have a timer
    //        - We should keep a dynamic interpolation from the control points
    void getKeyPointsFromPath(const std::vector<glm::dvec3>& pathPoints, double speed);

    // BEZIER
    void updateTrajectoryToBezier();
    static void computeBezierPath(const std::vector<glm::dvec3>& controlPoints, std::vector<glm::dvec3>& pathPoints, double distanceCriterion);
    static void computeBezierStep(const std::vector<glm::dvec3>& keyPoints, std::vector<std::vector<glm::dvec3>>& T);
    static double computeMaxDistanceBetweenKeyPoints(const std::vector<glm::dvec3>& keyPoints);

    // POLYNOMIAL
    void updateTrajectoryToPolynomial();
    void computePolynomialPath(const std::vector<glm::dvec3>& keyPoints, std::vector<glm::dvec3>& T);

    void extractAngles(const glm::dvec3& _origin, const glm::dvec3& _lookPoint, double& _theta, double& _phi) const;
    void AddViewPoint(glm::dvec3 _point, double _meterPerSec, double _radPerSec);

    // Update Observer
    void sendNewUIViewPoint();
    //void addCallback();

    //GuiData
    typedef void (CameraNode::* GuiDataFunction)(IGuiData*);
    inline void registerGuiDataFunction(guiDType type, GuiDataFunction fct)
    {
        m_dataDispatcher.registerObserverOnKey(this, type);
        m_functions.insert({ type, fct });
    };
    void onRenderUnitUsage(IGuiData* data);
    void onRenderColorMode(IGuiData* data);
    void onRenderPointSize(IGuiData* data);
    void onRenderTexelThreshold(IGuiData* data);
    void onRenderBrightness(IGuiData* data);
    void onRenderContrast(IGuiData* data);
    void onRenderLuminance(IGuiData* data);
    void onRenderSaturation(IGuiData* data);
    void onRenderBlending(IGuiData* data);
    void onRenderTransparency(IGuiData* data);
    void onRenderTransparencyOptions(IGuiData* data);
    void onRenderFlatColor(IGuiData* data);
    void onRenderDistanceRampValues(IGuiData* data);
    void onRenderMeasureOptions(IGuiData* data);
    void onRenderDisplayObjectTexts(IGuiData* data);
    void onRenderTextFilter(IGuiData* data);
    void onRenderTextTheme(IGuiData* data);
    void onRenderTextFontSize(IGuiData* data);
    void onRenderMarkerDisplayOptions(IGuiData* data);
    void onRenderAlphaObjects(IGuiData* data);
    void onRenderNormals(IGuiData* data);
    void onRenderAmbientOcclusion(IGuiData* data);
    void onRenderEdgeAwareBlur(IGuiData* data);
    void onRenderDepthLining(IGuiData* data);
    void onRenderRampScale(IGuiData* data);
    void onRenderColorimetricFilter(IGuiData* data);
    void onBackgroundColor(IGuiData* data);
    void onAdjustZoomToScene(IGuiData* data);
    void onRenderCameraMoveTo(IGuiData* data);
    void onRenderRotateCamera(IGuiData* data);
    void onRenderExamine(IGuiData* data);
    void onExamineOptions(IGuiData* data);
    void onMoveToData(IGuiData* data);
    void onCameraToViewPoint(IGuiData* data);
    void onRenderFov(IGuiData* data);
    void onDisplayGizmo(IGuiData* data);
    void onRenderExamineTarget(IGuiData* data);
    void onRenderNavigationConstraint(IGuiData* data);
    void onRenderApplyNavigationConstraint(IGuiData* data);
    void onRenderNavigationParameters(IGuiData* data);
    void onRenderPerspectiveZ(IGuiData* data);
    void onRenderOrthographicZ(IGuiData* data);
    ColorimetricFilterUniform buildColorimetricFilterUniform() const;

private:
    IDataDispatcher& m_dataDispatcher;

    SafePtr<PointCloudNode> m_panoramicScan;

    //GuiData
    std::unordered_map<guiDType, GuiDataFunction> m_functions;

    // Sauvegarde des ProjectionFrustums pour le changement de mode
    ProjectionData m_savedCameraParams[(size_t)ProjectionMode::MAX_ENUM];

    //Animation & mouvement
    double m_speed = 0.0;
    bool m_isAnimated = false;
    bool m_isOfflineRendering = false;
    bool m_loop = false;
    AnimationMode m_animMode = AnimationMode::Simple;

    std::deque<SafePtr<ViewPointNode>> m_animation;
    std::deque<SafePtr<ViewPointNode>> m_initialAnimation;
    std::vector<KeyPoint> m_trajectory;
    //from camera
    uint64_t m_currentKeyPoint = 0;
    uint64_t m_animFrames = 0;
    double m_offlineAnimStep = 0.0;
    std::chrono::steady_clock::time_point m_startTrajectoryTime;
    SimpleAnimation m_simpleAnimation = SimpleAnimation();

    // Uniform for projection and view matrix (separated)
    VkMultiUniform m_uniProjView;
    // Uniform view Matrix 
    VkMultiUniform m_uniView;
    // Uniform inversed view matrix
    VkMultiUniform m_uniInversedView;
    // Uniform for the clipping and ramp matrices, extra ramp parameters
    VkMultiUniform m_uniClipping;
    // Uniform for colorimetric filter data
    VkMultiUniform m_uniColorimetricFilter;

    // ++++++++++++++++++ View ++++++++++++++++++++
    glm::dvec3 m_forward = glm::dvec3();
    glm::dvec3 m_right = glm::dvec3();
    glm::dvec3 m_up = glm::dvec3();
    glm::dvec3 m_deltaLocalPos = glm::dvec3();
    glm::dvec3 m_deltaGlobalPos = glm::dvec3();
    double m_speedForward = 0.0;
    double m_speedRight = 0.0;
    double m_speedUp = 0.0;

    bool m_centeringExamine = true;
    bool m_keepExamineOnPan = false;
    glm::dvec3 m_examinePoint = glm::dvec3(NAN, NAN, NAN);

    // When working with the Euler angles we choose
    // -> First rotation around Oz
    // -> Second rotation around Ox
    // -> Third rotation around Oz (disabled for navigation convenience)
    // At angle (0, 0, 0) the camera is lookin toward Oz with the Oy axis downward and Ox on the right

    NaviConstraint m_constraint = NaviConstraint();
    NavigationParameters m_navParams = NavigationParameters();
    UserOrientation m_uo = UserOrientation();
    double m_decalThetaUO = 0.0;
    bool m_applyConstraint = false;
    bool m_useUo = false;

    glm::dvec3 m_largeCoordinatesCorrection = glm::dvec3();
    glm::dmat4 m_viewMatrix = glm::dmat4();

    std::chrono::steady_clock::time_point m_prev_time = std::chrono::steady_clock::time_point();

    // Animation
    bool m_animateProj = false;
    std::chrono::steady_clock::time_point m_start_zoom = std::chrono::steady_clock::time_point();
    glm::dvec3 m_startPos = glm::dvec3();
    glm::dvec3 m_endPos = glm::dvec3();
    double m_startFovy = 1.0;
    double m_endFovy = 1.0;
    double m_startRealH = 1.0;
    double m_endRealH = 1.0;
    double m_durationAnimZoomSec = 0.0;
};

#endif //! CAMERA_NODE_H_
