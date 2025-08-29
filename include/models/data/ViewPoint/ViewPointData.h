#ifndef VIEWPOINT_DATA_H
#define VIEWPOINT_DATA_H

#include "models/3d/RenderingParameters.h"
#include "utils/safe_ptr.h"
#include "utils/Color32.hpp"

#include <unordered_set>

class ScanNode;
class AGraphNode;
class AClippingNode;
class ViewPointNode;

class Controller;

class ViewPointData : public RenderingParameters
{
public:
	ViewPointData();
	ViewPointData(const RenderingParameters& data, const SafePtr<ScanNode>& panoramicScan);
	~ViewPointData();

	void copyViewPointData(const ViewPointData& data);

	void setPanoramicScan(SafePtr<ScanNode> panoScan);

	void setActiveClippings(const std::unordered_set<SafePtr<AClippingNode>>& list);
	void setInteriorClippings(const std::unordered_set<SafePtr<AClippingNode>>& list);
	void setActiveRamps(const std::unordered_set<SafePtr<AClippingNode>>& list);

	void setVisibleObjects(const std::unordered_set<SafePtr<AGraphNode>>& list);

	void setScanClusterColors(const std::unordered_map<SafePtr<AGraphNode>, Color32>& map);

	bool isPanoramicScan() const;
	SafePtr<ScanNode> getPanoramicScan() const;

	const std::unordered_set<SafePtr<AClippingNode>>& getActiveClippings() const;
	const std::unordered_set<SafePtr<AClippingNode>>& getInteriorClippings() const;
	const std::unordered_set<SafePtr<AClippingNode>>& getActiveRamps() const;

	const std::unordered_set<SafePtr<AGraphNode>>& getVisibleObjects() const;

	const std::unordered_map<SafePtr<AGraphNode>, Color32>& getScanClusterColors() const;

	static void updateViewpointsObjectsValue(Controller& controller, SafePtr<ViewPointNode> viewpoint);

protected:
	SafePtr<ScanNode> m_panoramicScan;

	std::unordered_set<SafePtr<AClippingNode>> m_activeClippings;
	std::unordered_set<SafePtr<AClippingNode>> m_interiorClippings;
	std::unordered_set<SafePtr<AClippingNode>> m_activeRamps;

	std::unordered_set<SafePtr<AGraphNode>> m_visibleObjects;

	std::unordered_map<SafePtr<AGraphNode>, Color32> m_scanClusterColors;

};

#endif // !SETTERVIEWPOINTDATA_H_ 
