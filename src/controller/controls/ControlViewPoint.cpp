#include "controller/controls/ControlViewPoint.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "controller/functionSystem/FunctionManager.h"

#include "utils/Logger.h"
#include "controller/messages/DataIDListMessage.h"

#include "models/graph/GraphManager.hxx"
#include "models/graph/AClippingNode.h"
#include "models/graph/ViewPointNode.h"
#include "models/graph/CameraNode.h"

namespace control::viewpoint
{
    /*
    * LaunchCreationContext
    */

    LaunchCreationContext::LaunchCreationContext()
    {}

    LaunchCreationContext::~LaunchCreationContext()
    {}

    void LaunchCreationContext::doFunction(Controller& controller)
    {
        controller.getFunctionManager().launchFunction(controller, ContextType::viewpointCreation);
        CONTROLLOG << "control::viewpoint::LaunchCreationContext do " << LOGENDL;
    }

    bool LaunchCreationContext::canUndo() const
    {
        return (false);
    }

    void LaunchCreationContext::undoFunction(Controller& controller)
    {}

    ControlType LaunchCreationContext::getType() const
    {
        return ControlType::contextViewPointCreation;
    }

    /*
    * LaunchUpdateContext
    */

    LaunchUpdateContext::LaunchUpdateContext()
        : m_viewpointToUpdate()
    {}

    LaunchUpdateContext::LaunchUpdateContext(SafePtr<AGraphNode> viewpointToUpdate)
        : m_viewpointToUpdate(viewpointToUpdate)
    {}

    LaunchUpdateContext::~LaunchUpdateContext()
    {}

    void LaunchUpdateContext::doFunction(Controller& controller)
    {
        if (!m_viewpointToUpdate.cget())
        {
            GraphManager& graphManager = controller.getGraphManager();

            std::unordered_set<SafePtr<AGraphNode>> viewpointToUpdate = graphManager.getNodesByTypes({ ElementType::ViewPoint }, ObjectStatusFilter::SELECTED);
            if (viewpointToUpdate.size() != 1)
            {
                CONTROLLOG << "control::viewpoint::LaunchUpdateContext do  wrong selection" << LOGENDL;
                return;
            }
            m_viewpointToUpdate = *(viewpointToUpdate.begin());
        }
        controller.getFunctionManager().launchFunction(controller, ContextType::viewpointUpdate);
        DataListMessage message({ m_viewpointToUpdate }, ElementType::ViewPoint);
        controller.getFunctionManager().feedMessage(controller, &message);
        CONTROLLOG << "control::viewpoint::LaunchUpdateContext do " << LOGENDL;
    }

    bool LaunchUpdateContext::canUndo() const
    {
        return (false);
    }

    void LaunchUpdateContext::undoFunction(Controller& controller)
    {}

    ControlType LaunchUpdateContext::getType() const
    {
        return ControlType::contextViewPointUpdate;
    }

    /*
    * UpdateViewPoint
    */

    UpdateViewPoint::UpdateViewPoint(SafePtr<ViewPointNode> viewpointToUpdate, SafePtr<CameraNode> updateCamera, bool canUndo)
        : m_updateCamera(updateCamera)
        , m_viewpointToUpdate(viewpointToUpdate)
        , m_canUndo(canUndo)
    {}

    void UpdateViewPoint::doFunction(Controller& controller)
    {
        {
            ReadPtr<CameraNode> rCameraInfos = m_updateCamera.cget();
            if (!rCameraInfos)
                return;

            WritePtr<ViewPointNode> wViewPoint = m_viewpointToUpdate.get();
            if (!wViewPoint)
                return;

            m_undoRedoTransfo = *&wViewPoint;
            m_undoRedoViewPointData = *&wViewPoint;

            wViewPoint->copyViewPointData(ViewPointData(*&rCameraInfos, rCameraInfos->getPanoramicScan()));
            wViewPoint->setTransformationModule(*&rCameraInfos);
        }

        ViewPointData::updateViewpointsObjectsValue(controller, m_viewpointToUpdate);

        CONTROLLOG << "control::viewpoint::UpdateViewPoint do " << LOGENDL;
    }

    bool UpdateViewPoint::canUndo() const
    {
        return m_canUndo;
    }

