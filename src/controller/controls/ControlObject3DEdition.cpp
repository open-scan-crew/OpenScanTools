#include "controller/Controls/ControlObject3DEdition.h"
#include "controller/Controller.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/messages/ManipulateMessage.h"

#include "models/graph/SphereNode.h"
#include "controller/controls/AEditionControl.hxx"

#include "utils/math/basic_define.h"

namespace control::object3DEdition
{
    /*
    ** SetCenter
    */

    SetCenter::SetCenter(const SafePtr<AGraphNode>& toEditData, const Pos3D& pos)
        : ATEditionControl({ toEditData }, pos, "SetCenter", &TransformationModule::setPosition, &TransformationModule::getCenter)
    {
        setEditCondition([](const AGraphNode& node, const Pos3D& pos) {return (glm::distance(pos, node.getCenter()) > CastEpsilonError); });
    }

    SetCenter::SetCenter(const std::unordered_map<SafePtr<AGraphNode>, Pos3D>& toEditDatas)
        : ATEditionControl(toEditDatas, "SetCenter", & TransformationModule::setPosition, & TransformationModule::getCenter)
    {
        setEditCondition([](const AGraphNode& node, const Pos3D& pos) {return (glm::distance(pos, node.getCenter()) > CastEpsilonError); });
    }

    SetCenter::~SetCenter()
    {}

    ControlType SetCenter::getType() const
    {
        return (ControlType::object3D_set_center);
    }

    /*
    ** SetSize
    */

    SetSize::SetSize(const SafePtr<AGraphNode>& toEditData, const glm::dvec3& size)
        : ATEditionControl({ toEditData }, size, "SetSize", &TransformationModule::setScale, &TransformationModule::getScale)
    {}

    SetSize::~SetSize()
    {}

    ControlType SetSize::getType() const
    {
        return (ControlType::object3D_set_size);
    }

    /*
    ** SetRotation
    */

    SetRotation::SetRotation(const SafePtr<AGraphNode>& toEditData, const glm::dquat& rot)
        : ATEditionControl({ toEditData }, rot, "SetRotation", [](AGraphNode& node, const glm::dquat& rot) {node.setRotation(rot); }, &TransformationModule::getOrientation)
    {
        setEditCondition([](const AGraphNode& data, const glm::dquat& rot) { return !glm::any(glm::isnan(rot)); });
    }

    SetRotation::SetRotation(const std::unordered_map<SafePtr<AGraphNode>, glm::dquat>& toEditDatas)
        : ATEditionControl(toEditDatas, "SetRotation", [](AGraphNode& node, const glm::dquat& rot) {node.setRotation(rot); }, & TransformationModule::getOrientation)
    {
        setEditCondition([](const AGraphNode& data, const glm::dquat& rot) { return !glm::any(glm::isnan(rot)); });
    }

    SetRotation::~SetRotation()
    {
    }

    ControlType SetRotation::getType() const
    {
        return (ControlType::object3D_set_rotation);
    }

    /*
    *  SetTransformation
    */

    SetTransformation::SetTransformation(const SafePtr<AGraphNode>& toEditData, const TransformationModule& transfo)
        : ATEditionControl({ toEditData }, transfo, "SetTransformation", &TransformationModule::setTransformationModule, &TransformationModule::getTransformationModule)
    {}

    SetTransformation::SetTransformation(const std::unordered_set<SafePtr<AGraphNode>>& toEditDatas, const TransformationModule& transfo)
        : ATEditionControl(toEditDatas, transfo, "SetTransformation", &TransformationModule::setTransformationModule, &TransformationModule::getTransformationModule)
    {}

    SetTransformation::~SetTransformation()
    {
    }

    ControlType SetTransformation::getType() const
    {
        return ControlType::object3D_set_transformation;
    }

    /*
    ** Extrude
    */

