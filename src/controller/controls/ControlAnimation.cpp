#include "controller/controls/ControlAnimation.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "models/graph/AGraphNode.h"
#include "models/graph/GraphManager.h"
#include "models/graph/CameraNode.h"
#include "models/graph/ViewPointNode.h"
#include "models/3d/DisplayParameters.h"

#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"

#include <algorithm>
#include <limits>
#include <qobject.h>


namespace control::animation
{
    namespace
    {
        std::vector<AnimationViewpointInfo> collectAllViewpointInfos(Controller& controller)
        {
            std::vector<AnimationViewpointInfo> viewpoints;
            const std::unordered_set<SafePtr<AGraphNode>> allViewpoints = controller.getGraphManager().getNodesByTypes({ ElementType::ViewPoint }, ObjectStatusFilter::ALL);
            viewpoints.reserve(allViewpoints.size());

            for (const SafePtr<AGraphNode>& node : allViewpoints)
            {
                SafePtr<ViewPointNode> viewpoint = static_pointer_cast<ViewPointNode>(node);
                ReadPtr<ViewPointNode> rViewpoint = viewpoint.cget();
                if (!rViewpoint)
                    continue;

                AnimationViewpointInfo info;
                info.id = rViewpoint->getId();
                info.name = QString::fromStdWString(rViewpoint->getComposedName());
                info.projectionMode = rViewpoint->getProjectionMode();
                info.renderMode = rViewpoint->m_mode;
                info.blendMode = rViewpoint->m_blendMode;
                info.normals = rViewpoint->m_postRenderingNormals.show;
                info.blendColor = rViewpoint->m_postRenderingNormals.blendColor;
                info.edgeAwareBlur = rViewpoint->m_edgeAwareBlur.enabled;
                info.depthLining = rViewpoint->m_depthLining.enabled;
                info.depthLiningStrongMode = rViewpoint->m_depthLining.strongMode;
                viewpoints.push_back(info);
            }

            std::sort(viewpoints.begin(), viewpoints.end(), [](const AnimationViewpointInfo& a, const AnimationViewpointInfo& b)
                {
                    return a.name.toLower() < b.name.toLower();
                });

            return viewpoints;
        }

        bool normalizeAnimationConfigs(Controller& controller)
        {
            bool changed = false;
            std::unordered_map<xg::Guid, AnimationViewpointInfo> infosById;
            for (const AnimationViewpointInfo& info : collectAllViewpointInfos(controller))
                infosById[info.id] = info;

            std::unordered_map<viewPointAnimationId, ViewPointAnimationConfig>& configs = controller.getContext().getViewPointAnimations();
            for (auto& pair : configs)
            {
                std::vector<ViewPointAnimationLine> normalizedLines;
                double previousPosition = 0.0;
                for (const ViewPointAnimationLine& line : pair.second.getLines())
                {
                    auto itInfo = infosById.find(line.viewpointId);
                    if (itInfo == infosById.end())
                    {
                        changed = true;
                        continue;
                    }

                    ViewPointAnimationLine normalized = line;
                    if (normalized.viewpointName != itInfo->second.name)
                    {
                        normalized.viewpointName = itInfo->second.name;
                        changed = true;
                    }
                    if (normalizedLines.empty())
                    {
                        if (normalized.position != 0.0)
                        {
                            normalized.position = 0.0;
                            changed = true;
                        }
                    }
                    else if (normalized.position < previousPosition)
                    {
                        normalized.position = previousPosition;
                        changed = true;
                    }

                    previousPosition = normalized.position;
                    normalizedLines.push_back(normalized);
                }

                if (normalizedLines.size() != pair.second.getLines().size())
                    changed = true;
                pair.second.setLines(normalizedLines);
            }

            return changed;
        }

        std::vector<ViewPointAnimationConfig> getSortedAnimationConfigs(Controller& controller)
        {
            std::vector<ViewPointAnimationConfig> ordered;
            const std::unordered_map<viewPointAnimationId, ViewPointAnimationConfig>& configs = controller.getContext().cgetViewPointAnimations();
            ordered.reserve(configs.size());
            for (const auto& pair : configs)
                ordered.push_back(pair.second);

            std::sort(ordered.begin(), ordered.end(), [](const ViewPointAnimationConfig& a, const ViewPointAnimationConfig& b)
                {
                    if (a.getOrder() != b.getOrder())
                        return a.getOrder() < b.getOrder();
                    return a.getName().toLower() < b.getName().toLower();
                });

            return ordered;
        }

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

    PrepareViewpointsAnimation::PrepareViewpointsAnimation(const viewPointAnimationId& animationId, int lengthSeconds)
        : m_animationId(animationId)
        , m_lengthSeconds(lengthSeconds)
    {}

    PrepareViewpointsAnimation::~PrepareViewpointsAnimation()
    {}

