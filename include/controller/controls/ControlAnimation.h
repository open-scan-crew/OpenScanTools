#ifndef CONTROL_ANIMATION_H_
#define CONTROL_ANIMATION_H_

#include "controller/controls/IControl.h"
#include "models/OpenScanToolsModelEssentials.h"

class IPanel;
class AGraphNode;

namespace control::animation
{
    class AddViewPoint : public AControl
    {
    public:
        AddViewPoint(SafePtr<AGraphNode> toAdd);
        ~AddViewPoint() {}
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        SafePtr<AGraphNode> m_toAdd;
    };

    class AddScansViewPoint : public AControl
    {
    public:
        AddScansViewPoint();
        ~AddScansViewPoint();
        void doFunction(Controller& controller) override;
        ControlType getType() const override;
    };

    class Examine : public AControl
    {
    public:
        Examine(const Pos3D& position);
        ~Examine() {}
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        bool			 doneIt;
        const Pos3D position;
    };
}

#endif // !CONTROLANIMATION_H_
