#include "models/graph/CameraNode.h"
#include "models/graph/ScanNode.h"
#include "models/graph/ScanObjectNode.h"
#include "models/graph/BeamBendingMeasureNode.h"
#include "models/graph/ColumnTiltMeasureNode.h"
#include "models/graph/PipeToPipeMeasureNode.h"
#include "models/graph/PipeToPlaneMeasureNode.h"
#include "models/graph/PointToPipeMeasureNode.h"
#include "models/graph/PointToPlaneMeasureNode.h"
#include "models/graph/SimpleMeasureNode.h"
#include "models/graph/PolylineMeasureNode.h"
#include "models/graph/ViewPointNode.h"
#include "models/3d/UniformClippingData.h"

#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "controller/controls/ControlViewPoint.h"
#include "controller/controls/ControlFunction.h"

#include "controller/messages/GeneralMessage.h"

#include "pointCloudEngine/RenderingLimits.h"
#include "services/MarkerDefinitions.hpp"

#include "vulkan/VulkanManager.h"

#include "utils/Logger.h"
#include "utils/math/trigo.h"
#include "utils/math/glm_extended.h"

CameraNode::CameraNode(const std::wstring& name, IDataDispatcher& dataDispatcher)
    : AGraphNode()
    , m_dataDispatcher(dataDispatcher)
    , m_speed(1.0)
{
    m_name = name;
    m_quaternion = tls::math::euler_zxz_to_quat(glm::dvec3(1.5 * M_PI, -0.5 * M_PI, 0.0));
    Logger::log(LoggerMode::DataLog) << "Sizeof CameraNode: " << sizeof(CameraNode) << Logger::endl;

    allocAllUniforms();

    registerGuiDataFunction(guiDType::renderBackgroundColor, &CameraNode::onBackgroundColor);
    registerGuiDataFunction(guiDType::renderValueDisplay, &CameraNode::onRenderUnitUsage);

    registerGuiDataFunction(guiDType::renderColorMode, &CameraNode::onRenderColorMode);
    registerGuiDataFunction(guiDType::renderPointSize, &CameraNode::onRenderPointSize);
    registerGuiDataFunction(guiDType::renderBrightness, &CameraNode::onRenderBrightness);
    registerGuiDataFunction(guiDType::renderContrast, &CameraNode::onRenderContrast);
    registerGuiDataFunction(guiDType::renderLuminance, &CameraNode::onRenderLuminance);
    registerGuiDataFunction(guiDType::renderSaturation, &CameraNode::onRenderSaturation);
    registerGuiDataFunction(guiDType::renderBlending, &CameraNode::onRenderBlending);
    registerGuiDataFunction(guiDType::renderTransparency, &CameraNode::onRenderTransparency);
    registerGuiDataFunction(guiDType::renderTransparencyOptions, &CameraNode::onRenderTransparencyOptions);
    registerGuiDataFunction(guiDType::renderFlatColor, &CameraNode::onRenderFlatColor);
    registerGuiDataFunction(guiDType::renderDistanceRampValues, &CameraNode::onRenderDistanceRampValues);
    registerGuiDataFunction(guiDType::renderMeasureOptions, &CameraNode::onRenderMeasureOptions);
    registerGuiDataFunction(guiDType::renderTextFilter, &CameraNode::onRenderTextFilter);
    registerGuiDataFunction(guiDType::renderTextTheme, &CameraNode::onRenderTextTheme);
    registerGuiDataFunction(guiDType::renderTextFontSize, &CameraNode::onRenderTextFontSize);
    registerGuiDataFunction(guiDType::renderDisplayAllMarkersTexts, &CameraNode::onRenderDisplayObjectTexts);
    registerGuiDataFunction(guiDType::renderMarkerDisplayOptions, &CameraNode::onRenderMarkerDisplayOptions);
    registerGuiDataFunction(guiDType::renderAlphaObjectsRendering, &CameraNode::onRenderAlphaObjects);
    registerGuiDataFunction(guiDType::renderPostRenderingNormals, &CameraNode::onRenderNormals);
    registerGuiDataFunction(guiDType::renderRampScale, &CameraNode::onRenderRampScale);
    registerGuiDataFunction(guiDType::renderFovValueChanged, &CameraNode::onRenderFov);
    registerGuiDataFunction(guiDType::renderExamine, &CameraNode::onRenderExamine);
    registerGuiDataFunction(guiDType::examineOptions, &CameraNode::onExamineOptions);
    registerGuiDataFunction(guiDType::moveToData, &CameraNode::onMoveToData);
    registerGuiDataFunction(guiDType::renderActiveCamera, &CameraNode::onCameraToViewPoint);
    registerGuiDataFunction(guiDType::renderGuizmo, &CameraNode::onDisplayGizmo);
    registerGuiDataFunction(guiDType::renderAdjustZoom, &CameraNode::onAdjustZoomToScene);
    registerGuiDataFunction(guiDType::renderCameraMoveTo, &CameraNode::onRenderCameraMoveTo);
    registerGuiDataFunction(guiDType::renderRotateThetaPhiView, &CameraNode::onRenderRotateCamera);
    registerGuiDataFunction(guiDType::renderNavigationConstraint, &CameraNode::onRenderNavigationConstraint);
    registerGuiDataFunction(guiDType::renderApplyNavigationConstraint, &CameraNode::onRenderApplyNavigationConstraint);
    registerGuiDataFunction(guiDType::renderNavigationParameters, &CameraNode::onRenderNavigationParameters);
    //Near-far distances
    registerGuiDataFunction(guiDType::renderPerspectiveZ, &CameraNode::onRenderPerspectiveZ);
    registerGuiDataFunction(guiDType::renderOrthographicZ, &CameraNode::onRenderOrthographicZ);
}

CameraNode::CameraNode(const CameraNode& camera)
    : AGraphNode(camera)
    , RenderingParameters(camera)
    , m_dataDispatcher(camera.m_dataDispatcher)
    , m_panoramicScan(camera.m_panoramicScan)
{
    allocAllUniforms();
}

CameraNode::~CameraNode()
{
    m_dataDispatcher.unregisterObserver(this);
    freeAllUniforms();
}

AGraphNode::Type CameraNode::getGraphType() const
{
    return Type::Camera;
}
/*
UIViewPoint CameraNode::getUIViewPoint() const
{
    // Note(Robin) sur la copie profonde
    // * UIObject3D est une classe abstraite
    // * CameraNode n’hérite pas de UIObject3D mais possède 2 héritages en commun (TransformationModule, Data)
    // * On doit remonter un cran plus haut dans l’héritage pour reconstruire un UIViewPoint à partir de { ViewPointData, TransformationModule, Data }.
    // * La partie UIColorData reste avec les valeurs par défaut.
    return UIViewPoint(ViewPointData(*this, m_panoramicScan), *this, *this);
}*/

SafePtr<ScanNode> CameraNode::getPanoramicScan() const
{
    if (m_isAnimated)
        return SafePtr<ScanNode>();
    return m_panoramicScan;
}

void CameraNode::resetPanoramicScan()
{
    m_panoramicScan.reset();
}

void CameraNode::applyProjection(const ProjectionData& projectionData)
{
    double oldRatio = getRatioW_H();
    static_cast<ProjectionData&>(*this) = projectionData;
    ProjectionData::setRatioW_H(oldRatio);
}

// *************************************
//         Rendering matrices
// *************************************

void CameraNode::prepareUniforms(uint32_t swapIndex)
{
    glm::mat4 view = getViewMatrix();
    glm::mat4 proj = getProjMatrix();
    glm::mat4 projView = proj * view;
    glm::mat4 inversedView = glm::inverse(view);

    VulkanManager& vkM = VulkanManager::getInstance();

    vkM.loadUniformData(sizeof(glm::mat4), &view, 0, swapIndex, m_uniProjView);
    vkM.loadUniformData(sizeof(glm::mat4), &proj, sizeof(glm::mat4), swapIndex, m_uniProjView);
    vkM.loadUniformData(sizeof(glm::mat4), &projView, 2 * sizeof(glm::mat4), swapIndex, m_uniProjView);
    vkM.loadUniformData(sizeof(glm::mat4), &view, 0, swapIndex, m_uniView);
    vkM.loadUniformData(sizeof(glm::mat4), &inversedView, 0, swapIndex, m_uniInversedView);
}

void CameraNode::uploadClippingUniform(const std::vector<UniformClippingData>& uniformData, uint32_t swapIndex) const
{
    VulkanManager& vkm = VulkanManager::getInstance();

    if (uniformData.size() > 0)
        vkm.loadUniformData((uint32_t)uniformData.size() * sizeof(UniformClippingData), uniformData.data(), 0, swapIndex, m_uniClipping);
}

VkUniformOffset CameraNode::getViewProjUniform(uint32_t swapIndex) const
{
    return m_uniProjView[swapIndex];
}

