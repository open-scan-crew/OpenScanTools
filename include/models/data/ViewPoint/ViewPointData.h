#ifndef VIEWPOINT_DATA_H
#define VIEWPOINT_DATA_H

#include "models/3d/RenderingParameters.h"
#include "utils/safe_ptr.h"
#include "utils/Color32.hpp"
#include "models/graph/TransformationModule.h"

#include <unordered_map>
#include <unordered_set>

class PointCloudNode;
class AGraphNode;
class AClippingNode;
class ViewPointNode;

class Controller;

class ViewPointData : public RenderingParameters
{
public:
	struct ClippingDistances
	{
		float minClip = 0.f;
		float maxClip = 1.f;
		float lengthThreshold = 0.f;
	};

	struct RampDistances
	{
		float minRamp = 0.f;
		float maxRamp = 1.f;
		int stepsRamp = 8;
		bool rampClamped = false;
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
	void setObjectsTransform(const std::unordered_map<SafePtr<AGraphNode>, TransformationModule>& map);
	void setObjectsClippingDistances(const std::unordered_map<SafePtr<AGraphNode>, ClippingDistances>& map);
	void setObjectsRampDistances(const std::unordered_map<SafePtr<AGraphNode>, RampDistances>& map);

	bool isPanoramicScan() const;
	SafePtr<PointCloudNode> getPanoramicScan() const;

	const std::unordered_set<SafePtr<AClippingNode>>& getActiveClippings() const;
	const std::unordered_set<SafePtr<AClippingNode>>& getInteriorClippings() const;
	const std::unordered_set<SafePtr<AClippingNode>>& getPhaseClippings() const;
	const std::unordered_set<SafePtr<AClippingNode>>& getActiveRamps() const;

	const std::unordered_set<SafePtr<AGraphNode>>& getVisibleObjects() const;

	const std::unordered_map<SafePtr<AGraphNode>, Color32>& getScanClusterColors() const;
	const std::unordered_map<SafePtr<AGraphNode>, bool>& getObjectsClippable() const;
	const std::unordered_map<SafePtr<AGraphNode>, TransformationModule>& getObjectsTransform() const;
	const std::unordered_map<SafePtr<AGraphNode>, ClippingDistances>& getObjectsClippingDistances() const;
	const std::unordered_map<SafePtr<AGraphNode>, RampDistances>& getObjectsRampDistances() const;

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
	std::unordered_map<SafePtr<AGraphNode>, TransformationModule> m_objectsTransform;
	std::unordered_map<SafePtr<AGraphNode>, ClippingDistances> m_objectsClippingDistances;
	std::unordered_map<SafePtr<AGraphNode>, RampDistances> m_objectsRampDistances;

};

#endif // !SETTERVIEWPOINTDATA_H_ 
