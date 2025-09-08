#include "models/data/ViewPoint/ViewPointData.h"

#include "models/graph/GraphManager.h"
#include "controller/Controller.h"

#include "models/graph/AClippingNode.h"
#include "models/graph/ViewPointNode.h"

ViewPointData::ViewPointData()
{}

ViewPointData::ViewPointData(const RenderingParameters& data, const SafePtr<PointCloudNode>& panoramicScan)
	: RenderingParameters(data)
	, m_panoramicScan(panoramicScan)
{}

ViewPointData::~ViewPointData()
{}

void ViewPointData::copyViewPointData(const ViewPointData& data)
{
	setRenderingParameters(data);
	m_panoramicScan = data.getPanoramicScan();
	m_activeClippings = data.getActiveClippings();
	m_interiorClippings = data.getInteriorClippings();
	m_activeRamps = data.getActiveRamps();
	m_visibleObjects = data.getVisibleObjects();
	m_scanClusterColors = data.getScanClusterColors();
}

void ViewPointData::setPanoramicScan(SafePtr<PointCloudNode> id)
{
	m_panoramicScan = id;
}

void ViewPointData::setActiveClippings(const std::unordered_set<SafePtr<AClippingNode>>& list)
{
	m_activeClippings = list;
}

void ViewPointData::setInteriorClippings(const std::unordered_set<SafePtr<AClippingNode>>& list)
{
	m_interiorClippings = list;
}

void ViewPointData::setActiveRamps(const std::unordered_set<SafePtr<AClippingNode>>& list)
{
	m_activeRamps = list;
}

void ViewPointData::setVisibleObjects(const std::unordered_set<SafePtr<AGraphNode>>& list)
{
	m_visibleObjects = list;
}

void ViewPointData::setScanClusterColors(const std::unordered_map<SafePtr<AGraphNode>, Color32>& map)
{
	m_scanClusterColors = map;
}

bool ViewPointData::isPanoramicScan() const
{
	return bool(m_panoramicScan); //Rajouter une vérification isNull const
}

SafePtr<PointCloudNode> ViewPointData::getPanoramicScan() const
{
	return m_panoramicScan;
}

const std::unordered_set<SafePtr<AClippingNode>>& ViewPointData::getActiveClippings() const
{
	return m_activeClippings;
}

const std::unordered_set<SafePtr<AClippingNode>>& ViewPointData::getInteriorClippings() const
{
	return m_interiorClippings;
}

const std::unordered_set<SafePtr<AClippingNode>>& ViewPointData::getActiveRamps() const
{
	return m_activeRamps;
}

//std::unordered_set<xg::Guid> ViewPointData::getActiveScans() const
//{
//    return m_activeScans;
//}

const std::unordered_set<SafePtr<AGraphNode>>& ViewPointData::getVisibleObjects() const
{
	return m_visibleObjects;
}

const std::unordered_map<SafePtr<AGraphNode>, Color32>& ViewPointData::getScanClusterColors() const
{
	return m_scanClusterColors;
}

void ViewPointData::updateViewpointsObjectsValue(Controller& controller, SafePtr<ViewPointNode> viewpoint)
{
	GraphManager& graphManager = controller.getGraphManager();

	std::unordered_set<SafePtr<AClippingNode>> activeClippings = graphManager.getClippingObjects(true, false);
	std::unordered_set<SafePtr<AClippingNode>> interiors;
	for (const SafePtr<AClippingNode>& clip : activeClippings)
	{
		ReadPtr<AClippingNode> rClip = clip.cget();
		if (rClip && rClip->getClippingMode() == ClippingMode::showInterior)
			interiors.insert(clip);
	}

	std::unordered_set<SafePtr<AClippingNode>> activeRamps = graphManager.getRampObjects(true, false);

	std::unordered_map<SafePtr<AGraphNode>, Color32> scanClusterColors;
	for (const SafePtr<AGraphNode>& object : graphManager.getNodesByTypes({ ElementType::Cluster, ElementType::Scan }))
	{
		ReadPtr<AGraphNode> rObject = object.cget();
		if (rObject)
			scanClusterColors[object] = rObject->getColor();
	}

	std::unordered_set<SafePtr<AGraphNode>> visible;
	for (const SafePtr<AGraphNode>& object : graphManager.getProjectNodes())
		if (object.cget()->isVisible())
			visible.insert(object);

	WritePtr<ViewPointNode> wVP = viewpoint.get();
	if (!wVP)
		return;

	wVP->setActiveClippings(activeClippings);
	wVP->setInteriorClippings(interiors);
	wVP->setActiveRamps(activeRamps);
	wVP->setScanClusterColors(scanClusterColors);
	wVP->setVisibleObjects(visible);
}