    void UpdateViewPoint::undoFunction(Controller& controller)
    {
        WritePtr<ViewPointNode> wViewPoint = m_viewpointToUpdate.get();
        if (!wViewPoint)
            return;

        ViewPointData newViewPointData = m_undoRedoViewPointData;
        TransformationModule newTransfoData = m_undoRedoTransfo;

        m_undoRedoTransfo = *&wViewPoint;
        m_undoRedoViewPointData = *&wViewPoint;

        wViewPoint->copyViewPointData(newViewPointData);
        wViewPoint->setTransformationModule(newTransfoData);

        //controller.actualizeOnId(m_viewpointToUpdate, true);
        CONTROLLOG << "control::viewpoint::UpdateViewPoint undo " << LOGENDL;
    }

    void UpdateViewPoint::redoFunction(Controller& controller)
    {
        undoFunction(controller);
    }

    ControlType UpdateViewPoint::getType() const
    {
        return ControlType::updateViewPoint;
    }

    /*
    * UpdateStatesFromViewpoint
    */

    UpdateStatesFromViewpoint::UpdateStatesFromViewpoint(SafePtr<ViewPointNode> viewpoint)
        : m_viewPoint(viewpoint)
    {}

    UpdateStatesFromViewpoint::~UpdateStatesFromViewpoint()
    {}

    void UpdateStatesFromViewpoint::doFunction(Controller& controller)
    {
        std::unordered_set<SafePtr<AClippingNode>> interiorList;
        std::unordered_set<SafePtr<AClippingNode>> activeList;
        std::unordered_set<SafePtr<AClippingNode>> activeRampList;

        std::unordered_set<SafePtr<AGraphNode>> visibleList;
        std::unordered_map<SafePtr<AGraphNode>, Color32> colorList;

        {
            ReadPtr<ViewPointNode> readViewpoint = m_viewPoint.cget();
            if (!readViewpoint)
                return;

            interiorList = readViewpoint->getInteriorClippings();
            activeList = readViewpoint->getActiveClippings();
            activeRampList = readViewpoint->getActiveRamps(); // NEW
            visibleList = readViewpoint->getVisibleObjects();
            colorList = readViewpoint->getScanClusterColors();
        }

        GraphManager& graphManager = controller.getGraphManager();

        std::unordered_set<SafePtr<AClippingNode>> clippings = graphManager.getClippingObjects(false, false);

        std::unordered_set<SafePtr<AGraphNode>> editedNodes;

        for (const SafePtr<AClippingNode>& clipping : clippings)
        {
            bool activeState = activeList.find(clipping) != activeList.end();
            ClippingMode clippingMode = interiorList.find(clipping) != interiorList.end() ? ClippingMode::showInterior : ClippingMode::showExterior;

            WritePtr<AClippingNode> writeClipping = clipping.get();

            if (writeClipping->isClippingActive() != activeState || writeClipping->getClippingMode() != clippingMode)
            {
                writeClipping->setClippingMode(clippingMode);
                writeClipping->setClippingActive(activeState);
                editedNodes.insert(clipping);
            }
        }

        // NEW - Ramp management
        std::unordered_set<SafePtr<AClippingNode>> ramps = graphManager.getRampObjects(false, false);
        for (const SafePtr<AClippingNode>& ramp : ramps)
        {
            bool activeState = activeRampList.find(ramp) != activeRampList.end();

            WritePtr<AClippingNode> writeRamp = ramp.get();
            if (writeRamp->isRampActive() != activeState)
            {
                writeRamp->setRampActive(activeState);
                editedNodes.insert(ramp);
            }
        }

        for (const SafePtr<AGraphNode>& object : graphManager.getProjectNodes())
        {
            WritePtr<AObjectNode> writeObject = static_pointer_cast<AObjectNode>(object).get();
            if (!writeObject)
                continue;

            bool visibleState = (visibleList.find(object) != visibleList.end());
            if (writeObject->isVisible() != visibleState)
            {
                writeObject->setVisible(visibleState);
                editedNodes.insert(object);
            }

            if (colorList.find(object) != colorList.end())
            {
                writeObject->setColor(colorList.at(object));
                editedNodes.insert(object);
            }
        }

            controller.actualizeTreeView(editedNodes);

        CONTROLLOG << "control::viewpoint::UpdateViewPoint undo " << LOGENDL;
    }

    ControlType UpdateStatesFromViewpoint::getType() const
    {
        return ControlType::updateStateFromViewPoint;
    }
}