VkUniformOffset CameraNode::getViewUniform(uint32_t swapIndex) const
{
    return m_uniView[swapIndex];
}

VkUniformOffset CameraNode::getInversedViewUniform(uint32_t swapIndex) const
{
    return m_uniInversedView[swapIndex];
}

VkUniformOffset CameraNode::getClippingUniform(uint32_t swapIndex) const
{
    return m_uniClipping[swapIndex];
}

// *************************************
//         Computing matrices
// *************************************

glm::dmat4 CameraNode::getInverseLocalViewMatrix() const
{
    return getModelMatrix();
}

bool CameraNode::updateAnimation()
{
    if (!m_isAnimated)
        return false;
    switch (m_animMode)
    {
    case AnimationMode::Simple:
        animateSimpleTrajectory();
        break;
    case AnimationMode::Complex:
        animateComplexTrajectory();
        break;
    }

    integrityCheck();
    return true;
}

glm::dvec3 CameraNode::getViewAxis() const
{
    return (glm::mat3_cast(m_quaternion) * glm::dvec3(0, 0, 1));
}

// adjust the camera to view the entirety of the scene
void CameraNode::adjustToScene(const BoundingBoxD& sceneBBox)
{
    if (!sceneBBox.isValid())
        return;

    if (getProjectionMode() == ProjectionMode::Orthographic)
    {
        // rotate the bounding box in the camera space
        glm::dmat3 matrix = glm::mat3_cast(glm::conjugate(getRotation()));
        BoundingBoxD rot_bbox = sceneBBox.rotate(matrix);
        glm::dvec3 size = rot_bbox.size();
        setOrthoZoom(size.x, size.y, sceneBBox.center(), 1.5);
    }
    else
    {
        // new position = center - r * view_axis
        glm::dvec3 new_pos = sceneBBox.center() - glm::length(sceneBBox.size()) * getViewAxis();
        translateTo(new_pos, 1.2);
    }
    return;
}

void CameraNode::alignView(AlignView align)
{
    UserOrientation uo = getUserOrientation();
    bool useUo = getApplyUserOrientation();

    double theta = useUo ? uo.getAngle() : 0.0;
    double phi = 0.0;

    switch (align)
    {
    case AlignView::Top:
    {
        theta += 0.0;
        phi = -M_PI;
    }
    break;
    case AlignView::Bottom:
    {
        theta += 0.0;
        phi = 0.0;
    }
    break;
    case AlignView::Front:
    {
        theta += 0.0;
        phi = -M_PI / 2.0;
    }
    break;
    case AlignView::Back:
    {
        theta += M_PI;
        phi = -M_PI / 2.0;
    }
    break;
    case AlignView::Left:
    {
        theta += 3 * M_PI / 2;
        phi = -M_PI / 2.0;
    }
    break;
    case AlignView::Right:
    {
        theta += M_PI / 2;
        phi = -M_PI / 2;
    }
    break;
    case AlignView::Iso:
    {
        //Utilise la troncature pour calculer l'angle théta (avec un décalage) sur les différentes diagonales
        theta += ceil((getTheta()) / (M_PI / 2))*(M_PI / 2) - M_PI / 4;
        phi = (getPhi() > -M_PI / 2 ? atan2(1, sqrt(2)) - M_PI / 2 : -atan2(1, sqrt(2)) - M_PI / 2);
    }
    break;
    }
    lookAt(theta, phi, 1.2);
    //setThetaAndPhi(theta, phi);

    if (align == AlignView::Iso)
    {
        // Inform the viewport and the quickbar
        //TODO INFORM m_dataDispatcher.updateInformation(new GuiDataRenderProjectionMode(ProjectionMode::Orthographic, null_tlid/*m_id*/), this);
        setProjectionMode(ProjectionMode::Orthographic);
    }
}

bool CameraNode::isAnimated() const
{
    return m_isAnimated;
}

void CameraNode::lookAt(glm::dvec3 _lookPoint, double _dt_sec)
{
    if (std::isnan(_lookPoint.x))
        return;

    // Extract the CameraNode angles from the current position and the look-at point
    glm::dvec3 dir = glm::normalize(_lookPoint - m_center);

    double endTheta = atan2(-dir.x, dir.y);
    double normXY = sqrt(dir.x * dir.x + dir.y * dir.y);
    double endPhi = atan2(-normXY, dir.z);

    modulo2Pi(getTheta(), endTheta);

    m_simpleAnimation = { { m_center, getTheta(), getPhi(), (double)std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()).time_since_epoch().count() },
                            { m_center, endTheta, endPhi, _dt_sec * 1000.0} };
    m_isAnimated = true;
}

void CameraNode::lookAt(double endTheta, double endPhi, double dt_sec)
{
    modulo2Pi(getTheta(), endTheta);

    m_simpleAnimation = { { m_center, getTheta(), getPhi(), (double)std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()).time_since_epoch().count() },
                            { m_center, endTheta, endPhi, dt_sec * 1000.0} };
    m_isAnimated = true;
}

void CameraNode::translateTo(glm::dvec3 _endPoint, double _dt_sec)
{
    if (std::isnan(_endPoint.x))
        return;
    _endPoint = glm::dvec3(glm::dvec4(_endPoint, 1.0));

    m_simpleAnimation = { { m_center, getTheta(), getPhi(), (double)std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()).time_since_epoch().count() },
                            { _endPoint, getTheta(), getPhi(), _dt_sec * 1000.0} };
    m_isAnimated = true;
}

void CameraNode::moveTo(glm::dvec3 _endPoint, double _stopDist, double _dt_sec)
{
    if (std::isnan(_endPoint.x))
        return;

    if (_endPoint == m_center)
        return;

    // set theta and phi of the destination
    glm::dvec3 direction = _endPoint - m_center;

    double endTheta = atan2(-direction.x, direction.y);
    double normXY = sqrt(direction.x * direction.x + direction.y * direction.y);
    double endPhi = atan2(-normXY, direction.z);
    glm::dvec3 stopPoint = _endPoint - _stopDist * glm::normalize(direction);

    modulo2Pi(getTheta(), endTheta);

    m_simpleAnimation = { { m_center, getTheta(), getPhi(), (double)std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()).time_since_epoch().count() },
                            { stopPoint, endTheta, endPhi, _dt_sec * 1000.0} };
    m_isAnimated = true;
}

/* \brief
  Move the CameraNode with an animation (_dt_sec > 0) to _endPoint.
  Change the orientation angles to _lookDir.
  Indicate an up direction if the lookDir is along Oz
*/
void CameraNode::moveTo(glm::dvec3 _endPoint, double _dt_sec, glm::dvec3 _lookDir, glm::dvec3 _upDir)
{
    double endTheta;
    if (_lookDir.x == 0.0 && _lookDir.y == 0.0)   // Must we change the test to x < epsilon ?
        endTheta = atan2(-_upDir.x, _upDir.y);
    else
        endTheta = atan2(-_lookDir.x, _lookDir.y);

    double normXY = sqrt(_lookDir.x * _lookDir.x + _lookDir.y * _lookDir.y);
    double endPhi = atan2(-normXY, _lookDir.z);

    modulo2Pi(getTheta(), endTheta);

    m_simpleAnimation = { { m_center, getTheta(), getPhi(), (double)std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()).time_since_epoch().count() },
                            { _endPoint, endTheta, endPhi, _dt_sec * 1000.0} };
    m_isAnimated = true;
}

void CameraNode::moveTo(const glm::dvec3& endPoint, double endTheta, double endPhi, double dt_sec)
{
    modulo2Pi(getTheta(), endTheta);

    m_simpleAnimation = { { m_center, getTheta(), getPhi(), (double)std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()).time_since_epoch().count() },
                            { endPoint, endTheta, endPhi, dt_sec * 1000.0} };
    m_isAnimated = true;
}

// ~ ~ ~ ~ ~
// return false if the animation is already finished
// return true if the animation is ongoing (even if it is finished on this frame)
// ~ _ ’ . -
bool CameraNode::animateSimpleTrajectory()
{
    if (!m_isAnimated)
        return false;
    double dt = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()).time_since_epoch().count() - m_simpleAnimation.start.dtime_arrival;
    double progress = dt / m_simpleAnimation.end.dtime_arrival;
    if (progress > 1.0 || !m_simpleAnimation.end.dtime_arrival)
    {
        m_center = m_simpleAnimation.end.point;
        if (!m_animation.empty())
        {
            //Note(Aurélien) #363 do stuff
            ReadPtr<ViewPointNode> rVp = m_animation.begin()->cget();
            static_cast<DisplayParameters&>(*this) = *&rVp;
            applyProjection(*&rVp);
            m_quaternion = rVp->getOrientation();
            m_dataDispatcher.sendControl(new control::viewpoint::UpdateStatesFromViewpoint(*m_animation.begin()));
            m_dataDispatcher.sendControl(new control::function::ForwardMessage(new GeneralMessage(GeneralInfo::ANIMATIONEND), ContextType::exportVideoHD));
            m_dataDispatcher.updateInformation(new GuiDataCameraInfo(SafePtr<CameraNode>()));
            m_animation.clear();
        }
        else
            setThetaAndPhi(m_simpleAnimation.end.theta, m_simpleAnimation.end.phi);
        m_isAnimated = false;
    }
    else
    {
        if (isnan(m_simpleAnimation.end.point.x))
        {
            m_isAnimated = false;
            return m_isAnimated;
        }
        m_center = m_simpleAnimation.end.point * progress + m_simpleAnimation.start.point * (1 - progress);
        setThetaAndPhi(m_simpleAnimation.end.theta * progress + m_simpleAnimation.start.theta * (1 - progress),
            m_simpleAnimation.end.phi * progress + m_simpleAnimation.start.phi * (1 - progress));
    }
    return m_isAnimated;
}

