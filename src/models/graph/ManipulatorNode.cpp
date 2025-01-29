#include "models/graph/ManipulatorNode.h"
#include "controller/controls/ControlObject3DEdition.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "models/graph/CameraNode.h"

#include "utils/math/trigo.h"
#include "utils/math/geometry.h"

#include "utils/Logger.h"
#include "vulkan/MeshManager.h"

#include <QtGui/qguiapplication.h>

#define SGLog Logger::log(LoggerMode::SceneGraphLog)

#define BASE_SCALE 0.80001
#define MIN_SCALE 0.20001
#define MAX_SCALE 1.40001

#define SLOW_MANIPULATOR 0.05

std::unordered_set<ElementType> ManipulatorNode::s_manipulableTypes = {
   ElementType::Box,
   ElementType::Grid,
   ElementType::Cylinder,
   ElementType::Torus,
   ElementType::Sphere,
   ElementType::PCO,
   ElementType::MeshObject
};

ManipulatorNode::ManipulatorNode(IDataDispatcher& dataDispatcher)
	: AGraphNode()
    , m_isLocalManipulation(true)
	, m_dataDispatcher(dataDispatcher)
	, m_currentSelection(Selection::None)
	, m_startCursor(0, 0)
	, m_screenSize(0, 0)
	, m_distanceToDisplay(0.0)
{
    m_manipulationMode = ManipulationMode::Translation;
    m_dataDispatcher.updateInformation(new GuiDataCallBackManipulatorMode(m_manipulationMode));

    m_cumulatedManipData = ManipulateData();
}

ManipulatorNode::~ManipulatorNode()
{}

bool ManipulatorNode::isDisplayed() const
{
    if (m_targets.empty())
        return false;

    for (const SafePtr<AObjectNode>& target : m_targets)
    {
        ReadPtr<AObjectNode> rTarget = target.cget();
        if (!rTarget || !rTarget->isVisible() || rTarget->isDead())
            continue;
        return true;
    }
    return false;
}

bool ManipulatorNode::setTarget(const std::unordered_set<SafePtr<AObjectNode>>& targets)
{
    if (targets.empty())
    {
        m_targets.clear();
        return true;
    }

    m_targets = targets;
    
    if (m_targets.size() == 1)
    {
        ReadPtr<AObjectNode> rTarget = m_targets.begin()->cget();
        if (rTarget && !rTarget->isAcceptingManipulatorMode(m_manipulationMode))
        {
            std::unordered_set modes(rTarget->getAcceptableManipulationModes());
            if (modes.empty())
                return false;
            m_manipulationMode = *modes.begin();
        }
    }
    else
    {
        if (m_manipulationMode != ManipulationMode::Translation && m_manipulationMode != ManipulationMode::Rotation)
            m_manipulationMode = ManipulationMode::Translation;
    }

    m_decalPosition = glm::dvec3(NAN);
    updateTransfo();
    m_dataDispatcher.updateInformation(new GuiDataCallBackManipulatorMode(m_manipulationMode));
    return true;
}

void ManipulatorNode::setManipulationMode(ManipulationMode mode)
{
    if (m_targets.size() == 1)
    {
        ReadPtr<AObjectNode> rTarget = m_targets.begin()->cget();
        if (rTarget && rTarget->isAcceptingManipulatorMode(mode))
            m_manipulationMode = mode;
    }
    else
    {
        if (mode == ManipulationMode::Translation || mode == ManipulationMode::Rotation)
            m_manipulationMode = mode;
    }

    m_dataDispatcher.updateInformation(new GuiDataCallBackManipulatorMode(m_manipulationMode));
}

ManipulationMode ManipulatorNode::getManipulationMode() const
{
    return m_manipulationMode;
}

std::unordered_set<Selection> ManipulatorNode::getAcceptableSelections() const
{
    std::unordered_set<Selection> acceptableSelections;

    if (m_targets.size() == 1)
    {
        ReadPtr<AObjectNode> rTarget = m_targets.begin()->cget();
        if (rTarget)
            acceptableSelections = rTarget->getAcceptableSelections(m_manipulationMode);
    }
    else
        acceptableSelections = { Selection::X, Selection::Y, Selection::Z };

    return acceptableSelections;
}

