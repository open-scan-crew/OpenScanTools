#include "models/data/ViewPoint/ViewPointData.h"

#include "models/graph/GraphManager.h"
#include "controller/Controller.h"

#include "models/graph/AClippingNode.h"
#include "models/graph/ViewPointNode.h"
#include "models/graph/PointCloudNode.h"

namespace
{
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

	bool supportsViewpointColor(ElementType type)
	{
		return type == ElementType::Box ||
			type == ElementType::Cylinder ||
			type == ElementType::Sphere ||
			type == ElementType::MeshObject ||
			type == ElementType::Tag ||
			type == ElementType::Point ||
			type == ElementType::PCO ||
			type == ElementType::Cluster ||
			type == ElementType::Scan;
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
	m_objectsClippable = data.getObjectsClippable();
	m_objectsTransform = data.getObjectsTransform();
	m_objectsClippingDistances = data.getObjectsClippingDistances();
	m_objectsRampDistances = data.getObjectsRampDistances();
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

void ViewPointData::setObjectsClippable(const std::unordered_map<SafePtr<AGraphNode>, bool>& map)
{
	m_objectsClippable = map;
}

void ViewPointData::setObjectsTransform(const std::unordered_map<SafePtr<AGraphNode>, TransformationModule>& map)
{
	m_objectsTransform = map;
}

void ViewPointData::setObjectsClippingDistances(const std::unordered_map<SafePtr<AGraphNode>, ClippingDistances>& map)
{
	m_objectsClippingDistances = map;
}

void ViewPointData::setObjectsRampDistances(const std::unordered_map<SafePtr<AGraphNode>, RampDistances>& map)
{
	m_objectsRampDistances = map;
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

const std::unordered_map<SafePtr<AGraphNode>, bool>& ViewPointData::getObjectsClippable() const
{
	return m_objectsClippable;
}

const std::unordered_map<SafePtr<AGraphNode>, TransformationModule>& ViewPointData::getObjectsTransform() const
{
	return m_objectsTransform;
}

const std::unordered_map<SafePtr<AGraphNode>, ViewPointData::ClippingDistances>& ViewPointData::getObjectsClippingDistances() const
{
	return m_objectsClippingDistances;
}

const std::unordered_map<SafePtr<AGraphNode>, ViewPointData::RampDistances>& ViewPointData::getObjectsRampDistances() const
{
	return m_objectsRampDistances;
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
	std::unordered_map<SafePtr<AGraphNode>, bool> objectsClippable;
	std::unordered_map<SafePtr<AGraphNode>, TransformationModule> objectsTransform;
	std::unordered_map<SafePtr<AGraphNode>, ClippingDistances> objectsClippingDistances;
	std::unordered_map<SafePtr<AGraphNode>, RampDistances> objectsRampDistances;
	for (const SafePtr<AGraphNode>& object : graphManager.getProjectNodes())
	{
		ReadPtr<AGraphNode> rObject = object.cget();
		if (!rObject)
			continue;

		const ElementType type = rObject->getType();

		if (supportsViewpointColor(type))
			scanClusterColors[object] = rObject->getColor();

		if (supportsViewpointClippable(type))
		{
			const PointCloudNode* pointCloud = static_cast<const PointCloudNode*>(rObject.operator->());
			objectsClippable[object] = pointCloud->getClippable();
		}

		if (supportsViewpointTransform(type))
			objectsTransform[object] = rObject->getTransformationModule();

		if (supportsViewpointClippingDistances(type))
		{
			const AClippingNode* clippingObject = static_cast<const AClippingNode*>(rObject.operator->());
			ClippingDistances clipDistances;
			clipDistances.minClip = clippingObject->getMinClipDist();
			clipDistances.maxClip = clippingObject->getMaxClipDist();
			clipDistances.lengthThreshold = supportsLengthThreshold(type) ? clippingObject->getLengthThresholdClip() : 0.f;
			objectsClippingDistances[object] = clipDistances;

			RampDistances rampDistances;
			rampDistances.minRamp = clippingObject->getRampMin();
			rampDistances.maxRamp = clippingObject->getRampMax();
			rampDistances.stepsRamp = clippingObject->getRampSteps();
			rampDistances.rampClamped = clippingObject->isRampClamped();
			objectsRampDistances[object] = rampDistances;
		}
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
	wVP->setObjectsClippable(objectsClippable);
	wVP->setObjectsTransform(objectsTransform);
	wVP->setObjectsClippingDistances(objectsClippingDistances);
	wVP->setObjectsRampDistances(objectsRampDistances);
	wVP->setVisibleObjects(visible);
}