void CameraNode::AddViewPoint(SafePtr<ViewPointNode> vp, const InterpolationValueWithPreviousAnimation& interpolation)
{
    //ViewPoint _context(context);
    //_context.setPosition(m_geometricParent->getTransformation() * glm::dvec4(context.getCenter(), 1.0));

    ReadPtr<ViewPointNode> vpnode = vp.cget();
    if (!vpnode)
        return;

    m_animation.push_back(vp);
    m_initialAnimation.push_back(vp);

    m_trajectory.push_back({ vpnode->getCenter(), 3.0 * (double)M_PI / 2.0, (double)-M_PI / 2.0, 0.0 });
}

void CameraNode::AddViewPoint1(SafePtr<ViewPointNode> vp, const InterpolationValueWithPreviousAnimation& interpolation)
{
    //m_animation.push_back(contexte);

    ReadPtr<ViewPointNode> vpnode = vp.cget();
    if (!vpnode)
        return;

    m_trajectory.push_back({ vpnode->getCenter(), 3.0 * (double)M_PI / 2.0, (double)-M_PI / 2.0, 0.0 });
}


bool CameraNode::startAnimation(const bool& isOffline, const uint64_t& step)
{
    m_animMode = AnimationMode::Complex;
    m_isOfflineRendering = isOffline;
    if (m_animation.size() == 0)
    {
        m_isAnimated = false;
        return true;
    }
    // TODO - Update the values in the UI.

    bool first(true);
    m_trajectory.clear();
    for (const SafePtr<ViewPointNode>& vp : m_animation)
    {
        ReadPtr<ViewPointNode> node = vp.cget();
        if (!node)
            continue;
        //Note (aurélien) quick fix from viewpointTransfert
        float speed = 1.0f / node->getScale().x;
        if (first)
        {
            first = false;
            speed = 1.0f;
        }
        //AddViewPoint(animation, InterpolationValueWithPreviousAnimation::BEZIER);
        AddViewPoint1(vp, InterpolationValueWithPreviousAnimation::BEZIER);
    }
    startPlayTrajectory(step);
    m_isAnimated = true;
    return true;
}

bool CameraNode::endAnimation()
{
    if (m_isAnimated)
    {
        m_animation = m_initialAnimation;
        m_isAnimated = false;
        return true;
    }
    return false;
}

void CameraNode::cleanAnimation()
{
    m_animation.clear();
    m_initialAnimation.clear();
}

void CameraNode::setSpeed(const int& speed)
{
    m_speed = speed ? speed * 1.2f : 1.0f;
}

void CameraNode::setLoop(const bool& loop)
{
    m_loop = loop;
}

constexpr glm::dvec3 VIEW_FORWARD(0.0, 0.0, 1.0);
constexpr glm::dvec3 VIEW_RIGHT(1.0, 0.0, 0.0);
constexpr glm::dvec3 VIEW_UP(0.0, -1.0, 0.0);

void CameraNode::yaw(double _amount)
{
    if (m_applyConstraint && m_constraint == NaviConstraint::LockVertical)
        return;
    if (_amount == 0.0)
        return;

    glm::dvec3 euler = tls::math::quat_to_euler_zxz(m_quaternion);
    euler.x += _amount;
    m_quaternion = tls::math::euler_zxz_to_quat(euler);
}

void CameraNode::pitch(double _amount)
{
    if (_amount == 0.0)
        return;
    if (m_applyConstraint && m_constraint == NaviConstraint::LockHorizontal) {
        return;
    }

    glm::dvec3 euler = tls::math::quat_to_euler_zxz(m_quaternion);
    euler.y += _amount;
    clampPhi(euler.y);
    m_quaternion = tls::math::euler_zxz_to_quat(euler);
}

double CameraNode::getTheta() const
{
    glm::dvec3 euler = tls::math::quat_to_euler_zxz(m_quaternion);
    return euler.x;
}

double CameraNode::getPhi() const
{
    glm::dvec3 euler = tls::math::quat_to_euler_zxz(m_quaternion);
    return euler.y;
}

bool CameraNode::getApplyConstraint() const
{
    return m_applyConstraint;
}

NaviConstraint CameraNode::getNaviConstraint() const
{
    return m_constraint;
}

bool CameraNode::getApplyUserOrientation() const
{
    return m_useUo;
}

const UserOrientation& CameraNode::getUserOrientation() const
{
    return m_uo;
}

const glm::dvec3& CameraNode::getLargeCoordinatesCorrection() const
{
    return m_largeCoordinatesCorrection;
}

void CameraNode::setMarkerShowMask(MarkerShowMask mask)
{
    m_markerMask = mask;
    // refreshScanRender -> NO
    // TODO - notify gui observers
}

void CameraNode::setThetaAndPhi(const double& theta, const double& phi)
{
    glm::dvec3 euler(theta, phi, 0.0);
    modulo2Pi(0.0, euler.y); // m_phi C [-Pi, Pi]
    if (euler.y > 0.0)
    {
        euler.y = -euler.y;
        euler.x += M_PI;
    }
    m_quaternion = tls::math::euler_zxz_to_quat(euler);
}

void CameraNode::computeDeportAngles(const glm::dvec3& centralPoint, double theta, double phi, double& depTheta, double& depPhi)
{
    if (glm_extended::isnan(centralPoint) || (centralPoint == m_center))
    {
        depPhi = 0.0;
        depTheta = 0.0;
        return;
    }

    glm::dvec3 vecPoint = glm::normalize(centralPoint - m_center);

    double f = glm::dot(m_forward, vecPoint);
    double u = glm::dot(m_up, vecPoint);
    double r = glm::dot(m_right, vecPoint);

    if (f != 0.0 || u != 0.0) //atan2(0,0) is undefined
        depPhi = atan2(u, f);
    else
        depPhi = 0.0;

    glm::dvec3 forward_deportPhi = sphericToCartesian(1, theta, phi + depPhi); //We recreate the m_forward with a rotation depPhi

    depTheta = (signbit(r) ? -1 : 1) * glm_extended::angleBetweenDV3(forward_deportPhi, vecPoint);

    assert(!isnan(depTheta) && !isnan(depPhi));
}

void CameraNode::setApplyUserOrientation(bool useUo)
{
    m_useUo = useUo;
    if (!useUo) m_decalThetaUO = 0.0;

}

void CameraNode::setUserOrientation(const UserOrientation& orientation)
{
    m_uo = orientation;
    m_decalThetaUO = m_useUo ? orientation.getAngle() : 0.0;
}

void CameraNode::setSpeedForward(double _amount)
{
    m_speedForward = _amount;
}

void CameraNode::setSpeedRight(double _amount)
{
    m_speedRight = _amount;
}

void CameraNode::setSpeedUp(double _amount)
{
    m_speedUp = _amount;
}

void CameraNode::moveLocal(double forward, double right, double up)
{
    if (forward == 0. && right == 0. && up == 0.)
        return;

    if ((right != 0. || up != 0.) && !m_keepExamineOnPan)
        resetExaminePoint();

    // Note - Navigations constraint will be applied in the refresh function
    m_deltaLocalPos += glm::dvec3(forward, right, up);
}

void CameraNode::moveForwardToPoint(double move, glm::dvec3 pos)
{
    if (move == 0.)
        return;

    glm::dvec3 dir = glm::normalize(pos - m_center);

    m_deltaGlobalPos += move * dir;
}

void CameraNode::moveGlobal(double x, double y, double z)
{
    if (x == 0.0 && y == 0.0 && z == 0.0)
        return;
    m_center += glm::dvec3(x, y, z);
}