// QUESTION(robin) - Do we need a specific function to reset the selection (ie when mouse is released) ?
//                 - Can this be usefull ?
// Answer(aur?lien) - We need at least a release method to send the "undo" control
bool ManipulatorNode::setCurrentSelection(Selection selection, const glm::ivec2& cursor, const SafePtr<ManipulatorNode>& manip)
{
    if (selection >= Selection::MAX_ENUM)
        return false;

    WritePtr<ManipulatorNode> wManip = manip.get();

    if (!wManip || wManip->m_targets.empty())
        return false;


    if (selection == Selection::None && wManip->m_currentSelection != Selection::None)
    {
        wManip->m_dataDispatcher.sendControl(new control::object3DEdition::ManipulateObjects(wManip->m_targets, wManip->m_cumulatedManipData, manip));
        wManip->m_cumulatedManipData = ManipulateData();
        wManip->m_currentSelection = Selection::None;
    }

    // Init a new manipulation
    if (wManip->m_currentSelection == Selection::None)
    {
        wManip->m_currentSelection = selection;
        wManip->m_startCursor = cursor;
        wManip->m_distanceToDisplay = 0.0;
    }

    switch (wManip->m_manipulationMode)
    {
    case ManipulationMode::Translation:
        wManip->m_gizmo_dir = glm::dvec3(0.0, 0.0, 0.0);
        wManip->m_gizmo_pos = glm::dvec3(std::numeric_limits<double>::quiet_NaN(),
            std::numeric_limits<double>::quiet_NaN(),
            std::numeric_limits<double>::quiet_NaN());
        wManip->m_cursor_on_axis = 0.0;
        // FIXME - We need the camera
        //initTranslation(camera);
        break;
    default:
        break;
    }

    return true;
}

AGraphNode::Type ManipulatorNode::getGraphType() const
{
    return AGraphNode::Type::Manipulator;
}

std::shared_ptr<MeshBuffer> ManipulatorNode::getActiveMeshBuffer() const
{
    return MeshManager::getInstance().getManipMesh(m_manipulationMode);
}

bool ManipulatorNode::isAcceptingObjectToManip(ElementType type)
{
    if (s_manipulableTypes.find(type) != s_manipulableTypes.end())
        return true;
    return false;
}

Selection ManipulatorNode::getCurrentSelection() const
{
    return m_currentSelection;
}

bool ManipulatorNode::isLocalManipulation() const
{
    return m_isLocalManipulation || m_manipulationMode == ManipulationMode::Extrusion || m_manipulationMode == ManipulationMode::Scale;
}

void ManipulatorNode::setLocalManipulation(const bool& isLocal)
{
    m_isLocalManipulation = isLocal;
}

// TODO - we can work with only the 'view' and 'proj' matrices
//  (*) viewProj = proj * view
//  (*) in updateRotation() the ProjectionModule can be replaced by the proj matrix
bool ManipulatorNode::updateEvent(const SafePtr<ManipulatorNode>& manipNode, const glm::ivec2& _mousePosition, const glm::ivec2& screenSize, const CameraNode& camera)
{
    WritePtr<ManipulatorNode> wManip = manipNode.get();
    if (!wManip)
        return false;

    if (wManip->getCurrentSelection() == Selection::None)
        return false;

    if (wManip->m_targets.empty())
        return false;

    wManip->m_screenSize = screenSize;
    glm::dmat4 inverseView = camera.getModelMatrix();

    ManipulateData dManipData;
    switch (wManip->m_manipulationMode)
    {
    case ManipulationMode::Translation:
    {
        wManip->updateTranslation(_mousePosition, dManipData, camera);
        break;
    }
    case ManipulationMode::Rotation:
    {
        wManip->updateRotation(_mousePosition, inverseView, dManipData, camera);
        break;
    }
    case ManipulationMode::Extrusion:
    {
        wManip->updateExtrusion(_mousePosition, dManipData, camera);
        break;
    }
    /*case ManipulationMode::Scale:
    {
        updateScale(_mousePosition, dTransfo, camera);
        break;
    }*/
    default:
        return false;
    }

    wManip->m_cumulatedManipData.addManipulateData(dManipData);
    wManip->manipulateTransfo(dManipData);
    for (const SafePtr<AObjectNode>& target : wManip->m_targets)
    {
        WritePtr<AObjectNode> wTarget = target.get();
        if (wTarget)
            wTarget->manipulateTransfo(dManipData);
    }

    return true;
}

const double& ManipulatorNode::getDistanceToDisplay() const
{
    return m_distanceToDisplay;
}