    /*Extrude::Extrude(SafePtr<AGraphNode> toEditData, double extrusion, Selection side)
        : m_toEditData(toEditData)
        , m_extrusion(extrusion)
        , m_side(side)
    {}


    Extrude::~Extrude()
    {}

    void Extrude::doFunction(Controller& controller)
    {
        if (isnan(m_extrusion))
            return;

        WritePtr<AGraphNode> object = m_toEditData.get();
        if (!object)
        {
            CONTROLLOG << "control::objectEdition::Extrude do : object null" << LOGENDL;
            return;
        }

        glm::dvec3 objectSize = object->getSize();

        switch (m_side)
        {
        case Selection::_X:
        case Selection::X:
            m_extrusion = std::max(m_extrusion, -objectSize.x + 0.00001);
            break;
        case Selection::_Y:
        case Selection::Y:
            m_extrusion = std::max(m_extrusion, -objectSize.y + 0.00001);
            break;
        case Selection::_Z:
        case Selection::Z:
            m_extrusion = std::max(m_extrusion, -objectSize.z + 0.00001);
            break;
        }

        glm::dvec3 translation = glm::dvec3(0);
        glm::dvec3 deltaScale = glm::dvec3(0);
        switch (m_side)
        {
        case Selection::X:
            translation.x = m_extrusion / 2.0;
            deltaScale.x = m_extrusion / 2.0;
            break;
        case Selection::_X:
            translation.x = -m_extrusion / 2.0;
            deltaScale.x = m_extrusion / 2.0;
            break;
        case Selection::Y:
            translation.y = m_extrusion / 2.0;
            deltaScale.y = m_extrusion / 2.0;
            break;
        case Selection::_Y:
            translation.y = -m_extrusion / 2.0;
            deltaScale.y = m_extrusion / 2.0;
            break;
        case Selection::Z:
            translation.z = m_extrusion / 2.0;
            deltaScale.z = m_extrusion / 2.0;
            break;
        case Selection::_Z:
            translation.z = -m_extrusion / 2.0;
            deltaScale.z = m_extrusion / 2.0;
            break;
        }

        object->addLocalTranslation(translation);
        object->addScale(deltaScale);
        doTimeModified(*&object);
    }

    bool Extrude::canUndo() const
    {
        return (false);
    }

    void Extrude::undoFunction(Controller& controller)
    {}

    ControlType Extrude::getType() const
    {
        return (ControlType::object3D_extrude);
    }

    /*
    ** ExtrudeEnd
    */

    ManipulateObjects::ManipulateObjects(const std::unordered_set<SafePtr<AObjectNode>>& targets, const ManipulateData& toAddTransfo, const SafePtr<ManipulatorNode>& manip)
        : m_targets(targets)
        , m_toAddTransfo(toAddTransfo)
        , m_manip(manip)
    {
    }

    void ManipulateObjects::doFunction(Controller& controller)
    {
        std::unordered_set<SafePtr<AGraphNode>> toActualize;
        for (const SafePtr<AObjectNode>& target : m_targets)
            toActualize.insert(target);

        controller.actualizeTreeView(toActualize);
    }

    bool ManipulateObjects::canUndo() const
    {
        return (!m_targets.empty())
            && (m_toAddTransfo.translation != glm::dvec3()
                || m_toAddTransfo.addRotation != glm::dquat()
                || m_toAddTransfo.addScale != glm::dvec3());
    }

    void ManipulateObjects::undoFunction(Controller& controller)
    {
        ManipulateData reverseManip;
        reverseManip.globalRotationCenter = m_toAddTransfo.globalRotationCenter;
        reverseManip.addRotation = glm::inverse(m_toAddTransfo.addRotation);
        reverseManip.addScale = -m_toAddTransfo.addScale;
        reverseManip.translation = -m_toAddTransfo.translation;
        undoRedo(controller, reverseManip);
    }

    void ManipulateObjects::redoFunction(Controller& controller)
    {
        undoRedo(controller, m_toAddTransfo);
    }

    void ManipulateObjects::undoRedo(Controller& controller, const ManipulateData& manipData)
    {
        std::unordered_set<SafePtr<AGraphNode>> toActualize;
        for (const SafePtr<AGraphNode>& object : m_targets)
        {
            toActualize.insert(object);
        }
        controller.actualizeTreeView(toActualize);

        for (const SafePtr<AObjectNode>& target : m_targets)
        {
            WritePtr<AObjectNode> wTarget = target.get();
            if (!wTarget)
                continue;
            wTarget->manipulateTransfo(manipData);
        }

        {
            WritePtr<ManipulatorNode> wManip = m_manip.get();
            if (!wManip)
                return;
            wManip->updateTransfo();
        }
    }

    ControlType ManipulateObjects::getType() const
    {
        return ControlType::manipulation_end;
    }

    /*
    ** ManipulationUpdateUI
    */

    ManipulationUpdateUI::ManipulationUpdateUI(const std::unordered_set<SafePtr<AObjectNode>>& objects, const ManipulateData& transfo)
        : objects_(objects)
        , transfo_(transfo)
    {
    }

    void ManipulationUpdateUI::doFunction(Controller& controller)
    {
        std::unordered_set<SafePtr<AGraphNode>> toActualize;
        for (const SafePtr<AObjectNode>& target : objects_)
            toActualize.insert(target);

        controller.actualizeTreeView(toActualize);
    }

    bool ManipulationUpdateUI::canUndo() const
    {
        return false;
    }

    ControlType ManipulationUpdateUI::getType() const
    {
        return ControlType::manipulation_update_ui;
    }

    /*
    **  SetSphereRadius
    */

    SetSphereRadius::SetSphereRadius(const SafePtr<SphereNode>& toEditData, double radius)
        : ATEditionControl({ toEditData }, radius, "SetSphereRadius", &SphereNode::setRadius, &SphereNode::getRadius)
    {}

    SetSphereRadius::~SetSphereRadius()
    {}

