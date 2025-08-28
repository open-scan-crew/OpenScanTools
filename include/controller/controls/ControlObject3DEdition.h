#ifndef CONTROL_OBJECT3D_EDITION_H
#define CONTROL_OBJECT3D_EDITION_H

#include "models/OpenScanToolsModelEssentials.h"
#include "controller/controls/AEditionControl.h"
#include "models/3d/ManipulationTypes.h"
#include "models/graph/ManipulatorNode.h"
#include <glm/glm.hpp>
#include <unordered_map>

#include "controller/messages/ManipulateMessage.h"

class AGraphNode;
class AObjectNode;
class SphereNode;

namespace control::object3DEdition
{
    class SetCenter : public ATEditionControl<AGraphNode, glm::dvec3>
    {
    public:
        SetCenter(const SafePtr<AGraphNode>& toEditData, const Pos3D& pos);
        SetCenter(const std::unordered_map<SafePtr<AGraphNode>, Pos3D>& toEditDatas);
        ~SetCenter();
        ControlType getType() const override;
    };

    class SetSize : public ATEditionControl<AGraphNode, glm::dvec3>
    {
    public:
        SetSize(const SafePtr<AGraphNode>& toEditData, const glm::dvec3& size);
        ~SetSize();
        ControlType getType() const override;
    };

    class SetRotation : public ATEditionControl<AGraphNode, glm::dquat>
    {
    public:
        SetRotation(const SafePtr<AGraphNode>& toEditData, const glm::dquat& rotationRad);
        SetRotation(const std::unordered_map<SafePtr<AGraphNode>, glm::dquat>& toEditDatas);
        ~SetRotation();
        ControlType getType() const override;
    };

    class SetTransformation : public ATEditionControl<AGraphNode, TransformationModule>
    {
    public:
        SetTransformation(const std::unordered_set<SafePtr<AGraphNode>>& toEditData, const TransformationModule& transfo);
        SetTransformation(const SafePtr<AGraphNode>& toEditData, const TransformationModule& transfo);
        ~SetTransformation();
        ControlType getType() const override;
    };

    class SetSphereRadius : public ATEditionControl<SphereNode, double>
    {
    public:
        SetSphereRadius(const SafePtr<SphereNode>& toEditData, double radius);
        ~SetSphereRadius();
        ControlType getType() const override;
    };

    class ManipulateObjects : public AEditionControl
    {
    public:
        ManipulateObjects(const std::unordered_set<SafePtr<AObjectNode>>& targets, const ManipulateData& toAddTransfo, const SafePtr<ManipulatorNode>& manip);
        ~ManipulateObjects() {};
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        void redoFunction(Controller& controller) override;
        void undoRedo(Controller& controller, const ManipulateData& manipData);
        ControlType getType() const override;
    private:
        std::unordered_set<SafePtr<AObjectNode>> m_targets;
        ManipulateData m_toAddTransfo;
        SafePtr<ManipulatorNode> m_manip;
    };

    class ManipulationUpdateUI : public AControl
    {
    public:
        ManipulationUpdateUI(const std::unordered_set<SafePtr<AObjectNode>>& objects, const ManipulateData& transfo);
        ~ManipulationUpdateUI() {};
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        ControlType getType() const override;
    private:
        std::unordered_set<SafePtr<AObjectNode>> objects_;
        ManipulateData transfo_;
    };

    class LaunchManipulateContext : public AEditionControl
    {
    public:
        LaunchManipulateContext(bool rotate, ZMovement zmove);
        ~LaunchManipulateContext();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        ControlType getType() const override;
    private:
        bool m_rotate;
        ZMovement m_zmove;

    };

    /*class Extrude : public AEditionControl
    {
    public:
        Extrude(SafePtr<AGraphNode> id, double value, Selection side);
        ~Extrude();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        SafePtr<AGraphNode>	 m_toEditData;
        double		 m_extrusion;
        const Selection m_side;
    };

    class ExtrudeEnd : public AEditionControl
    {
    public:
        ExtrudeEnd(SafePtr<AGraphNode> id, double value, Selection side);
        ~ExtrudeEnd();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        SafePtr<AGraphNode> m_toEditData;
        double m_extrusion;
        const Selection m_side;
        bool m_undoDone;
    };*/
}

#endif