glm::dmat4 ManipulatorNode::computeTransformationMatrix()
{
    glm::dvec3 translation = getCenter();
    glm::dquat qrot = getOrientation();

    if (isLocalManipulation())
        return (glm::translate(glm::dmat4(1.0), translation) * glm::mat4_cast(qrot));
    else
        return (glm::translate(glm::dmat4(1.0), translation));
}

void ManipulatorNode::initTranslation(const CameraNode& camera)
{
    m_gizmo_dir = getWorldAxis();
    m_gizmo_pos = getGizmoAncorage();

    m_cursor_on_axis = intersect_with_gizmo_axis(m_gizmo_pos, m_gizmo_dir, m_startCursor, camera);
}

void ManipulatorNode::updateTranslation(const glm::ivec2& _mousePosition, ManipulateData& manipData, const CameraNode& camera)
{
    // +++++ Additive method -----
    glm::dvec3 gizmo_dir = getWorldAxis();
    glm::dvec3 gizmo_pos = getGizmoAncorage();
    double cursor_on_axis = intersect_with_gizmo_axis(gizmo_pos, gizmo_dir, m_startCursor, camera);
    double current_cursor = intersect_with_gizmo_axis(gizmo_pos, gizmo_dir, _mousePosition, camera);
   
    glm::dvec3 delta = (current_cursor - cursor_on_axis) * gizmo_dir * (QGuiApplication::queryKeyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier) ? SLOW_MANIPULATOR : 1);

    m_startCursor = _mousePosition;

    // ***** Absolute method *****
    // FIXME - Pour que méthode fonctionne, il faut travailler en coordonnées absolues (pas de décalage par le root). On travaille avec de double en dehors de l'affichage, pas besoin de tronquer les coordonnées !
    /*
    if (std::isnan(m_gizmo_pos.x))
        initTranslation(camera);

    double current_cursor = intersect_with_gizmo_axis(m_gizmo_pos, m_gizmo_dir, _mousePosition, camera);
    glm::dvec3 delta = (current_cursor - m_cursor_on_axis) * m_gizmo_dir;
    m_dataDispatcher.sendControl(new control::object3DEdition::SetCenter(m_target->getId(), m_gizmo_pos + delta));
    */

    //.......S.T.O.R.E............
    m_distanceToDisplay += (glm::dot(delta, gizmo_dir) >= 0 ? 1. : -1.) * length(delta);

    manipData.translation = delta;
}

double ManipulatorNode::intersect_with_gizmo_axis(glm::dvec3 pos_ws, glm::dvec3 dir_ws, glm::ivec2 cursor, const CameraNode& camera)
{
    glm::dvec3 gizmo_pos_end_ws = pos_ws + dir_ws;
    // screen space
    glm::dvec3 gizmo_pos_ss = camera.getScreenProjection(pos_ws, m_screenSize);
    glm::dvec3 gizmo_pos_end_ss = camera.getScreenProjection(gizmo_pos_end_ws, m_screenSize);

    glm::dvec2 gizmo_dir_ss = glm::dvec2(gizmo_pos_end_ss.x - gizmo_pos_ss.x, gizmo_pos_end_ss.y - gizmo_pos_ss.y);
    double l = length(gizmo_dir_ss);
    if (l == 0)
        return 0.0;
    gizmo_dir_ss /= l;
    glm::dvec2 cursor_on_gizmo = tls::math::projection_point_line_2d(cursor, gizmo_pos_ss, gizmo_dir_ss);

    glm::dvec3 cursor_pos_gs, cursor_dir_gs;
    camera.getScreenToViewDirection(cursor_on_gizmo, m_screenSize, cursor_pos_gs, cursor_dir_gs);

    glm::dvec2 st = tls::math::intersect_line_line(pos_ws, dir_ws, cursor_pos_gs, cursor_dir_gs);
    return st.x;
}

