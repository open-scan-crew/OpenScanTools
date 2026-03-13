#ifndef CONTROL_ANIMATION_H_
#define CONTROL_ANIMATION_H_

#include "controller/controls/IControl.h"
#include "models/OpenScanToolsModelEssentials.h"
#include "models/application/ViewPointAnimation.h"

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

    class PrepareViewpointsAnimation : public AControl
    {
    public:
        PrepareViewpointsAnimation(const viewPointAnimationId& animationId, int lengthSeconds);
        ~PrepareViewpointsAnimation();
        void doFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        viewPointAnimationId m_animationId;
        int m_lengthSeconds;
    };

    class RefreshViewpointsAnimationState : public AControl
    {
    public:
        RefreshViewpointsAnimationState();
        ~RefreshViewpointsAnimationState();
        void doFunction(Controller& controller) override;
        ControlType getType() const override;
    };

    class CreateEditViewPointAnimation : public AControl
    {
    public:
        CreateEditViewPointAnimation(const ViewPointAnimationConfig& config);
        ~CreateEditViewPointAnimation();
        void doFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        ViewPointAnimationConfig m_config;
    };

    class DeleteViewPointAnimation : public AControl
    {
    public:
        DeleteViewPointAnimation(viewPointAnimationId id);
        ~DeleteViewPointAnimation();
        void doFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        viewPointAnimationId m_id;
    };

    class SendViewPointAnimationData : public AControl
    {
    public:
        SendViewPointAnimationData();
        ~SendViewPointAnimationData();
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