void CameraNode::moveAroundExamine(double _dRadius, double _dTheta, double _dPhi)
{
    if (glm_extended::isnan(m_examinePoint))
        return;
    if (_dRadius == 0.0 && _dTheta == 0.0 && _dPhi == 0.0)
        return;

    double deportTheta = 0.0;
    double deportPhi = 0.0;
    glm::dvec3 euler = tls::math::quat_to_euler_zxz(m_quaternion);
    computeDeportAngles(m_examinePoint, euler.x, euler.y, deportTheta, deportPhi);

    glm::dvec3 localPos = m_center - m_examinePoint;
    if (!(m_applyConstraint && m_constraint == NaviConstraint::LockVertical)) {
        euler.x += _dTheta;
        modulo2Pi(0.0, euler.x);
    }

    if (!(m_applyConstraint && m_constraint == NaviConstraint::LockZValue)) {
        euler.y += _dPhi;
        clampPhi(euler.y);
    }

    if (m_applyConstraint && m_constraint == NaviConstraint::LockHorizontal) {
        euler.y = -M_PI / 2;
    }

    // compute new position
    double localRadius = glm::length(localPos);

    // TODO  
    if (!(m_applyConstraint && m_constraint == NaviConstraint::LockZValue))
        localRadius *= std::exp(_dRadius);

    double minRadius = std::max(m_navParams.examineMinimumRadius, 0.01);
    if (localRadius < minRadius)
        localRadius = minRadius;

    localPos = sphericToCartesian_withDeport(localRadius, euler.x, euler.y, deportTheta, deportPhi);
    m_center = m_examinePoint + localPos;
    m_quaternion = tls::math::euler_zxz_to_quat(euler);
}

void CameraNode::rotate90degrees()
{
    if (isExamineActive())
        moveAroundExamine(0.0, -M_PI / 2, 0.0);
    else
        setThetaAndPhi(getTheta() - M_PI / 2, getPhi());
}

bool CameraNode::isExamineActive() const
{
    return (!std::isnan(m_examinePoint.x) &&
            !std::isnan(m_examinePoint.y) &&
            !std::isnan(m_examinePoint.z));
}

void CameraNode::resetExaminePoint()
{
    m_examinePoint = glm::dvec3(NAN, NAN, NAN);
    sendNewUIViewPoint();
}

// When refreshing the position and Global and local matrices the operations are done in this order:
// 1. Advance the position based on the current speed, previous directions and time elapsed,
// 2. Refresh the toGlobal matrix with the new position,
// 3. Refresh the global directions,
// 4. Refresh the view matrix.
void CameraNode::refresh()
{
    // Stop the chrono here
    std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
    double dTimeSec = std::chrono::duration<double>(current_time - m_prev_time).count();
    m_prev_time = current_time;
    glm::dvec3 oldPosition = m_center;

    if (m_animateProj)
    {
        double dTimeZoom = std::chrono::duration<double, std::ratio<1>>(current_time - m_start_zoom).count();
        double progress = dTimeZoom / m_durationAnimZoomSec;
        if (m_projectionMode == ProjectionMode::Perspective)
        {
            if (progress > 1.0)
            {
                ProjectionData::setFovy(m_endFovy);
                m_animateProj = false;
            }
            else
            {
                ProjectionData::setFovy(m_startFovy * (1.0 - progress) + m_endFovy * progress);
            }
        }
        else if (m_projectionMode == ProjectionMode::Orthographic)
        {
            if (progress > 1.0)
            {
                setOrthoHeight(m_endRealH);
                CameraNode::setPosition(m_endPos);
                m_animateProj = false;
            }
            else
            {
                setOrthoHeight(m_startRealH * (1.0 - progress) + m_endRealH * progress);
                CameraNode::setPosition(m_startPos * (1.0 - progress) + m_endPos * progress);
            }
        }

        sendNewUIViewPoint();
    }

    m_center += dTimeSec * (m_speedForward * m_forward + m_speedRight * m_right);

    m_center += m_deltaLocalPos.x * m_forward + m_deltaLocalPos.y * m_right + m_deltaLocalPos.z * m_up;
    m_center += m_deltaGlobalPos;

    // TODO - Intégrer les user orientation (Ajout temporaire)
    if (m_applyConstraint)
    {
        double cosUO = cos(m_decalThetaUO);
        double sinUO = sin(m_decalThetaUO);

        double deplX = m_center.x - oldPosition.x;
        double deplY = m_center.y - oldPosition.y;

        switch (m_constraint) {
        case NaviConstraint::LockXValue:
        {
            m_center.x -= (deplX * cosUO + deplY * sinUO) * cosUO;
            m_center.y -= (deplX * cosUO + deplY * sinUO) * sinUO;
        }
        break;
        case NaviConstraint::LockYValue:
        {
            m_center.x -= (deplY * cosUO - deplX * sinUO) * (-sinUO);
            m_center.y -= (deplY * cosUO - deplX * sinUO) * cosUO;
        }
        break;
        case NaviConstraint::LockZValue:
            m_center.z = oldPosition.z;
            break;
        default:
            break;
        }
    }

    m_center += dTimeSec * (m_speedUp * glm::dvec3(0.0, 0.0, 1.0));

    m_deltaLocalPos = glm::dvec3();
    m_deltaGlobalPos = glm::dvec3();

    // ***** Refresh global directions *****
    glm::dmat3 rotationToGlobal = glm::mat3_cast(m_quaternion);
    m_forward = rotationToGlobal * VIEW_FORWARD;
    m_right = rotationToGlobal * VIEW_RIGHT;
    m_up = rotationToGlobal * VIEW_UP;

    // Compute the large coordinates with a threshold
    m_largeCoordinatesCorrection = -(glm::round(m_center / 1024.0) * 1024.0);

    // Translate(-position)
    m_viewMatrix = glm::dmat4(1.0);
    m_viewMatrix[3][0] -= m_center.x + m_largeCoordinatesCorrection.x;
    m_viewMatrix[3][1] -= m_center.y + m_largeCoordinatesCorrection.y;
    m_viewMatrix[3][2] -= m_center.z + m_largeCoordinatesCorrection.z;

    glm::dmat4 inverseRotation = glm::mat4_cast(glm::conjugate(m_quaternion));

    // Refresh the View Matrix
    m_viewMatrix = inverseRotation * m_viewMatrix;
}

glm::dmat4 CameraNode::getViewMatrix() const
{
    return m_viewMatrix;
}

glm::dmat4 CameraNode::getModelMatrix() const
{
    // NOTE - The camera does not use the geometric parent provided by the AGraphNode
    // This matrix is used without the large coordinates correction because all opérations stays in double.
    glm::dmat4 modelMat = glm::mat4_cast(m_quaternion);
    modelMat[3][0] = m_center.x;
    modelMat[3][1] = m_center.y;
    modelMat[3][2] = m_center.z;
    return modelMat;
}

glm::dvec3 CameraNode::getScreenProjection(glm::dvec3 point, glm::ivec2 screenSize) const
{
    glm::dmat4 transfo = getProjMatrix() * m_viewMatrix;

    glm::dvec4 result = transfo * glm::dvec4(point, 1.0);
    result /= result.w;

    return glm::dvec3((result.x + 1.0) / 2.0 * screenSize.x, (result.y + 1.0) / 2.0 * screenSize.y, result.z);
}

void CameraNode::getScreenToViewDirection(glm::dvec2 screenPos, glm::ivec2 screenSize, glm::dvec3& linePoint, glm::dvec3& lineDir) const
{
    glm::dvec3 viewPos_0 = getEyeCoord(screenPos.x, screenPos.y, 0.0, screenSize.x, screenSize.y);
    glm::dvec3 viewPos_1 = getEyeCoord(screenPos.x, screenPos.y, 1.0, screenSize.x, screenSize.y);
    glm::dvec3 viewDir = viewPos_1 - viewPos_0;
    viewDir /= length(viewDir);

    glm::dmat4 model = getModelMatrix();
    linePoint = model * glm::dvec4(viewPos_0, 1.0);
    lineDir = model * glm::dvec4(viewDir, 0.0);
}

void CameraNode::modulo2Pi(double ref_angle, double& modified_angle)
{
    assert(!std::isnan(ref_angle) || !std::isnan(modified_angle));
    //int div2Pi = (int)floorl((modified_angle - ref_angle)/ (2.0 * M_PI));
    //modified_angle -= div2Pi * (2.0 * M_PI);
    while ((modified_angle - ref_angle) > (double)M_PI)
        modified_angle -= (double)(2 * M_PI);

    while ((modified_angle - ref_angle) < (double)-M_PI)
        modified_angle += (double)(2 * M_PI);
}

inline void CameraNode::clampPhi(double& phi)
{
    if (phi < -M_PI)
        phi = -M_PI;
    if (phi > 0)
        phi = 0;
}

glm::dvec3 CameraNode::sphericToCartesian(double r, double theta, double phi)
{
    double x = r * sin(-phi) * sin(-theta);
    double y = r * sin(-phi) * cos(theta);
    double z = r * cos(phi);

    return glm::dvec3(x, y, z);
}