void ManipulatorNode::updateRotation(const glm::ivec2& _mousePos, const glm::dmat4& inverseView, ManipulateData& manipData, const CameraNode& camera)
{
    // We do all the transformation in the global space
    // Note(robin) - For the inverse_model we use the inverse_rotation because in fine
    //               we only want to compute the angle between the I on the plane.
    glm::dvec3 axis_gs = getWorldAxis();
    // We turn around the box center. And in the model space the center is 0 !

    glm::dmat4 transfo = getTransformation();

    glm::dvec4 center_gs = transfo * glm::dvec4(0.0, 0.0, 0.0, 1.0);
    double planD = glm::dot(axis_gs, glm::dvec3(center_gs));

    glm::dvec3 mouse_pos_gs, mouse_dir_gs;
    glm::dvec3 oldMouse_pos_gs, oldMouse_dir_gs;
    camera.getScreenToViewDirection(_mousePos, m_screenSize, mouse_pos_gs, mouse_dir_gs);
    camera.getScreenToViewDirection(m_startCursor, m_screenSize, oldMouse_pos_gs, oldMouse_dir_gs);

    glm::dvec3 newI_gs = tls::math::intersect_line_plane(mouse_pos_gs, mouse_dir_gs, axis_gs, planD);
    glm::dvec3 oldI_gs = tls::math::intersect_line_plane(oldMouse_pos_gs, oldMouse_dir_gs, axis_gs, planD);

    glm::dvec3 newCI_gs = newI_gs - glm::dvec3(center_gs);
    glm::dvec3 oldCI_gs = oldI_gs - glm::dvec3(center_gs);

    glm::dvec3 adjusted_newCI_gs = QGuiApplication::queryKeyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier) ? (1-SLOW_MANIPULATOR) * oldCI_gs + SLOW_MANIPULATOR * newCI_gs : newCI_gs;

    glm::dquat rotation(oldCI_gs, adjusted_newCI_gs);

    double half_cos = rotation.w;
    double sign = (glm::dot(axis_gs, glm::dvec3(rotation.x, rotation.y, rotation.z)) >= 0) ? 1.0 : -1.0;
    double angle = 2 * acos(half_cos) * sign;

    m_startCursor = _mousePos;
    m_distanceToDisplay += tls::math::radiansToDegrees(angle);

    manipData.addRotation = rotation;
    manipData.globalRotationCenter = getTranslation(true);
}

void ManipulatorNode::updateExtrusion(const glm::ivec2& _mousePosition, ManipulateData& manipData, const CameraNode& camera)
{
    // +++++ Additive method -----
    glm::dvec3 gizmo_dir = getWorldAxis();
    glm::dvec3 gizmo_pos = getGizmoAncorage();
    double cursor_on_axis = intersect_with_gizmo_axis(gizmo_pos, gizmo_dir, m_startCursor, camera);
    double current_cursor = intersect_with_gizmo_axis(gizmo_pos, gizmo_dir, _mousePosition, camera);

    double deltaS = (current_cursor - cursor_on_axis) * (QGuiApplication::queryKeyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier) ? SLOW_MANIPULATOR : 1);
    m_startCursor = _mousePosition;

    m_distanceToDisplay += deltaS;

    glm::dvec3 dsize = glm::dvec3(0.);
    bool inv = false;
    switch (m_currentSelection)
    {
    case Selection::_X:
        inv = true;
    case Selection::X:
        dsize = glm::dvec3(deltaS, 0.0, 0.0);
        break;
    case Selection::_Y:
        inv = true;
    case Selection::Y:
        dsize = glm::dvec3(0.0, deltaS, 0.0);
        break;
    case Selection::_Z:
        inv = true;
    case Selection::Z:
        dsize = glm::dvec3(0.0, 0.0, deltaS);
        break;
    }
    dsize /= 2.;

    glm::dvec3 translation =  inv ? -dsize : dsize;
    translation = glm::mat3_cast(getOrientation()) * translation;

    manipData.addScale = dsize;
    manipData.translation = translation;
}