    void PrepareViewpointsAnimation::doFunction(Controller& controller)
    {
        const std::unordered_map<viewPointAnimationId, ViewPointAnimationConfig>& configs = controller.getContext().cgetViewPointAnimations();
        auto itConfig = configs.find(m_animationId);
        if (itConfig == configs.end())
        {
            controller.updateInfo(new GuiDataRenderAnimationToolbarState(false));
            controller.updateInfo(new GuiDataWarning(TEXT_CONTEXT_ANIMATION_NEED_TWO_VIEWPOINTS));
            return;
        }

        std::unordered_map<xg::Guid, SafePtr<ViewPointNode>> perspectiveById;
        for (const SafePtr<ViewPointNode>& viewpoint : collectPerspectiveViewpointsSorted(controller))
        {
            ReadPtr<ViewPointNode> rViewpoint = viewpoint.cget();
            if (!rViewpoint)
                continue;
            perspectiveById.insert_or_assign(rViewpoint->getId(), viewpoint);
        }

        std::vector<SafePtr<ViewPointNode>> viewpoints;
        std::vector<double> controlTimes;
        viewpoints.reserve(itConfig->second.getLines().size());
        controlTimes.reserve(itConfig->second.getLines().size());

        double previousTime = -std::numeric_limits<double>::infinity();
        for (const ViewPointAnimationLine& line : itConfig->second.getLines())
        {
            auto itVp = perspectiveById.find(line.viewpointId);
            if (itVp == perspectiveById.end())
                continue;

            viewpoints.push_back(itVp->second);
            controlTimes.push_back(line.position);

            if (itConfig->second.getMode() == ViewPointAnimationMode::PositionAsTime)
            {
                if (line.position <= previousTime)
                {
                    controller.updateInfo(new GuiDataWarning(TEXT_CONTEXT_ANIMATION_INCONSISTENT_TIMES));
                    controller.updateInfo(new GuiDataRenderAnimationToolbarState(false));
                    return;
                }
                previousTime = line.position;
            }
        }

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
        wCam->setSpeed(1);
        wCam->setAnimationTiming(itConfig->second.getMode(), static_cast<double>(m_lengthSeconds), controlTimes);
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
        normalizeAnimationConfigs(controller);
        const std::vector<SafePtr<ViewPointNode>> viewpoints = collectPerspectiveViewpointsSorted(controller);
        controller.updateInfo(new GuiDataRenderAnimationToolbarState(viewpoints.size() >= 2));
        controller.updateInfo(new GuiDataSendViewPointAnimationData(getSortedAnimationConfigs(controller), collectAllViewpointInfos(controller)));
    }

    ControlType RefreshViewpointsAnimationState::getType() const
    {
        return ControlType::refreshViewpointsAnimationState;
    }

    CreateEditViewPointAnimation::CreateEditViewPointAnimation(const ViewPointAnimationConfig& config)
        : m_config(config)
    {}

    CreateEditViewPointAnimation::~CreateEditViewPointAnimation()
    {}

    void CreateEditViewPointAnimation::doFunction(Controller& controller)
    {
        std::unordered_map<viewPointAnimationId, ViewPointAnimationConfig>& configs = controller.getContext().getViewPointAnimations();

        for (const auto& pair : configs)
        {
            if (pair.first != m_config.getId() && pair.second.getName().compare(m_config.getName(), Qt::CaseInsensitive) == 0)
            {
                controller.updateInfo(new GuiDataWarning(QObject::tr("Animation name already exists.")));
                return;
            }
        }

        auto inserted = configs.insert_or_assign(m_config.getId(), m_config);
        if (inserted.second)
            configs[m_config.getId()].setOrder(static_cast<uint32_t>(configs.size() - 1));

        normalizeAnimationConfigs(controller);
        controller.updateInfo(new GuiDataSendViewPointAnimationData(getSortedAnimationConfigs(controller), collectAllViewpointInfos(controller)));
    }

    ControlType CreateEditViewPointAnimation::getType() const
    {
        return ControlType::createEditViewPointAnimation;
    }

    DeleteViewPointAnimation::DeleteViewPointAnimation(viewPointAnimationId id)
        : m_id(id)
    {}

    DeleteViewPointAnimation::~DeleteViewPointAnimation()
    {}

    void DeleteViewPointAnimation::doFunction(Controller& controller)
    {
        std::unordered_map<viewPointAnimationId, ViewPointAnimationConfig>& configs = controller.getContext().getViewPointAnimations();
        auto it = configs.find(m_id);
        if (it == configs.end())
            return;

        const uint32_t deletedOrder = it->second.getOrder();
        configs.erase(it);
        for (auto& pair : configs)
        {
            if (pair.second.getOrder() > deletedOrder)
                pair.second.setOrder(pair.second.getOrder() - 1);
        }

        controller.updateInfo(new GuiDataSendViewPointAnimationData(getSortedAnimationConfigs(controller), collectAllViewpointInfos(controller)));
    }

    ControlType DeleteViewPointAnimation::getType() const
    {
        return ControlType::deleteViewPointAnimation;
    }

    SendViewPointAnimationData::SendViewPointAnimationData()
    {}

    SendViewPointAnimationData::~SendViewPointAnimationData()
    {}

    void SendViewPointAnimationData::doFunction(Controller& controller)
    {
        normalizeAnimationConfigs(controller);
        controller.updateInfo(new GuiDataSendViewPointAnimationData(getSortedAnimationConfigs(controller), collectAllViewpointInfos(controller)));
    }

    ControlType SendViewPointAnimationData::getType() const
    {
        return ControlType::sendViewPointAnimationData;
    }

}