glm::dvec3 CameraNode::sphericToCartesian_withDeport(double r, double theta, double phi, double depTheta, double depPhi)
{
    double x = -r * (sin(depTheta) * cos(theta) + cos(depTheta) * sin(-theta) * sin(-phi - depPhi));
    double y = -r * (sin(depTheta) * sin(theta) + cos(depTheta) * cos(theta) * sin(-phi - depPhi));
    double z = -r * cos(depTheta) * cos(phi + depPhi);

    assert(!isnan(x) && !isnan(y) && !isnan(z));
    return glm::dvec3(x, y, z);
}

void CameraNode::checkAndHorizontalAnimation()
{
    if (!getApplyConstraint() || getNaviConstraint() != NaviConstraint::LockHorizontal || m_isAnimated)
        return;

    if (isExamineActive())
    {
        glm::dvec3 examinePos = m_examinePoint + sphericToCartesian(glm::length(m_examinePoint - getTranslation()), getTheta(), M_PI / 2);
        moveTo(examinePos, getTheta(), -M_PI / 2, 0.6f);
    }
    else
    {
        lookAt(getTheta(), -M_PI / 2, 0.6f);
    }
}

void CameraNode::extractAngles(const glm::dvec3& _origin, const glm::dvec3& _lookPoint, double& _theta, double& _phi) const
{
    // Extract the CameraNode angles from the current position and the look-at point
    glm::dvec3 dir = glm::normalize(_lookPoint - _origin);
    _theta = atan2(-dir.x, dir.y);
    _phi = -acos(dir.z);
}

void CameraNode::AddViewPoint(glm::dvec3 _point, double _meterPerSec, double _radPerSec)
{
    if (m_trajectory.size() == 0)
    {
        m_trajectory.push_back({ _point, 3.0 * (double)M_PI / 2.0, (double)-M_PI / 2.0, 0.0 });
    }
    else
    {
        KeyPoint prevPt = m_trajectory.back();

        double theta_next, phi_next;
        extractAngles(prevPt.point, _point, theta_next, phi_next);

        // Turn the CameraNode toward the distination
        double delta_theta = (prevPt.theta - theta_next);
        modulo2Pi(0.0, delta_theta);

        double delta_phi = phi_next - prevPt.phi;

        double delta_angle = sqrt(delta_phi * delta_phi + delta_theta * delta_theta);

        double dt_turn = prevPt.dtime_arrival + delta_angle / _radPerSec;
        m_trajectory.push_back({ prevPt.point, theta_next, phi_next, dt_turn });

        // Move the CameraNode to the distination
        double dt_move = dt_turn + glm::distance(prevPt.point, _point) / _meterPerSec;
        m_trajectory.push_back({ _point, theta_next, phi_next, dt_move });
    }
}

void CameraNode::sendNewUIViewPoint()
{
    //TODO REWORK CAMERAINFO
    //Le but c'est de mettre à jour la gui qui a déjà un SafePtr vers la caméra, est-ce qu'on devrait pas utiliser un autre GuiData de refresh
    m_dataDispatcher.updateInformation(new GuiDataCameraInfo(SafePtr<CameraNode>()), this);
}

bool CameraNode::integrityCheck()
{
    bool result = true;
    if (glm::isnan(m_center.x) || glm::isinf(m_center.x) ||
        glm::isnan(m_center.y) || glm::isinf(m_center.y) ||
        glm::isnan(m_center.z) || glm::isinf(m_center.z))
    {
        m_center = glm::dvec3(0.0, 0.0, 0.0);
        result = false;
    }
    
    if (glm::isnan(m_quaternion.w))
    {
        m_quaternion = glm::dquat(0.0, 0.0, 0.0, 1.0);
        result = false;
    }

    return result;
}

void CameraNode::allocAllUniforms()
{
    VulkanManager& vkM = VulkanManager::getInstance();
    vkM.allocUniform(3 * sizeof(glm::mat4), 2, m_uniProjView);
    vkM.allocUniform(sizeof(glm::mat4), 2, m_uniView);
    vkM.allocUniform(sizeof(glm::mat4), 2, m_uniInversedView);
    vkM.allocUniform(sizeof(UniformClippingData) * MAX_ACTIVE_CLIPPING, 2, m_uniClipping);
}

void CameraNode::freeAllUniforms()
{
    VulkanManager& vkM = VulkanManager::getInstance();
    vkM.freeUniform(m_uniProjView);
    vkM.freeUniform(m_uniView);
    vkM.freeUniform(m_uniInversedView);
    vkM.freeUniform(m_uniClipping);
}

void CameraNode::startPlayTrajectory(const uint64_t& animationStep)
{
    m_currentKeyPoint = 1;
    m_initialAnimation = m_animation;
    if (m_trajectory.size() < 2)
        return;

    updateTrajectoryToBezier();

    if (m_isOfflineRendering)
    {
        m_animFrames = 0;
        m_offlineAnimStep = animationStep ? animationStep : 1000.0;
    }
    else
        m_startTrajectoryTime = std::chrono::steady_clock::now();
}

bool CameraNode::animateComplexTrajectory()
{
    // Get time elapsed since the start of the trajectory
    double dtime((double)m_animFrames);
    if (m_isOfflineRendering)
        dtime = (dtime += m_speed) / m_offlineAnimStep;
    else
        dtime = std::chrono::duration<double, std::ratio<1>>(std::chrono::steady_clock::now() - m_startTrajectoryTime).count() * m_speed;

    if (m_currentKeyPoint >= m_trajectory.size())
        return true;

    if (dtime < m_trajectory[m_currentKeyPoint].dtime_arrival)
    {
        KeyPoint kPt0 = m_trajectory[m_currentKeyPoint - 1];
        KeyPoint kPt1 = m_trajectory[m_currentKeyPoint];

        double progress = (dtime - kPt0.dtime_arrival) / (kPt1.dtime_arrival - kPt0.dtime_arrival);

        m_center = (kPt1.point * progress + kPt0.point * (1 - progress));

        double delta_theta = kPt1.theta - kPt0.theta;
        modulo2Pi(0.0, delta_theta);

        double theta1_modulo = kPt0.theta + progress * delta_theta;
        modulo2Pi(0.0, theta1_modulo); //unsure about that

        setThetaAndPhi(theta1_modulo, kPt1.phi * progress + kPt0.phi * (1 - progress));
    }
    else if (m_currentKeyPoint + 1 < m_trajectory.size())
    {
        m_currentKeyPoint++;
        return animateComplexTrajectory();
    }
    else
    {
        // the trajectory is finished

        if (m_loop == false)
        {
            setPosition(m_trajectory[m_currentKeyPoint].point);
            setThetaAndPhi(m_trajectory[m_currentKeyPoint].theta, m_trajectory[m_currentKeyPoint].phi);
            m_animation = m_initialAnimation;
            m_isAnimated = false;
            return true;
        }
        else
        {
            setPosition(m_trajectory[0].point);
            setThetaAndPhi(m_trajectory[0].theta, m_trajectory[0].phi);
            m_startTrajectoryTime = std::chrono::steady_clock::now();
            m_currentKeyPoint = 1;
        }
    }

    return true;
}

/*
void CameraNode::getKeyPointsFromPath(const std::vector<glm::dvec3>& pathPoints, double speed)
{
    m_trajectory = std::vector<KeyPoint>(0);
    TLID vpid = m_animation[0];

    ViewPointNode* vp = nullptr;

    m_animation.clear();

    std::vector<KeyPoint> result;
    if (pathPoints.size() < 2)
        return;
    double dtime = 0;
    KeyPoint firstKeyPoint;
    for (size_t i = 0; i < pathPoints.size(); i++)
    {
        KeyPoint currKeyPoint;
        currKeyPoint.point = pathPoints[i];
        glm::dvec3 direction;
        if (i > 0)
        {
            direction = pathPoints[i] - pathPoints[i - 1];
            dtime += glm::length(direction) / speed;
        }
        else
            direction = pathPoints[1] - pathPoints[0];
        double rho = glm::length(direction);
        double phi = -acos(-direction.z / rho);
        phi = -phi - M_PI;
        double theta = -atan(direction.x / direction.y);
        if (direction.y < 0)
            theta += M_PI;

        currKeyPoint.phi = phi;
        currKeyPoint.theta = theta;
        currKeyPoint.dtime_arrival = dtime;
        if (i == 0)
            firstKeyPoint = currKeyPoint;
        else if (i == (pathPoints.size() - 1))
            firstKeyPoint.dtime_arrival = currKeyPoint.dtime_arrival + 1.0f;
        m_trajectory.push_back(currKeyPoint);
        vp->setPosition(currKeyPoint.point);
        m_animation.push_back(vpid);
    }

    if (m_loop)
    {
        m_trajectory.push_back(firstKeyPoint);
        vp->setPosition(firstKeyPoint.point);
        m_animation.push_back(vpid);
    }
    return;
}
*/

