#ifndef VIEWPOINT_DATA_H
#define VIEWPOINT_DATA_H

#include "models/3d/RenderingParameters.h"
#include "utils/safe_ptr.h"
#include "utils/Color32.hpp"
#include "models/graph/TransformationModule.h"
#include "models/data/Clipping/ClippingData.h"

#include <unordered_map>
#include <unordered_set>
#include <optional>

class PointCloudNode;
class AGraphNode;
class AClippingNode;
class ViewPointNode;

class Controller;

class ViewPointData : public RenderingParameters
{
public:
	struct ObjectState
	{
		bool visible = true;

		std::optional<Color32> color;

		std::optional<glm::dvec3> center;
		std::optional<glm::dquat> orientation;
		std::optional<glm::dvec3> scale;

		std::optional<bool> clippable;

		std::optional<ClippingMode> clippingMode;
		std::optional<bool> clippingActive;
		std::optional<float> minClipDist;
		std::optional<float> maxClipDist;
		std::optional<float> lengthThresholdClip;

		std::optional<bool> rampActive;
		std::optional<float> rampMin;
		std::optional<float> rampMax;
		std::optional<int> rampSteps;
		std::optional<bool> rampClamped;
	};

	ViewPointData();
	ViewPointData(const RenderingParameters& data, const SafePtr<PointCloudNode>& panoramicScan);
	~ViewPointData();

	void copyViewPointData(const ViewPointData& data);

	void setPanoramicScan(SafePtr<PointCloudNode> panoScan);

	void setActiveClippings(const std::unordered_set<SafePtr<AClippingNode>>& list);
	void setInteriorClippings(const std::unordered_set<SafePtr<AClippingNode>>& list);
	void setPhaseClippings(const std::unordered_set<SafePtr<AClippingNode>>& list);
	void setActiveRamps(const std::unordered_set<SafePtr<AClippingNode>>& list);

	void setVisibleObjects(const std::unordered_set<SafePtr<AGraphNode>>& list);

	void setScanClusterColors(const std::unordered_map<SafePtr<AGraphNode>, Color32>& map);
	void setObjectsClippable(const std::unordered_map<SafePtr<AGraphNode>, bool>& map);
	void setObjectStates(const std::unordered_map<SafePtr<AGraphNode>, ObjectState>& map);

	bool isPanoramicScan() const;
	SafePtr<PointCloudNode> getPanoramicScan() const;

	const std::unordered_set<SafePtr<AClippingNode>>& getActiveClippings() const;
	const std::unordered_set<SafePtr<AClippingNode>>& getInteriorClippings() const;
	const std::unordered_set<SafePtr<AClippingNode>>& getPhaseClippings() const;
	const std::unordered_set<SafePtr<AClippingNode>>& getActiveRamps() const;

	const std::unordered_set<SafePtr<AGraphNode>>& getVisibleObjects() const;

	const std::unordered_map<SafePtr<AGraphNode>, Color32>& getScanClusterColors() const;
	const std::unordered_map<SafePtr<AGraphNode>, bool>& getObjectsClippable() const;
	const std::unordered_map<SafePtr<AGraphNode>, ObjectState>& getObjectStates() const;

	static void updateViewpointsObjectsValue(Controller& controller, SafePtr<ViewPointNode> viewpoint);

protected:
	SafePtr<PointCloudNode> m_panoramicScan;

	std::unordered_set<SafePtr<AClippingNode>> m_activeClippings;
	std::unordered_set<SafePtr<AClippingNode>> m_interiorClippings;
	std::unordered_set<SafePtr<AClippingNode>> m_phaseClippings;
	std::unordered_set<SafePtr<AClippingNode>> m_activeRamps;

	std::unordered_set<SafePtr<AGraphNode>> m_visibleObjects;

	std::unordered_map<SafePtr<AGraphNode>, Color32> m_scanClusterColors;
	std::unordered_map<SafePtr<AGraphNode>, bool> m_objectsClippable;
	std::unordered_map<SafePtr<AGraphNode>, ObjectState> m_objectStates;

};

#endif // !SETTERVIEWPOINTDATA_H_ 
