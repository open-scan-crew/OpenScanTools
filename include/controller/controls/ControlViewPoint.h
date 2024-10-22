#ifndef CONTROL_VIEWPOINT_H_
#define CONTROL_VIEWPOINT_H_

#include "controller/controls/IControl.h"
#include "models/graph/TransformationModule.h"
#include "models/data/ViewPoint/ViewPointData.h"

class CameraNode;
class ViewPointNode;

namespace control::viewpoint
{
    class LaunchCreationContext : public AControl
    {
    public:
        LaunchCreationContext();
        ~LaunchCreationContext();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    };

    class LaunchUpdateContext : public AControl
    {
    public:
        LaunchUpdateContext();
        LaunchUpdateContext(SafePtr<AGraphNode> viewpointToUpdate);
        ~LaunchUpdateContext();
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        SafePtr<AGraphNode> m_viewpointToUpdate;
    };

    class UpdateViewPoint : public AControl
    {
    public:
        UpdateViewPoint(SafePtr<ViewPointNode> viewpointToUpdate, SafePtr<CameraNode> updateCamera, bool canUndo);
        ~UpdateViewPoint() {};
        void doFunction(Controller& controller) override;
        bool canUndo() const override;
        void undoFunction(Controller& controller) override;
        void redoFunction(Controller& controller) override;
        ControlType getType() const override;
    private:
        SafePtr<ViewPointNode> m_viewpointToUpdate;
        SafePtr<CameraNode> m_updateCamera;

        ViewPointData m_undoRedoViewPointData;
        TransformationModule m_undoRedoTransfo;

        bool m_canUndo = false;
    };

    class UpdateStatesFromViewpoint : public AControl
    {
        public:
            UpdateStatesFromViewpoint(SafePtr<ViewPointNode> viewpoint);
            ~UpdateStatesFromViewpoint();
            void doFunction(Controller& controller) override;
            ControlType getType() const override;
        private:
            SafePtr<ViewPointNode> m_viewPoint;
    };
}
#endif //! CONTROL_VIEWPOINT_H_