void CameraNode::updateTrajectoryToBezier()
{
    double distanceThreshold(0.1);
    std::vector<glm::dvec3> controlPoints;
    for (int i = 0; i < m_trajectory.size(); i++)
    {
        controlPoints.push_back(m_trajectory[i].point);
    }
    if (m_loop)
    {
        controlPoints.push_back(m_trajectory[0].point);
    }
    std::vector<glm::dvec3> pathPoints;
    computeBezierPath(controlPoints, pathPoints, distanceThreshold);
    //getKeyPointsFromPath(pathPoints, m_speed);
}

void CameraNode::computeBezierPath(const std::vector<glm::dvec3>& controlPoints, std::vector<glm::dvec3>& pathPoints, double distanceCriterion)
{
    uint64_t N = controlPoints.size();
    if (computeMaxDistanceBetweenKeyPoints(controlPoints) < distanceCriterion)
    {
        return;
    }
    else
    {
        std::vector<std::vector<glm::dvec3>> T;
        std::vector<glm::dvec3> T1, T2;
        computeBezierStep(controlPoints, T);
        for (int i = 0; i < N; i++)
        {
            T1.push_back(T[i][0]);
            T2.push_back(T[N - i - 1][i]);
        }
        computeBezierPath(T1, pathPoints, distanceCriterion);
        pathPoints.push_back(T[N - 1][0]);
        computeBezierPath(T2, pathPoints, distanceCriterion);
    }
}

void CameraNode::computeBezierStep(const std::vector<glm::dvec3>& keyPoints, std::vector<std::vector<glm::dvec3>>& T)
{
    std::vector<std::vector<glm::dvec3>> result;
    uint64_t N = keyPoints.size();

    //initialize 
    for (uint64_t j = 0; j < N; j++)
    {
        std::vector<glm::dvec3> temp;
        result.push_back(temp);
        for (uint64_t i = 0; i < N; i++)
        {
            result[j].push_back(keyPoints[i]);
        }
    }

    //fill
    for (uint64_t i = 1; i < N; i++)
    {
        for (uint64_t j = 0; j < N - i; j++)
        {
            result[i][j] = ((result[i - 1][j] + result[i - 1][j + 1]) * 0.5);
        }
    }
    T = result;
    return;
}

double CameraNode::computeMaxDistanceBetweenKeyPoints(const std::vector<glm::dvec3>& keyPoints)
{
    double maxDistance(0);
    for (uint64_t i = 0; i < (keyPoints.size() - 1); i++)
    {
        for (uint64_t j = i + 1; j < keyPoints.size(); j++)
        {
            double currDistance = glm::length(keyPoints[j] - keyPoints[i]);
            if (currDistance > maxDistance)
                maxDistance = currDistance;
        }
    }
    return maxDistance;
}

void CameraNode::updateTrajectoryToPolynomial()
{
    std::vector<glm::dvec3> controlPoints;
    for (uint64_t i = 0; i < m_trajectory.size(); i++)
    {
        controlPoints.push_back(m_trajectory[i].point);
    }
    std::vector<glm::dvec3> pathPoints;
    computePolynomialPath(controlPoints, pathPoints);
    //getKeyPointsFromPath(pathPoints, m_speed);
}

void CameraNode::computePolynomialPath(const std::vector<glm::dvec3>& keyPoints, std::vector<glm::dvec3>& T)
{
    uint64_t numberOfSteps(50);
    uint64_t N = keyPoints.size();
    if (N < 3) return;
    for (uint64_t i = 0; i < (N - 1); i++)
    {
        glm::dvec3 direction1, direction2;	//tangent at controlPoints
        if (i == 0)
        {
            direction1 = keyPoints[1] - 0.75 * keyPoints[0] - 0.25 * keyPoints[2];
            direction2 = keyPoints[2] - keyPoints[0];
        }
        else if (i == (N - 2))
        {
            direction1 = keyPoints[N - 1] - keyPoints[N - 3];
            direction2 = keyPoints[N - 1] - 0.75 * keyPoints[N - 2] - 0.25 * keyPoints[N - 3];
        }
        else
        {
            direction1 = keyPoints[i + 1] - keyPoints[i - 1];
            direction2 = keyPoints[i + 2] - keyPoints[i];
        }
        direction1 /= 0.5 * sqrt(glm::length(direction1));
        direction2 /= 0.5 * sqrt(glm::length(direction2));

        for (uint64_t j = 0; j < numberOfSteps; j++)
        {
            double t = j / (double)numberOfSteps;
            glm::dvec3 temp1;

            temp1 = (t - 1) * (t - 1) * (2 * t + 1) * keyPoints[i] + t * t * (3 - 2 * t) * keyPoints[i + 1] + (t - 1) * (t - 1) * t * direction1 + t * t * (t - 1) * direction2; //interpolation
            T.push_back(temp1);
        }
    }
}

//GUIData
void CameraNode::informData(IGuiData* data)
{
    if (m_functions.find(data->getType()) != m_functions.end())
    {
        GuiDataFunction fct = m_functions.at(data->getType());
        (this->*fct)(data);
    }
}

void CameraNode::onRenderUnitUsage(IGuiData* data)
{
    m_unitUsage = static_cast<GuiDataRenderUnitUsage*>(data)->m_valueParameters;
}

void CameraNode::onRenderColorMode(IGuiData* data)
{
    m_mode = static_cast<GuiDataRenderColorMode*>(data)->m_mode;
}

void CameraNode::onRenderPointSize(IGuiData* data)
{
    m_pointSize = static_cast<GuiDataRenderPointSize*>(data)->m_pointSize;
}

void CameraNode::onRenderBrightness(IGuiData* data)
{
    m_brightness = static_cast<GuiDataRenderBrightness*>(data)->m_brightness;
}

void CameraNode::onRenderContrast(IGuiData* data)
{
    m_contrast = static_cast<GuiDataRenderContrast*>(data)->m_contrast;
}

void CameraNode::onRenderLuminance(IGuiData* data)
{
    m_luminance = static_cast<GuiDataRenderLuminance*>(data)->m_luminance;
}

void CameraNode::onRenderSaturation(IGuiData* data)
{
    m_saturation = static_cast<GuiDataRenderSaturation*>(data)->m_saturation;
}

void CameraNode::onRenderBlending(IGuiData* data)
{
    m_hue = static_cast<GuiDataRenderBlending*>(data)->m_hue;
}

void CameraNode::onRenderTransparency(IGuiData* data)
{
    auto castData = static_cast<GuiDataRenderTransparency*>(data);
    m_transparency = castData->m_transparencyValue;
    m_blendMode = castData->m_mode;
    sendNewUIViewPoint();
}

void CameraNode::onRenderTransparencyOptions(IGuiData* data)
{
    auto castData = static_cast<GuiDataRenderTransparencyOptions*>(data);
    m_reduceFlash = castData->m_reduceFlash;
    m_negativeEffect = castData->m_negativeEffect;
    sendNewUIViewPoint();
}

void CameraNode::onRenderFlatColor(IGuiData* data)
{
    m_flatColor = static_cast<GuiDataRenderFlatColor*>(data)->m_color;
}

void CameraNode::onRenderDistanceRampValues(IGuiData* data)
{
    auto castdata = static_cast<GuiDataRenderDistanceRampValues*>(data);
    m_distRampMin = castdata->m_min;
    m_distRampMax = castdata->m_max;
    m_distRampSteps = castdata->m_steps;
}

void CameraNode::onRenderMeasureOptions(IGuiData* data)
{
    auto measureOptions = static_cast<GuiDataRenderMeasureOptions*>(data);
    m_measureMask = measureOptions->m_showMask;
}

void CameraNode::onRenderDisplayObjectTexts(IGuiData* data)
{
    auto displayData = static_cast<GuiDataRenderDisplayObjectTexts*>(data);
    m_displayAllMarkersTexts = displayData->m_display;
}

void CameraNode::onRenderTextFilter(IGuiData* data)
{
    auto textFilter = static_cast<GuiDataRenderTextFilter*>(data);
    m_textOptions.m_filter = textFilter->m_textFilter;
}

void CameraNode::onRenderTextTheme(IGuiData* data)
{
    auto textTheme = static_cast<GuiDataRenderTextTheme*>(data); 
    m_textOptions.m_textTheme = textTheme->m_textTheme;
}

void CameraNode::onRenderTextFontSize(IGuiData* data)
{
    auto textFont = static_cast<GuiDataRenderTextFontSize*>(data);
    m_textOptions.m_textFontSize = textFont->m_textFontSize;
}

void CameraNode::onRenderMarkerDisplayOptions(IGuiData* data)
{
    auto options = static_cast<GuiDataMarkerDisplayOptions*>(data);
    m_markerOptions = options->m_parameters;
}

void CameraNode::onRenderAlphaObjects(IGuiData* data)
{
    m_alphaObject = static_cast<GuiDataAlphaObjectsRendering*>(data)->m_alpha;
}