void ManipulatorNode::updateScale(const glm::ivec2& _mousePosition, ManipulateData& manipData, const CameraNode& camera)
{
    glm::dvec2 deltaMouse = _mousePosition - m_startCursor;
    m_startCursor = _mousePosition;

    glm::dvec2 _2dir = getAxisScreenProjection(camera);
    glm::dmat3 rotMat = glm::mat3_cast(getOrientation());

    double mv = glm::dot(_2dir, deltaMouse);
    if (std::isnan(mv) || std::isinf(mv))
        mv = 0.0f;

    glm::dvec3 translation;
    glm::dvec3 dsize;
    switch (m_currentSelection)
    {
    case Selection::_X:
        dsize = glm::dvec3(-mv, 0.0, 0.0);
        break;
    case Selection::X:
        dsize = glm::dvec3(mv, 0.0, 0.0);
        break;
    case Selection::_Y:
        dsize = glm::dvec3(0.0, -mv, 0.0);
        break;
    case Selection::Y:
        dsize = glm::dvec3(0.0, mv, 0.0);
        break;
    case Selection::_Z:
        dsize = glm::dvec3(0.0, 0.0, -mv);
        break;
    case Selection::Z:
        dsize = glm::dvec3(0.0, 0.0, mv);
        break;
    case Selection::XY:
        dsize = glm::dvec3(mv, mv, 0.0);
        break;
    case Selection::X_Y:
        dsize = glm::dvec3(-mv, -mv, 0.0);
        break;
    case Selection::_XY:
        dsize = glm::dvec3(mv, mv, 0.0);
        break;
    case Selection::_X_Y:
        dsize = glm::dvec3(-mv, -mv, 0.0);
        break;
    case Selection::YZ:
        dsize = glm::dvec3(0.0, mv, mv);
        break;
    case Selection::Y_Z:
        dsize = glm::dvec3(0.0, mv, mv);
        break;
    case Selection::_YZ:
        dsize = glm::dvec3(0.0, mv, mv);
        break;
    case Selection::_Y_Z:
        dsize = glm::dvec3(0.0, -mv, -mv);
        break;
    case Selection::XZ:
        dsize = glm::dvec3(mv, 0.0, mv);
        break;
    case Selection::X_Z:
        dsize = glm::dvec3(mv, 0.0, mv);
        break;
    case Selection::_XZ:
        dsize = glm::dvec3(mv, 0.0, mv);
        break;
    case Selection::_X_Z:
        dsize = glm::dvec3(-mv, 0.0, -mv);
        break;
    }

    dsize *= (QGuiApplication::queryKeyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier) ? 0.2 : 1);
    if (m_isLocalManipulation)
        translation = rotMat * translation;

    m_distanceToDisplay += mv;

    manipData.addScale = dsize;
}


glm::dvec2 ManipulatorNode::getAxisScreenProjection(const CameraNode& camera)
{
    glm::dmat4 transfo = camera.getProjMatrix() * camera.getViewMatrix() * computeTransformationMatrix();

    glm::dvec4 pt0(0.0, 0.0, 0.0, 1.0);
    glm::dvec4 pt1;

    //set 'origin' of manipulator
    switch (m_currentSelection)
    {
    case Selection::_X:
    case Selection::X:
        pt1 = glm::dvec4(1.0, 0.0, 0.0, 1.0);
        break;
    case Selection::_Y:
    case Selection::Y:
        pt1 = glm::dvec4(0.0, 1.0, 0.0, 1.0);
        break;
    case Selection::_Z:
    case Selection::Z:
        pt1 = glm::dvec4(0.0, 0.0, 1.0, 1.0);
        break;
    case Selection::XY:
    case Selection::_XY:
    case Selection::X_Y:
    case Selection::_X_Y:
        pt1 = glm::dvec4(1.0, 1.0, 0.0, 1.0);
        break;
    case Selection::YZ:
    case Selection::_YZ:
    case Selection::Y_Z:
    case Selection::_Y_Z:
        pt1 = glm::dvec4(0.0, 1.0, 1.0, 1.0);
        break;
    case Selection::XZ:
    case Selection::_XZ:
    case Selection::X_Z:
    case Selection::_X_Z:
        pt1 = glm::dvec4(1.0, 0.0, 1.0, 1.0);
        break;
    }

    pt0 = transfo * pt0;
    pt0 /= pt0.w;
    pt1 = transfo * pt1;
    pt1 /= pt1.w;

    // NOTE - device coordinates are in [-1,1]. Divide by 2.
    glm::dvec2 dir = glm::dvec2((pt1.x - pt0.x) * m_screenSize.x / 2, (pt1.y - pt0.y) * m_screenSize.y / 2);
    return (dir / length(dir));
}

