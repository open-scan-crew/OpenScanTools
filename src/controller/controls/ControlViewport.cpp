#include "controller/controls/ControlViewport.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/messages/FullClickMessage.h"
#include "gui/GuiData/GuiDataRendering.h"

#include "models/graph/GraphManager.h"

#include <cmath>

namespace control::viewport
{
    //
    // Examine
    //
    Examine::Examine(const ClickInfo& info)
        : m_clickInfo(info)
    {}

    Examine::Examine(SafePtr<CameraNode> target)
        : m_clickInfo{ 0, 0, false, NAN, 0.0, SafePtr<AGraphNode>(), nullptr, glm::dmat4(), glm::dvec3(), glm::dvec3(), glm::dvec3(), xg::Guid(), target, false }
    {}

    Examine::~Examine()
    {}

    void Examine::doFunction(Controller& controller)
    {
        if (controller.getFunctionManager().isActiveContext() == ContextType::pointsMeasure)
        {
            if (std::isnan(m_clickInfo.picking.x))
                controller.updateInfo(new GuiDataRenderExamine(m_clickInfo.viewport, true));
            else
                controller.updateInfo(new GuiDataRenderExamine(m_clickInfo.viewport, m_clickInfo.picking));
            return;
        }
        controller.getFunctionManager().launchFunction(controller, ContextType::examine);
        if (!std::isnan(m_clickInfo.fov))
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
    // AdjustZoomToScene
    //

    AdjustZoomToScene::AdjustZoomToScene(SafePtr<CameraNode> dest_camera)
        : dest_camera_(dest_camera)
    {}

    void AdjustZoomToScene::doFunction(Controller& controller)
    {
        BoundingBoxD projectBoundingBox = controller.cgetGraphManager().getScanBoundingBox(ObjectStatusFilter::VISIBLE);

        controller.updateInfo(new GuiDataRenderAdjustZoom(projectBoundingBox, dest_camera_));
    }

    ControlType AdjustZoomToScene::getType() const
    {
        return ControlType::adjustZoomToScene;
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
