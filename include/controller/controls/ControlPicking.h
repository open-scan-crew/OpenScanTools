#ifndef CONTROL_PICKING_H_
#define CONTROL_PICKING_H_

#include "controller/controls/IControl.h"
#include "models/3d/ClickInfo.h"

namespace control::picking
{
    class Click : public AControl
    {
    public:
        Click(const ClickInfo& info);
        ~Click();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        ClickInfo m_clickInfo;
    };

    class FindScanFromPick : public AControl
    {
    public:
        FindScanFromPick();
        ~FindScanFromPick();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    };

}

#endif // !CONTROL_PICKING_H_