void CameraNode::onRenderNormals(IGuiData* data)
{
    GuiDataPostRenderingNormals* normalsInfo = static_cast<GuiDataPostRenderingNormals*>(data);
    if (normalsInfo->m_onlySimpleNormalsInfo)
    {
        m_postRenderingNormals.show = normalsInfo->m_normals.show;
        m_postRenderingNormals.inverseTone = normalsInfo->m_normals.inverseTone;
        m_postRenderingNormals.normalStrength = normalsInfo->m_normals.normalStrength;
    }
    else
        m_postRenderingNormals = normalsInfo->m_normals;

    sendNewUIViewPoint();
}

void CameraNode::onRenderRampScale(IGuiData* data)
{
    GuiDataRampScale* cast_data = static_cast<GuiDataRampScale*>(data);

    m_rampScale = cast_data->m_rampScale;

    sendNewUIViewPoint();
}

void CameraNode::onBackgroundColor(IGuiData* data)
{
    m_backgroundColor = static_cast<GuiDataRenderBackgroundColor*>(data)->m_color;
}

void CameraNode::onAdjustZoomToScene(IGuiData* data)
{
    auto cast_data = static_cast<GuiDataRenderAdjustZoom*>(data);

    adjustToScene(cast_data->scene_bbox_);
}

void CameraNode::onRenderCameraMoveTo(IGuiData* data)
{
    auto castData = static_cast<GuiDataRenderCameraMoveTo*>(data);
    setPosition(castData->m_newPosition);
}

void CameraNode::onRenderRotateCamera(IGuiData* data)
{
    auto viewData = static_cast<GuiDataRenderRotateCamera*>(data);
    if (viewData->m_additive)
    {
        if (!std::isnan(viewData->m_theta))
            yaw(viewData->m_theta);
        if (!std::isnan(viewData->m_phi))
            pitch(viewData->m_phi);
    }
    else
    {
        setThetaAndPhi(viewData->m_theta, (std::isnan(viewData->m_phi) ? getPhi() : viewData->m_phi));
    }
}

void CameraNode::onRenderExamine(IGuiData* data)
{
    auto renderdata = static_cast<GuiDataRenderExamine*>(data);
    if (renderdata->m_activate == true)
    {
        m_examinePoint = renderdata->m_position;

        if (isExamineActive())
        {
            if (m_centeringExamine) {
                if (m_projectionMode == ProjectionMode::Perspective)
                {
                    lookAt(m_examinePoint, 0.6f);
                }
                else if (m_projectionMode == ProjectionMode::Orthographic)
                {
                    translateTo(m_examinePoint, 0.6f);
                }
            }
        }
        resetPanoramicScan();
    }

    sendNewUIViewPoint();
}

void CameraNode::onExamineOptions(IGuiData* data)
{
    auto castdata = static_cast<GuiDataExamineOptions*>(data);
    m_centeringExamine = castdata->m_targetCentering;
    m_keepExamineOnPan = castdata->m_keepOnPan;
}

void CameraNode::onMoveToData(IGuiData* data)
{
    GuiDataMoveToData* guiData = static_cast<GuiDataMoveToData*>(data);
    moveToData(guiData->m_object);
}

void CameraNode::moveToData(const SafePtr<AGraphNode>& data)
{
    float distance(1.0f);
    float time(1.0f);
    bool isResetProj(false);
    m_panoramicScan.reset();
    glm::dvec3 position;
    glm::dvec3 lookDir;
    SafePtr<AGraphNode> object = data;

    ElementType type;
    {
        ReadPtr<AGraphNode> node = object.cget();
        if (!node)
            return;
        type = node->getType();
    }

    switch (type)
    {
    case ElementType::Scan:
    {
        SafePtr<ScanNode> scan = static_pointer_cast<ScanNode>(object);
        ReadPtr<ScanNode> rScan = scan.cget();
        if (rScan->getScanGuid() == tls::ScanGuid())
            return;
        distance = 0.0f;
        time = 1.5f;
        isResetProj = true;
        position = rScan->getCenter();
        m_panoramicScan = scan;
    }
    break;
    case ElementType::Tag:
    {
        ReadPtr<AObjectNode> rTag = static_pointer_cast<AObjectNode>(object).cget();
        position = rTag->getCenter();
    }
    break;
    case ElementType::BeamBendingMeasure:
    {
        ReadPtr<BeamBendingMeasureNode> rBbm = static_pointer_cast<BeamBendingMeasureNode>(object).cget();
        position = rBbm->getMaxBendingPos();
    }
    break;
    case ElementType::ColumnTiltMeasure:
    {
        ReadPtr<ColumnTiltMeasureNode> rCtm = static_pointer_cast<ColumnTiltMeasureNode>(object).cget();
        position = rCtm->getTopPoint();
    }
    break;
    case ElementType::PipeToPipeMeasure:
    {
        ReadPtr<PipeToPipeMeasureNode> pipim = static_pointer_cast<PipeToPipeMeasureNode>(object).cget();
        position = pipim->getPipe1Center();
    }
    break;
    case ElementType::PipeToPlaneMeasure:
    {
        ReadPtr<PipeToPlaneMeasureNode> piplm = static_pointer_cast<PipeToPlaneMeasureNode>(object).cget();
        position = piplm->getPipeCenter();
    }
    break;
    case ElementType::PointToPipeMeasure:
    {
        ReadPtr<PointToPipeMeasureNode> popim = static_pointer_cast<PointToPipeMeasureNode>(object).cget();
        position = popim->getPipeCenter();
    }
    break;
    case ElementType::PointToPlaneMeasure:
    {
        ReadPtr<PointToPlaneMeasureNode> poplm = static_pointer_cast<PointToPlaneMeasureNode>(object).cget();
        position = poplm->getPointOnPlane();
    }
    break;
    case ElementType::SimpleMeasure:
    {
        ReadPtr<SimpleMeasureNode> sm = static_pointer_cast<SimpleMeasureNode>(object).cget();
        position = sm->getOriginPos();
    }
    break;
    case ElementType::PolylineMeasure:
    {
        ReadPtr<PolylineMeasureNode> pm = static_pointer_cast<PolylineMeasureNode>(object).cget();
        position = pm->getFirstPos();
    }
    break;
    case ElementType::Point:
    {
        ReadPtr<AObjectNode> cli = static_pointer_cast<AObjectNode>(object).cget();
        position = cli->getCenter();
    }
    break;
    case ElementType::Torus:
    case ElementType::Sphere:
    case ElementType::Cylinder:
    case ElementType::Box:
    case ElementType::MeshObject:
    {
        ReadPtr<AObjectNode> cli = static_pointer_cast<AObjectNode>(object).cget();
        position = cli->getCenter();
        distance = glm::distance(cli->getScale(), glm::dvec3(0.0f)) * 2.0f;
    }
    break;
    case ElementType::PCO:
    {
        ReadPtr<ScanObjectNode> cli = static_pointer_cast<ScanObjectNode>(object).cget();
        position = cli->getCenter();
        distance = glm::distance(cli->getScale(), glm::dvec3(0.0f)) * 2.0f;
    }
    break;
    case ElementType::ViewPoint:
    {
        SafePtr<ViewPointNode> vp = static_pointer_cast<ViewPointNode>(object);
        ReadPtr<ViewPointNode> cli = vp.cget();
        position = cli->getCenter();
        distance = 0.0f;
        if (cli->isPanoramicScan())
            m_panoramicScan = cli->getPanoramicScan();

        lookDir = glm::dvec4(0.0, 0.0, 1.0, 1.0) * cli->getInverseTransformation();
        m_animation.push_back(vp);
    }
    break;
    default:
        return;
    }

    if (getProjectionMode() == ProjectionMode::Orthographic)
        translateTo(position, time);
    else
    {
        if (type == ElementType::ViewPoint)
            moveTo(position, time, lookDir, glm::dvec3(0.0, 0.0, 1.0));
        else
            moveTo(position, distance, time);

        if (isResetProj)
            resetPerspectiveZoom();
    }


    if (type != ElementType::ViewPoint && type != ElementType::Scan)
        m_examinePoint = position;
    else
        resetExaminePoint();

    sendNewUIViewPoint();
}

void CameraNode::onCameraToViewPoint(IGuiData* data)
{
    auto gui = static_cast<GuiDataCameraInfo*>(data);

    ReadPtr<CameraNode> rCam = gui->m_camera.cget();
    if (!rCam)
        return;

    m_center = rCam->getCenter();
    m_quaternion = rCam->getOrientation();

    static_cast<DisplayParameters&>(*this) = rCam->getDisplayParameters();
    // NOTE(Robin) - On veut garder le ratio de la camera ET prendre le "zoom" du ViewPoint
    applyProjection(*&rCam);

    //TODO INFORM m_dataDispatcher.sendControl(new control::application::RenderModeUpdate(m_mode, null_tlid));
    //TODO INFORM m_dataDispatcher.updateInformation(new GuiDataDisplayGuizmo(null_tlid, m_displayGizmo), this);
}

