#ifndef ANIMATION_HELPER_H_
#define ANIMATION_HELPER_H_

#include "models/application/ViewPointAnimation.h"
#include "models/graph/ViewPointNode.h"
#include "models/graph/AGraphNode.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <limits>
#include <algorithm>

class Controller;

namespace control::animation::helper
{
    inline std::vector<SafePtr<ViewPointNode>> collectPerspectiveViewpointsSorted(Controller& controller)
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

    inline bool areViewpointsInterpolationCompatible(const ViewPointNode& reference, const ViewPointNode& candidate)
    {
        return reference.m_mode == candidate.m_mode &&
            reference.m_blendMode == candidate.m_blendMode &&
            reference.m_reduceFlash == candidate.m_reduceFlash &&
            reference.m_flashAdvanced == candidate.m_flashAdvanced &&
            reference.m_negativeEffect == candidate.m_negativeEffect &&
            reference.m_postRenderingNormals.show == candidate.m_postRenderingNormals.show &&
            reference.m_postRenderingAmbientOcclusion.enabled == candidate.m_postRenderingAmbientOcclusion.enabled &&
            reference.m_postRenderingNormals.blendColor == candidate.m_postRenderingNormals.blendColor &&
            reference.m_edgeAwareBlur.enabled == candidate.m_edgeAwareBlur.enabled &&
            reference.m_depthLining.enabled == candidate.m_depthLining.enabled &&
            reference.m_depthLining.strongMode == candidate.m_depthLining.strongMode;
    }

    inline bool canInterpolateSelectedViewpoints(const std::vector<SafePtr<ViewPointNode>>& viewpoints)
    {
        if (viewpoints.size() < 2)
            return false;

        ReadPtr<ViewPointNode> rReference = viewpoints.front().cget();
        if (!rReference)
            return false;

        for (size_t i = 1; i < viewpoints.size(); ++i)
        {
            ReadPtr<ViewPointNode> rCandidate = viewpoints[i].cget();
            if (!rCandidate)
                return false;

            if (!areViewpointsInterpolationCompatible(*&rReference, *&rCandidate))
                return false;
        }

        return true;
    }

    inline bool buildAnimationViewpointSequence(
        Controller& controller,
        const viewPointAnimationId& animationId,
        std::vector<SafePtr<ViewPointNode>>& viewpoints,
        std::vector<double>& controlTimes,
        ViewPointAnimationMode& mode)
    {
        viewpoints.clear();
        controlTimes.clear();

        const std::unordered_map<viewPointAnimationId, ViewPointAnimationConfig>& configs = controller.getContext().cgetViewPointAnimations();
        auto itConfig = configs.find(animationId);
        if (itConfig == configs.end())
            return false;

        mode = itConfig->second.getMode();

        std::unordered_map<xg::Guid, SafePtr<ViewPointNode>> perspectiveById;
        for (const SafePtr<ViewPointNode>& viewpoint : collectPerspectiveViewpointsSorted(controller))
        {
            ReadPtr<ViewPointNode> rViewpoint = viewpoint.cget();
            if (!rViewpoint)
                continue;
            perspectiveById.insert_or_assign(rViewpoint->getId(), viewpoint);
        }

        viewpoints.reserve(itConfig->second.getLines().size());
        controlTimes.reserve(itConfig->second.getLines().size());

        double previousTime = -std::numeric_limits<double>::infinity();
        for (const ViewPointAnimationLine& line : itConfig->second.getLines())
        {
            auto itVp = perspectiveById.find(line.viewpointId);
            if (itVp == perspectiveById.end())
                continue;

            if (mode == ViewPointAnimationMode::PositionAsTime && line.position <= previousTime)
                return false;

            viewpoints.push_back(itVp->second);
            controlTimes.push_back(line.position);
            previousTime = line.position;
        }

        return viewpoints.size() >= 2;
    }
}

#endif
