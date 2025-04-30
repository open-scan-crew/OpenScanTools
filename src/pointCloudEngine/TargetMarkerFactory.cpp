#include "pointCloudEngine/TargetMarkerFactory.h"
#include "services/MarkerSystem.h"
#include "models/data/Marker.h"
#include "models/OpenScanToolsModelEssentials.h"

TargetMarkerFactory::TargetMarkerFactory()
{}

TargetMarkerFactory::~TargetMarkerFactory()
{
	freeClickMarkers();
}

MarkerDrawData TargetMarkerFactory::generateTargetMarkerData(const glm::dvec3& position, const Color32& color) const
{
	MarkerSystem::RenderStyle marker_style = MarkerSystem::getRenderStyle(scs::MarkerIcon::Target);

	return {
		{ (float)position.x, (float)position.y, (float)position.z },
		color.RGBA(),
		INVALID_PICKING_ID,
		(uint32_t)scs::MarkerIcon::Target,
		marker_style.firstVertex,
		marker_style.vertexCount,
		0 // not hover, not selected, dont show true color
	};
}

void TargetMarkerFactory::createClickTarget(const glm::dvec3& position, const Color32& color)
{
	MarkerDrawData data = generateTargetMarkerData(position, color);
	m_clicks.push_back(data);
}

void TargetMarkerFactory::freeClickMarkers()
{
	m_clicks.clear();
}

std::vector<MarkerDrawData> TargetMarkerFactory::generateMarkersList() const
{
	return m_clicks;
}
