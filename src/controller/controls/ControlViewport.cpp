#include "controller/controls/ControlViewport.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/controls/ControlTree.h"
#include "controller/controls/ControlPicking.h"
#include "controller/messages/FullClickMessage.h"
#include "controller/messages/SimpleNumberMessage.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "pointCloudEngine/TlScanOverseer.h"

#include "models/3d/Graph/OpenScanToolsGraphManager.hxx"

namespace control
{
    namespace viewport
    {
        //
        // Examine
        //
        Examine::Examine(const ClickInfo& info)
            : m_clickInfo(info)
        {}

        Examine::Examine(SafePtr<CameraNode> target)
            : m_clickInfo{ 0, 0, false, NAN, 0.0, SafePtr<AObjectNode>(), nullptr, glm::dmat4(), glm::dvec3(), glm::dvec3(), glm::dvec3(), xg::Guid(), target }
        {}

        Examine::~Examine()
        {}

        void Examine::doFunction(Controller& controller)
        {
            controller.getFunctionManager().launchFunction(controller, ContextType::examine);
            if (!isnan(m_clickInfo.fov))
            {
                FullClickMessage message(m_clickInfo);
                controller.getFunctionManager().feedMessage(controller, &message);
            }
        }

        bool Examine::canUndo() const
        {
            return false;
        }

        void Examine::undoFunction(Controller& controller)
        {}
        
        ControlType  Examine::getType() const
        {
            return ControlType::examine;
        }

        //
        // ChangeBackgroundColor
        //

        ChangeBackgroundColor::ChangeBackgroundColor()
        {}

        ChangeBackgroundColor::~ChangeBackgroundColor()
        {}

        void ChangeBackgroundColor::doFunction(Controller& controller)
        {
            controller.updateInfo(new GuiDataRenderBackgroundColor(SafePtr<CameraNode>(), controller.getContext().getNextBackgroundColor()));
        }

        bool ChangeBackgroundColor::canUndo() const
        {
            return(false);
        }

        void ChangeBackgroundColor::undoFunction(Controller& controller)
        {}

        ControlType ChangeBackgroundColor::getType() const
        {
            return ControlType::changeBackgroundColor;
        }

        //
        // AlignViewSide
        //

        AlignViewSide::AlignViewSide(AlignView sideToAlign, SafePtr<CameraNode> destCamera)
            : m_side(sideToAlign)
            , m_destCamera(destCamera)
        {}

        AlignViewSide::~AlignViewSide()
        {}

        void AlignViewSide::doFunction(Controller& controller)
        {
            std::vector<tls::PointCloudInstance> scanInfos = controller.getOpenScanToolsGraphManager().getVisiblePointCloudInstances(xg::Guid(), true, true);
            TlScanOverseer::getInstance().setWorkingScansTransfo(scanInfos);
            tls::BoundingBox projectBoundingBox = TlScanOverseer::getInstance().getActiveBoundingBox();

            controller.updateInfo(new GuiDataRenderAlignView(m_side, projectBoundingBox, m_destCamera, scanInfos.empty()));
        }

        bool AlignViewSide::canUndo() const
        {
            return (false);
        }

        void AlignViewSide::undoFunction(Controller& controller)
        {

        }

        ControlType AlignViewSide::getType() const
        {
            return ControlType::alignViewSide;
        }

        //
        // AlignView2PointsFunction
        //

        AlignView2PointsFunction::AlignView2PointsFunction()
        {}

        AlignView2PointsFunction::~AlignView2PointsFunction()
        {}

        void AlignView2PointsFunction::doFunction(Controller& controller)
        {
            controller.getFunctionManager().launchFunction(controller, ContextType::alignView2P);
        }

        bool AlignView2PointsFunction::canUndo() const
        {
            return (true);
        }

        void AlignView2PointsFunction::undoFunction(Controller& controller)
        {
            controller.getFunctionManager().abort(controller);
        }

        ControlType AlignView2PointsFunction::getType() const
        {
            return ControlType::alignView2PointsFunction;
        }

       //
       // AlignView3PointsFunction
       //

        AlignView3PointsFunction::AlignView3PointsFunction()
        {}

        AlignView3PointsFunction::~AlignView3PointsFunction()
        {}

        void AlignView3PointsFunction::doFunction(Controller& controller)
        {
            controller.getFunctionManager().launchFunction(controller, ContextType::alignView3P);
        }

        bool AlignView3PointsFunction::canUndo() const
        {
            return (true);
        }

        void AlignView3PointsFunction::undoFunction(Controller& controller)
        {
            controller.getFunctionManager().abort(controller);
        }

        ControlType AlignView3PointsFunction::getType() const
        {
            return ControlType::alignView3PointsFunction;
        }

        //
       // AlignViewBoxFunction
       //

        AlignViewBoxFunction::AlignViewBoxFunction()

        {}

        AlignViewBoxFunction::~AlignViewBoxFunction()
        {}

        void AlignViewBoxFunction::doFunction(Controller& controller)
        {
            controller.getFunctionManager().launchFunction(controller, ContextType::alignViewBox);
        }

        bool AlignViewBoxFunction::canUndo() const
        {
            return (true);
        }

        void AlignViewBoxFunction::undoFunction(Controller& controller)
        {
            controller.getFunctionManager().abort(controller);
        }

        ControlType AlignViewBoxFunction::getType() const
        {
            return ControlType::alignViewBoxFunction;
        }

       //
       // MoveManipFunction
       //

        MoveManipFunction::MoveManipFunction()
        {}

        MoveManipFunction::~MoveManipFunction()
        {}

        void MoveManipFunction::doFunction(Controller& controller)
        {
            controller.getFunctionManager().launchFunction(controller, ContextType::moveManip);
        }

        ControlType MoveManipFunction::getType() const
        {
            return ControlType::moveManipFunction;
        }
}
}