    ControlType SetSphereRadius::getType() const
    {
        return ControlType::object3D_set_sphere_radius;
    }

    LaunchManipulateContext::LaunchManipulateContext(bool rotate, ZMovement zmove)
        : m_rotate(rotate)
        , m_zmove(zmove)
    {}

    LaunchManipulateContext::~LaunchManipulateContext()
    {
    }

    void LaunchManipulateContext::doFunction(Controller& controller)
    {
        controller.getFunctionManager().launchFunction(controller, ContextType::manipulateObjects);

        ManipulateMessage* msg = new ManipulateMessage(m_rotate, m_zmove);
        controller.getFunctionManager().feedMessage(controller, msg);
    }

    bool LaunchManipulateContext::canUndo() const
    {
        return false;
    }

    ControlType LaunchManipulateContext::getType() const
    {
        return ControlType::launchManipulateContext;
    }

    /*ExtrudeEnd::ExtrudeEnd(SafePtr<AGraphNode> toEditData, double value, Selection side)
        : AEditionControl(std::time(nullptr))
        , m_toEditData(toEditData)
        , m_extrusion(value)
        , m_side(side)
        , m_undoDone(false)
    {}


    ExtrudeEnd::~ExtrudeEnd()
    {}

    void ExtrudeEnd::doFunction(Controller& controller)
    {

        if (isnan(m_extrusion))
            return;

        WritePtr<AGraphNode> object = m_toEditData.get();
        if (!object)
        {
            CONTROLLOG << "control::objectEdition::Extrude redo : object null" << LOGENDL;
            return;
        }
            
        if (m_undoDone)
        {
            glm::dvec3 objectSize = object->getSize();

            switch (m_side)
            {
            case Selection::_X:
            case Selection::X:
                m_extrusion = std::max(m_extrusion, -objectSize.x);
                break;
            case Selection::_Y:
            case Selection::Y:
                m_extrusion = std::max(m_extrusion, -objectSize.y);
                break;
            case Selection::_Z:
            case Selection::Z:
                m_extrusion = std::max(m_extrusion, -objectSize.z);
                break;
            }

            glm::dvec3 translation = glm::dvec3(0);
            glm::dvec3 deltaScale = glm::dvec3(0);
            switch (m_side)
            {
            case Selection::X:
                translation.x = m_extrusion / 2.0;
                deltaScale.x = m_extrusion / 2.0;
                break;
            case Selection::_X:
                translation.x = -m_extrusion / 2.0;
                deltaScale.x = m_extrusion / 2.0;
                break;
            case Selection::Y:
                translation.y = m_extrusion / 2.0;
                deltaScale.y = m_extrusion / 2.0;
                break;
            case Selection::_Y:
                translation.y = -m_extrusion / 2.0;
                deltaScale.y = m_extrusion / 2.0;
                break;
            case Selection::Z:
                translation.z = m_extrusion / 2.0;
                deltaScale.z = m_extrusion / 2.0;
                break;
            case Selection::_Z:
                translation.z = -m_extrusion / 2.0;
                deltaScale.z = m_extrusion / 2.0;
                break;
            }

            object->addLocalTranslation(translation);
            object->addScale(deltaScale);
            doTimeModified(*&object);
        }
    }

    bool ExtrudeEnd::canUndo() const
    {
        return !(isnan(m_extrusion));
    }

    void ExtrudeEnd::undoFunction(Controller& controller)
    {
        WritePtr<AGraphNode> object = m_toEditData.get();
        if (!object)
        {
            CONTROLLOG << "control::objectEdition::Extrude undo : object null" << LOGENDL;
            return;
        }

        glm::dvec3 translation = glm::dvec3(0);
        glm::dvec3 deltaScale = glm::dvec3(0);
        switch (m_side)
        {
        case Selection::X:
            translation.x = m_extrusion / 2.0;
            deltaScale.x = m_extrusion / 2.0;
            break;
        case Selection::_X:
            translation.x = -m_extrusion / 2.0;
            deltaScale.x = m_extrusion / 2.0;
            break;
        case Selection::Y:
            translation.y = m_extrusion / 2.0;
            deltaScale.y = m_extrusion / 2.0;
            break;
        case Selection::_Y:
            translation.y = -m_extrusion / 2.0;
            deltaScale.y = m_extrusion / 2.0;
            break;
        case Selection::Z:
            translation.z = m_extrusion / 2.0;
            deltaScale.z = m_extrusion / 2.0;
            break;
        case Selection::_Z:
            translation.z = -m_extrusion / 2.0;
            deltaScale.z = m_extrusion / 2.0;
            break;
        }

        object->addLocalTranslation(-translation);
        object->addScale(-deltaScale);
        undoTimeModified(*&object);

        m_undoDone = true;
    }

    ControlType ExtrudeEnd::getType() const
    {
        return (ControlType::addObject3DExtrudeEditEnd);
    }*/
}