void CameraNode::onRenderFov(IGuiData* data)
{
    auto fov = static_cast<GuiDataRenderFov*>(data);
    setFovy((fov->m_fov) * M_PI / 180.0);
}

void CameraNode::onDisplayGizmo(IGuiData* data)
{
    m_displayGizmo = static_cast<GuiDataDisplayGuizmo*>(data)->m_isDisplayed;
}

void CameraNode::onRenderNavigationConstraint(IGuiData* data)
{
    auto castData = static_cast<GuiDataRenderNavigationConstraint*>(data);
    m_constraint = castData->m_constraint;
    checkAndHorizontalAnimation();
}

void CameraNode::onRenderApplyNavigationConstraint(IGuiData* data)
{
    auto castData = static_cast<GuiDataRenderApplyNavigationConstraint*>(data);
    m_applyConstraint = castData->m_checked;
    checkAndHorizontalAnimation();
}

void CameraNode::onRenderNavigationParameters(IGuiData* data)
{
    m_navParams = static_cast<GuiDataRenderNavigationParameters*>(data)->m_navParam;
}

void CameraNode::onRenderPerspectiveZ(IGuiData* data)
{
    auto castData = static_cast<GuiDataRenderPerspectiveZBounds*>(data);
    if (m_projectionMode == ProjectionMode::Perspective)
    {
        setPerspectiveZBounds(castData->m_zBounds);
    }

    m_savedCameraParams[(int)ProjectionMode::Perspective].setPerspectiveZBounds(castData->m_zBounds);
}

void CameraNode::onRenderOrthographicZ(IGuiData* data)
{
    auto castData = static_cast<GuiDataRenderOrthographicZBounds*>(data);
    if (m_projectionMode == ProjectionMode::Orthographic)
    {
        setOrthographicZBounds(castData->m_zBounds);
    }

    m_savedCameraParams[(int)ProjectionMode::Orthographic].setOrthographicZBounds(castData->m_zBounds);
}

//FROM Camera
void CameraNode::initPerspective(double _fovy, double _nearZ, double _farZ, double _screenRatio)
{
    if (_fovy <= 0.0)
        return;

    if (m_projectionMode == ProjectionMode::Orthographic)
        m_savedCameraParams[(int)ProjectionMode::Orthographic].setProjectionData(*this);

    ProjectionData::initPerspective(_fovy, _nearZ, _farZ, _screenRatio);

    // Replace the backup fovy
    m_savedCameraParams[(int)ProjectionMode::Perspective].setProjectionData(*this);
}

void CameraNode::initOrthographic(double _realH, double _nearZ, double _farZ, double _screenRatio)
{
    if (_realH <= 0.0)
        // invalid parameters
        return;

    if (m_projectionMode == ProjectionMode::Perspective)
        m_savedCameraParams[(int)ProjectionMode::Perspective].setProjectionData(*this);

    ProjectionData::initOrthographic(_realH, _nearZ, _farZ, _screenRatio);
}

void CameraNode::setScreenRatio(int _screenW, int _screenH)
{
    if (_screenW <= 0 || _screenH <= 0)
        // invalid parameters
        return;

    ProjectionData::setRatioW_H((double)_screenW / (double)_screenH);

    // Maintain the screenRatio for both projection mode
    for (size_t mode = 0; mode < (size_t)ProjectionMode::MAX_ENUM; ++mode)
    {
        m_savedCameraParams[mode].setRatioW_H((double)_screenW / (double)_screenH);
    }
}

void CameraNode::setProjectionMode(ProjectionMode _projMode)
{
    static_cast<ProjectionData&>(*this) = m_savedCameraParams[(size_t)_projMode];
    sendNewUIViewPoint();
}

void CameraNode::setProjectionFrustum(double _left, double _right, double _bottom, double _top, double _near, double _far)
{
    m_projectionFrustum = { _left, _right, _bottom, _top, _near, _far };
}

void CameraNode::zoom(double _amount, double _dtime)
{
    if (_amount == 1.0)
        return;

    m_durationAnimZoomSec = _dtime;
    m_start_zoom = m_prev_time;

    if (m_projectionMode == ProjectionMode::Perspective)
    {
        m_startFovy = getFovy();
        m_endFovy = m_endFovy * _amount;
        if (m_endFovy < MIN_FOVY) m_endFovy = MIN_FOVY;
        if (m_endFovy > MAX_FOVY) m_endFovy = MAX_FOVY;
    }
    else
    {
        m_startRealH = getHeightAt1m();
        // Init the "end" value if there is no animation ongoing
        m_endRealH = m_animateProj ? m_endRealH : m_startRealH;
        m_endRealH = m_endRealH * _amount;
        if (m_endRealH < MIN_HEIGHT) m_endRealH = MIN_HEIGHT;
        if (m_endRealH > MAX_HEIGHT) m_endRealH = MAX_HEIGHT;
    }
    m_startPos = m_center;
    m_endPos = m_center;

    m_animateProj = true;
}

void CameraNode::zoomOnPointOfInterest(glm::dvec2 mouse_cursor, double amount, double dtime)
{
    // 1.0 does nothing, negative value is a bug
    if (amount == 1.0 || amount <= 0.0)
        return;

    // If we have an examine point, it becomes the POI
    glm::dvec2 poi_centered;
    if (isExamineActive())
    {
        poi_centered = m_viewMatrix * glm::dvec4(m_examinePoint, 1.0);
        poi_centered.x /= getWidthAt1m();
        poi_centered.y /= getHeightAt1m();
    }
    else
    {
        poi_centered = (mouse_cursor - glm::dvec2(0.5, 0.5));
    }

    if (m_projectionMode == ProjectionMode::Perspective)
    {
        // TODO(Robin) - Les maths sont compliqués, à voir plus tard...
        zoom(amount, dtime);
    }
    else
    {
        m_durationAnimZoomSec = dtime;
        m_start_zoom = m_prev_time;

        m_startPos = m_center;
        m_startRealH = getHeightAt1m();
        // Init the "end" value if there is no animation ongoing
        m_endRealH = m_animateProj ? m_endRealH : m_startRealH;
        m_endRealH = m_endRealH * amount;
        if (m_endRealH < MIN_HEIGHT) m_endRealH = MIN_HEIGHT;
        if (m_endRealH > MAX_HEIGHT) m_endRealH = MAX_HEIGHT;

        double deltaH = m_startRealH - m_endRealH;
        m_endPos = m_startPos + (deltaH * getRatioW_H() * poi_centered.x) * m_right + (deltaH * -poi_centered.y) * m_up;
    }

    m_animateProj = true;
}

void CameraNode::setOrthoZoom(double _realW, double _realH, const glm::dvec3& endPos, double _duration)
{
    m_animateProj = true;
    m_durationAnimZoomSec = _duration;
    m_start_zoom = m_prev_time;
    m_startPos = m_center;
    m_endPos = endPos;

    m_startRealH = getHeightAt1m();

    if (_realW / _realH > getRatioW_H())
        m_endRealH = _realW / getRatioW_H();
    else
        m_endRealH = _realH;
}

void CameraNode::resetPerspectiveZoom()
{
    // Do not reset again if a zoom animation is ongoing
    if (m_animateProj == false && m_projectionMode == ProjectionMode::Perspective)
    {
        m_animateProj = true;
        m_start_zoom = m_prev_time;
        m_startFovy = getFovy();
        m_endFovy = m_savedCameraParams[(int)ProjectionMode::Perspective].getFovy();
        m_durationAnimZoomSec = 0.5;
    }
}

void CameraNode::setFovy(float _fovy, bool _saveBackup)
{
    if (_saveBackup)
        m_savedCameraParams[(int)ProjectionMode::Perspective].setFovy(_fovy);
    if (m_projectionMode == ProjectionMode::Perspective)
        ProjectionData::setFovy(_fovy);
}

void CameraNode::setOrthoHeight(double _height)
{
    m_savedCameraParams[(int)ProjectionMode::Orthographic].setOrthoHeight(_height);
    if (m_projectionMode == ProjectionMode::Orthographic)
        ProjectionData::setOrthoHeight(_height);
    sendNewUIViewPoint();
}


MarkerDrawData CameraNode::getExamineTarget() const
{
    scs::MarkerStyleDefinition marker_style = scs::markerStyleDefs.at(scs::MarkerIcon::Target);
    scs::PrimitiveDef prim = scs::g_shapePrimitives.at(marker_style.shape);
    Color32 color = { 223u, 98u, 0u, 255u };

    return {
        { (float)m_examinePoint.x, (float)m_examinePoint.y, (float)m_examinePoint.z },
        color.RGBA(),
        INVALID_PICKING_ID,
        (uint32_t)scs::MarkerIcon::Target,
        prim.firstVertex,
        prim.vertexCount,
        0 // not hover, not selected, dont show true color
    };
}
