#include "models/data/ViewPoint/ViewPointData.h"

#include "models/graph/GraphManager.h"
#include "controller/Controller.h"

#include "models/graph/AClippingNode.h"
#include "models/graph/ViewPointNode.h"
#include "models/graph/PointCloudNode.h"

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
	m_phaseClippings = data.getPhaseClippings();
	m_activeRamps = data.getActiveRamps();
	m_visibleObjects = data.getVisibleObjects();
	m_scanClusterColors = data.getScanClusterColors();
	m_pointCloudObjectsClippable = data.getPointCloudObjectsClippable();
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

void ViewPointData::setPhaseClippings(const std::unordered_set<SafePtr<AClippingNode>>& list)
{
	m_phaseClippings = list;
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

void ViewPointData::setPointCloudObjectsClippable(const std::unordered_map<SafePtr<AGraphNode>, bool>& map)
{
	m_pointCloudObjectsClippable = map;
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

const std::unordered_set<SafePtr<AClippingNode>>& ViewPointData::getPhaseClippings() const
{
	return m_phaseClippings;
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

const std::unordered_map<SafePtr<AGraphNode>, bool>& ViewPointData::getPointCloudObjectsClippable() const
{
	return m_pointCloudObjectsClippable;
}

void ViewPointData::updateViewpointsObjectsValue(Controller& controller, SafePtr<ViewPointNode> viewpoint)
{
	GraphManager& graphManager = controller.getGraphManager();

	std::unordered_set<SafePtr<AClippingNode>> activeClippings = graphManager.getClippingObjects(true, false);
	std::unordered_set<SafePtr<AClippingNode>> interiors;
	std::unordered_set<SafePtr<AClippingNode>> phases;
	for (const SafePtr<AClippingNode>& clip : activeClippings)
	{
		ReadPtr<AClippingNode> rClip = clip.cget();
		if (rClip)
		{
			if (rClip->getClippingMode() == ClippingMode::showInterior)
				interiors.insert(clip);
			else if (rClip->getClippingMode() == ClippingMode::byPhase)
				phases.insert(clip);
		}
	}

	std::unordered_set<SafePtr<AClippingNode>> activeRamps = graphManager.getRampObjects(true, false);

	std::unordered_map<SafePtr<AGraphNode>, Color32> scanClusterColors;
	std::unordered_map<SafePtr<AGraphNode>, bool> pointCloudObjectsClippable;
	for (const SafePtr<AGraphNode>& object : graphManager.getNodesByTypes({ ElementType::Cluster, ElementType::Scan }))
	{
		ReadPtr<AGraphNode> rObject = object.cget();
		if (rObject)
			scanClusterColors[object] = rObject->getColor();
	}

	for (const SafePtr<AGraphNode>& object : graphManager.getNodesByTypes({ ElementType::Scan, ElementType::PCO }))
	{
		ReadPtr<PointCloudNode> rPointCloud = static_pointer_cast<PointCloudNode>(object).cget();
		if (!rPointCloud)
			continue;

		pointCloudObjectsClippable[object] = rPointCloud->getClippable();
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
	wVP->setPhaseClippings(phases);
	wVP->setActiveRamps(activeRamps);
	wVP->setScanClusterColors(scanClusterColors);
	wVP->setPointCloudObjectsClippable(pointCloudObjectsClippable);
	wVP->setVisibleObjects(visible);
}
