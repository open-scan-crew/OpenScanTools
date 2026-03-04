#include "controller/controls/ControlAnimation.h"
#include "controller/Controller.h"
#include "models/graph/AGraphNode.h"
#include "models/graph/GraphManager.h"
#include "models/graph/CameraNode.h"
#include "models/graph/ViewPointNode.h"

#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"

#include <algorithm>


namespace control::animation
{
    namespace
    {
        std::vector<SafePtr<ViewPointNode>> collectPerspectiveViewpointsSorted(Controller& controller)
        {
            std::vector<SafePtr<ViewPointNode>> viewpoints;
            const std::unordered_set<SafePtr<AGraphNode>> allViewpoints = controller.getGraphManager().getNodesByTypes({ ElementType::ViewPoint }, ObjectStatusFilter::ALL);
            viewpoints.reserve(allViewpoints.size());

            for (const SafePtr<AGraphNode>& node : allViewpoints)
            {
                SafePtr<ViewPointNode> viewpoint = static_pointer_cast<ViewPointNode>(node);
                ReadPtr<ViewPointNode> rViewpoint = viewpoint.cget();
                if (!rViewpoint || rViewpoint->getProjectionMode() != ProjectionMode::Perspective)
                    continue;
                viewpoints.push_back(viewpoint);
            }

            std::sort(viewpoints.begin(), viewpoints.end(), [](const SafePtr<ViewPointNode>& a, const SafePtr<ViewPointNode>& b)
                {
                    ReadPtr<ViewPointNode> ra = a.cget();
                    ReadPtr<ViewPointNode> rb = b.cget();
                    if (!ra || !rb)
                        return bool(ra);
                    return ra->getUserIndex() < rb->getUserIndex();
                });

            return viewpoints;
        }
    }

    AddViewPoint::AddViewPoint(SafePtr<AGraphNode> toAdd)
        : m_toAdd(toAdd)
    {}

    void AddViewPoint::doFunction(Controller& controller)
    {
        /*ContextType currentContext(controller.getFunctionManager().isActiveContext());

            

        if (currentContext != ContextType::viewPointAnimation)
        {
            if (currentContext != ContextType::none)
                return;
            controller.getFunctionManager().launchFunction(controller, ContextType::viewPointAnimation);
        }
            
        for (const xg::Guid & id : data)
        {
            Object3D* object = controller.getContext().getCurrentProject()->getObjectOnId<Object3D>(id);
            if (object)
            {
                ClickMessage message(tag->getCenter());
                controller.getFunctionManager().feedMessage(controller, &message);
            }
        }*/
            
    }

    bool AddViewPoint::canUndo() const
    {
        return true;
    }

    void AddViewPoint::undoFunction(Controller& controller)
    {
        //TO Do (Aurélien)
        //send message that remove position form list;
    }

    ControlType AddViewPoint::getType() const
    {
        return ControlType::addAnimationKeyPoint;
    }


    AddScansViewPoint::AddScansViewPoint()
    {}

    AddScansViewPoint::~AddScansViewPoint()
    {}

    void AddScansViewPoint::doFunction(Controller& controller)
    {
        /*
        ContextType currentContext(controller.getFunctionManager().isActiveContext());
        Project* project = controller.getContext().getCurrentProject();
        assert(project);
        std::vector<Scan*> scans(controller.getContext().getCurrentProject()->getObjectsOnType<Scan>(ElementType::Scan));
        struct {
            bool operator()(Scan* a, Scan* b) const { return a->getId() < b->getId(); }
        } dataIdSort;
        std::sort(scans.begin(), scans.end(), dataIdSort);
        for (uint64_t iterator(0); iterator < scans.size(); iterator++)
        {
            ViewPoint animKeyPoint((*scans[iterator]), RenderingParameters());
            controller.updateInfo(new GuiDataRenderAnimationViewPoint(animKeyPoint));
        }
        */
    }
            
    ControlType AddScansViewPoint::getType() const 
    {
        return ControlType::addScansAnimationKeyPoint;
    }

    PrepareViewpointsAnimation::PrepareViewpointsAnimation()
    {}

    PrepareViewpointsAnimation::~PrepareViewpointsAnimation()
    {}

    void PrepareViewpointsAnimation::doFunction(Controller& controller)
    {
        const std::vector<SafePtr<ViewPointNode>> viewpoints = collectPerspectiveViewpointsSorted(controller);
        const bool canStart = viewpoints.size() >= 2;
        controller.updateInfo(new GuiDataRenderAnimationToolbarState(canStart));

        if (!canStart)
        {
            controller.updateInfo(new GuiDataWarning(TEXT_CONTEXT_ANIMATION_NEED_TWO_VIEWPOINTS));
            return;
        }

        WritePtr<CameraNode> wCam = controller.getGraphManager().getCameraNode().get();
        if (!wCam)
            return;

        wCam->cleanAnimation();
        wCam->setLoop(false);
        wCam->setSpeed(3);
        for (const SafePtr<ViewPointNode>& viewpoint : viewpoints)
            wCam->AddViewPoint(viewpoint);
    }

    ControlType PrepareViewpointsAnimation::getType() const
    {
        return ControlType::prepareViewpointsAnimation;
    }

    RefreshViewpointsAnimationState::RefreshViewpointsAnimationState()
    {}

    RefreshViewpointsAnimationState::~RefreshViewpointsAnimationState()
    {}

    void RefreshViewpointsAnimationState::doFunction(Controller& controller)
    {
        const std::vector<SafePtr<ViewPointNode>> viewpoints = collectPerspectiveViewpointsSorted(controller);
        controller.updateInfo(new GuiDataRenderAnimationToolbarState(viewpoints.size() >= 2));
    }

    ControlType RefreshViewpointsAnimationState::getType() const
    {
        return ControlType::refreshViewpointsAnimationState;
    }

}