glm::dvec3 ManipulatorNode::getGizmoAncorage()
{
    glm::dvec4 ancorage(0.0, 0.0, 0.0, 0.0);
    switch (m_manipulationMode)
    {
    case ManipulationMode::Translation:
    case ManipulationMode::Rotation:
        ancorage = glm::dvec4(0.0, 0.0, 0.0, 1.0);
        break;
    case ManipulationMode::Extrusion:
        switch (m_currentSelection)
        {
        case Selection::X:
            ancorage = glm::dvec4(1.0, 0.0, 0.0, 1.0);
            break;
        case Selection::_X:
            ancorage = glm::dvec4(-1.0, 0.0, 0.0, 1.0);
            break;
        case Selection::Y:
            ancorage = glm::dvec4(0.0, 1.0, 0.0, 1.0);
            break;
        case Selection::_Y:
            ancorage = glm::dvec4(0.0, -1.0, 0.0, 1.0);
            break;
        case Selection::Z:
            ancorage = glm::dvec4(0.0, 0.0, 1.0, 1.0);
            break;
        case Selection::_Z:
            ancorage = glm::dvec4(0.0, 0.0, -1.0, 1.0);
            break;
        }
        break;
    default:
        ancorage = glm::dvec4(0.0, 0.0, 0.0, 1.0);
    }

    //if (m_isLocalManipulation)
        return ancorage = computeTransformationMatrix() * ancorage;
    
    //return glm::dvec3(ancorage.x, ancorage.y, ancorage.z);
}

glm::dvec3 ManipulatorNode::getWorldAxis()
{
    glm::dvec3 axis = getLocalAxis();
    glm::dmat3 rotMat = glm::mat3_cast(getOrientation());

    if (isLocalManipulation())
        return rotMat * axis;
    else
        return axis;
}

glm::dvec3 ManipulatorNode::getLocalAxis()
{
    switch (m_currentSelection)
    {
        case Selection::_X:
            return glm::dvec3(-1.0, 0.0, 0.0);
        case Selection::X:
            return glm::dvec3(1.0, 0.0, 0.0);
        case Selection::_Y:
            return glm::dvec3(0.0, -1.0, 0.0);
        case Selection::Y:
            return glm::dvec3(0.0, 1.0, 0.0);
        case Selection::_Z:
            return glm::dvec3(0.0, 0.0, -1.0);
        case Selection::Z:
            return glm::dvec3(0.0, 0.0, 1.0);
        default:
            return glm::dvec3(0.0, 0.0, 0.0);
    }
}

double ManipulatorNode::getManipSizeFactor(double factor)
{
    double userScaleMultiplicator = factor/100 * (MAX_SCALE-MIN_SCALE) + MIN_SCALE;
    userScaleMultiplicator = userScaleMultiplicator < MIN_SCALE ? MIN_SCALE : userScaleMultiplicator;
    userScaleMultiplicator = userScaleMultiplicator > MAX_SCALE ? MAX_SCALE : userScaleMultiplicator;
    return userScaleMultiplicator;
}

void ManipulatorNode::setScanManipulable(bool value)
{
    if (value)
        s_manipulableTypes.insert(ElementType::Scan);
    else
        s_manipulableTypes.erase(ElementType::Scan);
}

std::unordered_set<ElementType> ManipulatorNode::getManipulableTypes()
{
    return s_manipulableTypes;
}

void ManipulatorNode::updateTransfo()
{
    if (m_targets.empty())
        return;

    //setTransformationModule(TransformationModule());

    if (m_targets.size() == 1)
    {
        ReadPtr<AObjectNode> rTarget = m_targets.begin()->cget();
        if (!rTarget || !rTarget->isVisible() || rTarget->isDead())
            return;
        setTransformationModule((TransformationModule)*&rTarget);
    }
    else
    {
        glm::dvec3 center(0);
        uint32_t counter(0);
        for (const SafePtr<AObjectNode>& target : m_targets)
        {
            ReadPtr<AObjectNode> rTarget = target.cget();
            if (!rTarget || !rTarget->isVisible() || rTarget->isDead())
                continue;
            center += rTarget->getTranslation(true);
            counter++;
        }
        if (counter == 0)
            return;
        center /= counter;
        setPosition(center);
    }

    if(m_manipulationMode == ManipulationMode::Rotation || m_manipulationMode == ManipulationMode::Translation)
        if (!glm::any(glm::isnan(m_decalPosition)))
            addLocalTranslation(m_decalPosition);
}

void ManipulatorNode::setTempManipPos(const glm::dvec3& newTempPos)
{
    if (m_manipulationMode != ManipulationMode::Rotation && m_manipulationMode != ManipulationMode::Translation)
        return;
    if (!glm::any(glm::isnan(m_decalPosition)))
        addLocalTranslation(-m_decalPosition);
    m_decalPosition = getInverseRotationTranslation() * glm::dvec4(newTempPos, 1.0);
}

// L.607