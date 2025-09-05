#ifndef CONTROL_CYLINDER_EDITION_H
#define CONTROL_CYLINDER_EDITION_H

#include "controller/controls/AEditionControl.h"
#include "models/data/Piping/StandardRadiusData.h"

class CylinderNode;
class AGraphNode;
class SphereNode;

namespace control::cylinderEdition
{
    class SetForcedRadius : public AEditionControl
    {
    public:
        SetForcedRadius(SafePtr<CylinderNode> toEditData, const double& radius);
        ~SetForcedRadius();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        SafePtr<CylinderNode>  m_toEditData;
        double m_newRadius;
        double m_oldRadius;
        StandardRadiusData::DiameterSet m_oldDiameterSet;
    };

    class SetDetectedRadius : public AEditionControl
    {
    public:
        SetDetectedRadius(SafePtr<CylinderNode> toEditData);
        ~SetDetectedRadius();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        SafePtr<CylinderNode>  m_toEditData;
        double m_oldRadius;
        StandardRadiusData::DiameterSet m_oldDiameterSet;
    };

    class SetStandard : public AEditionControl
    {
    public:
        SetStandard(SafePtr<CylinderNode> toEditData, const SafePtr<StandardList>& standard);
        ~SetStandard();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        SafePtr<CylinderNode>  m_toEditData;
        SafePtr<StandardList> m_standard;
        SafePtr<StandardList> m_oldStandard;
        StandardRadiusData::DiameterSet m_oldDiameterSet;
    };

    class SetLength : public ATEditionControl<CylinderNode, double>
    {
    public:
        SetLength(SafePtr<CylinderNode> toEditData, const double& larger);
        SetLength(const std::unordered_set<SafePtr<CylinderNode>>& toEditDatas, const double& larger);
        ~SetLength();
        ControlType getType() const override;
    };

}

#endif // !CONTROL_FUNCTION_CYLINDER_H_