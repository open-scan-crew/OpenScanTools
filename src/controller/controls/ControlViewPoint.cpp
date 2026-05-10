#include "controller/controls/ControlViewPoint.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/messages/DataIDListMessage.h"

#include "models/graph/GraphManager.h"
#include "models/graph/AClippingNode.h"
#include "models/graph/ViewPointNode.h"
#include "models/graph/PointCloudNode.h"
#include "models/graph/CameraNode.h"
#include "gui/GuiData/GuiDataUserOrientation.h"

#include "utils/Logger.h"

#include <algorithm>
#include <charconv>

namespace
{
uint32_t getPolygonSuffix(const std::string& name)
{
    constexpr const char* prefix = "polygon_";
    constexpr size_t prefixLen = 8;
    if (name.rfind(prefix, 0) != 0 || name.size() <= prefixLen)
        return 0;

    uint32_t value = 0;
    const char* begin = name.data() + prefixLen;
    const char* end = name.data() + name.size();
    auto result = std::from_chars(begin, end, value);
    if (result.ec != std::errc() || result.ptr != end || value == 0)
        return 0;

    return value;
}

uint32_t computeNextPolygonId(const PolygonalSelectorSettings& settings)
{
    uint32_t maxSuffix = 0;
    for (const PolygonalSelectorPolygon& polygon : settings.polygons)
        maxSuffix = std::max<uint32_t>(maxSuffix, getPolygonSuffix(polygon.name));

    return std::max<uint32_t>(std::max<uint32_t>(settings.nextPolygonId, maxSuffix + 1), 1u);
}

bool supportsViewpointTransform(ElementType type)
{
    return type == ElementType::Box ||
        type == ElementType::Cylinder ||
        type == ElementType::Sphere ||
        type == ElementType::MeshObject ||
        type == ElementType::Tag ||
        type == ElementType::Point ||
        type == ElementType::PCO;
}

bool supportsViewpointClippable(ElementType type)
{
    return type == ElementType::Scan || type == ElementType::PCO;
}

bool supportsViewpointClippingDistances(ElementType type)
{
    return type == ElementType::Box ||
        type == ElementType::Tag ||
        type == ElementType::Point ||
        type == ElementType::Cylinder ||
        type == ElementType::Sphere ||
        type == ElementType::SimpleMeasure ||
        type == ElementType::PolylineMeasure;
}

bool supportsLengthThreshold(ElementType type)
{
    return type == ElementType::Cylinder ||
        type == ElementType::SimpleMeasure ||
        type == ElementType::PolylineMeasure;
}
}

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
        std::unordered_set<SafePtr<AClippingNode>> phaseList;
        std::unordered_set<SafePtr<AClippingNode>> activeList;
        std::unordered_set<SafePtr<AClippingNode>> activeRampList;

        std::unordered_set<SafePtr<AGraphNode>> visibleList;
        std::unordered_map<SafePtr<AGraphNode>, Color32> colorList;
        std::unordered_map<SafePtr<AGraphNode>, bool> clippableList;
        std::unordered_map<SafePtr<AGraphNode>, TransformationModule> transformList;
        std::unordered_map<SafePtr<AGraphNode>, ViewPointData::ClippingDistances> clippingDistancesList;
        std::unordered_map<SafePtr<AGraphNode>, ViewPointData::RampDistances> rampDistancesList;
        bool useUserOrientation = false;
        std::string userOrientationId;

        {
            ReadPtr<ViewPointNode> readViewpoint = m_viewPoint.cget();
            if (!readViewpoint)
                return;

            interiorList = readViewpoint->getInteriorClippings();
            phaseList = readViewpoint->getPhaseClippings();
            activeList = readViewpoint->getActiveClippings();
            activeRampList = readViewpoint->getActiveRamps(); // NEW
            visibleList = readViewpoint->getVisibleObjects();
            colorList = readViewpoint->getScanClusterColors();
            clippableList = readViewpoint->getObjectsClippable();
            transformList = readViewpoint->getObjectsTransform();
            clippingDistancesList = readViewpoint->getObjectsClippingDistances();
            rampDistancesList = readViewpoint->getObjectsRampDistances();
            useUserOrientation = readViewpoint->m_viewpointUserOrientationEnabled;
            userOrientationId = readViewpoint->m_viewpointUserOrientationId;
        }

        GraphManager& graphManager = controller.getGraphManager();

        std::unordered_set<SafePtr<AClippingNode>> clippings = graphManager.getClippingObjects(false, false);

        std::unordered_set<SafePtr<AGraphNode>> editedNodes;

        for (const SafePtr<AClippingNode>& clipping : clippings)
        {
            bool activeState = activeList.find(clipping) != activeList.end();
            ClippingMode clippingMode = ClippingMode::showExterior;
            if (phaseList.find(clipping) != phaseList.end())
                clippingMode = ClippingMode::byPhase;
            else if (interiorList.find(clipping) != interiorList.end())
                clippingMode = ClippingMode::showInterior;

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
            WritePtr<AGraphNode> writeObject = object.get();
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

            if (transformList.find(object) != transformList.end() && supportsViewpointTransform(writeObject->getType()))
            {
                writeObject->setTransformationModule(transformList.at(object));
                editedNodes.insert(object);
            }

            if (clippingDistancesList.find(object) != clippingDistancesList.end() && supportsViewpointClippingDistances(writeObject->getType()))
            {
                const ViewPointData::ClippingDistances& clipValues = clippingDistancesList.at(object);
                AClippingNode* clippingObject = static_cast<AClippingNode*>(writeObject.operator->());
                if (clippingObject->getMinClipDist() != clipValues.minClip)
                    clippingObject->setMinClipDist(clipValues.minClip);
                if (clippingObject->getMaxClipDist() != clipValues.maxClip)
                    clippingObject->setMaxClipDist(clipValues.maxClip);
                if (supportsLengthThreshold(writeObject->getType()) && clippingObject->getLengthThresholdClip() != clipValues.lengthThreshold)
                    clippingObject->setLengthThresholdClip(clipValues.lengthThreshold);
                editedNodes.insert(object);
            }

            if (rampDistancesList.find(object) != rampDistancesList.end() && supportsViewpointClippingDistances(writeObject->getType()))
            {
                const ViewPointData::RampDistances& rampValues = rampDistancesList.at(object);
                AClippingNode* clippingObject = static_cast<AClippingNode*>(writeObject.operator->());
                if (clippingObject->getRampMin() != rampValues.minRamp)
                    clippingObject->setRampMin(rampValues.minRamp);
                if (clippingObject->getRampMax() != rampValues.maxRamp)
                    clippingObject->setRampMax(rampValues.maxRamp);
                if (clippingObject->getRampSteps() != rampValues.stepsRamp)
                    clippingObject->setRampSteps(rampValues.stepsRamp);
                if (writeObject->getType() == ElementType::Box && clippingObject->isRampClamped() != rampValues.rampClamped)
                    clippingObject->setRampClamped(rampValues.rampClamped);
                editedNodes.insert(object);
            }

            if (clippableList.find(object) != clippableList.end() && supportsViewpointClippable(writeObject->getType()))
            {
                PointCloudNode* pointCloud = static_cast<PointCloudNode*>(writeObject.operator->());
                const bool clippableState = clippableList.at(object);
                if (pointCloud->getClippable() != clippableState)
                {
                    pointCloud->setClippable(clippableState);
                    editedNodes.insert(object);
                }
            }
        }

        // Restore user-orientation toolbar state from viewpoint.
        ControllerContext& context = controller.getContext();
        SafePtr<CameraNode> camera = graphManager.getCameraNode();
        WritePtr<CameraNode> wCamera = camera.get();
        if (useUserOrientation && !userOrientationId.empty())
        {
            xg::Guid orientationId(userOrientationId);
            auto it = context.getUserOrientations().find(orientationId);
            if (it != context.getUserOrientations().end())
            {
                if (wCamera)
                {
                    wCamera->setApplyUserOrientation(true);
                    wCamera->setUserOrientation(it->second);
                }
                context.setActiveUserOrientation(it->second);
                controller.updateInfo(new GuiDataSetUserOrientation(it->second));
            }
        }
        else
        {
            if (wCamera)
                wCamera->setApplyUserOrientation(false);
            context.setActiveUserOrientation(UserOrientation());
            controller.updateInfo(new GuiDataUnsetUserOrientation());
        }

        controller.actualizeTreeView(editedNodes);

        CONTROLLOG << "control::viewpoint::UpdateViewPoint undo " << LOGENDL;
    }

    ControlType UpdateStatesFromViewpoint::getType() const
    {
        return ControlType::updateStateFromViewPoint;
    }

    DeletePolygonFromProject::DeletePolygonFromProject(const std::string& polygonName)
        : m_polygonName(polygonName)
    {}

    void DeletePolygonFromProject::doFunction(Controller& controller)
    {
        if (m_polygonName.empty())
            return;

        GraphManager& graphManager = controller.getGraphManager();
        std::unordered_set<SafePtr<AGraphNode>> viewpoints = graphManager.getNodesByTypes({ ElementType::ViewPoint }, ObjectStatusFilter::ALL);
        bool removedAnyPolygon = false;

        for (const SafePtr<AGraphNode>& vpNode : viewpoints)
        {
            SafePtr<ViewPointNode> viewpoint = static_pointer_cast<ViewPointNode>(vpNode);
            WritePtr<ViewPointNode> wViewPoint = viewpoint.get();
            if (!wViewPoint)
                continue;

            PolygonalSelectorSettings& selector = wViewPoint->m_polygonalSelector;
            auto removeIt = std::remove_if(selector.polygons.begin(), selector.polygons.end(),
                [this](const PolygonalSelectorPolygon& polygon) { return polygon.name == m_polygonName; });
            if (removeIt == selector.polygons.end())
                continue;

            selector.polygons.erase(removeIt, selector.polygons.end());
            removedAnyPolygon = true;
            selector.appliedPolygonCount = std::min<uint32_t>(selector.appliedPolygonCount, static_cast<uint32_t>(selector.polygons.size()));
            selector.pendingApply = selector.appliedPolygonCount < selector.polygons.size();
            selector.nextPolygonId = computeNextPolygonId(selector);
            if (selector.polygons.empty())
            {
                selector.enabled = false;
                selector.active = false;
                selector.pendingApply = false;
                selector.highlightedPolygonIndex = -1;
                selector.manageMode = false;
            }
        }

        // ProjectDataChange (A-ready, B-migration anchor)
        // Deletion can be initiated from toolbar settings tied to the active camera, even when
        // no persisted viewpoint selector contains the polygon anymore at this stage.
        controller.getContext().markCurrentProjectDataChanged();

        CONTROLLOG << "control::viewpoint::DeletePolygonFromProject do " << m_polygonName << LOGENDL;
    }

    ControlType DeletePolygonFromProject::getType() const
    {
        return ControlType::deletePolygonFromProject;
    }
}
