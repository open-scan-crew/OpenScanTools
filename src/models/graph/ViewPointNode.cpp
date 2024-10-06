#include "models/graph/ViewPointNode.h"
#include "services/MarkerDefinitions.hpp"

ViewPointNode::ViewPointNode()
	: AObjectNode()
	, ViewPointData()
{
	updateMarker();
}

ViewPointNode::ViewPointNode(const ViewPointNode& node)
	: AObjectNode(node)
	, ViewPointData(node)
{
	updateMarker();
}

ViewPointNode::~ViewPointNode()
{
}

ElementType ViewPointNode::getType() const
{
	return ElementType::ViewPoint;
}

TreeType ViewPointNode::getDefaultTreeType() const
{
	return TreeType::ViewPoint;
}

std::unordered_set<Selection> ViewPointNode::getAcceptableSelections(const ManipulationMode& mode) const
{
	return {};
}

std::unordered_set<ManipulationMode> ViewPointNode::getAcceptableManipulationModes() const
{
	return {};
}

scs::MarkerIcon ViewPointNode::getIconType() const
{
	return scs::MarkerIcon::ViewPoint;
}

MarkerDrawData ViewPointNode::getMarkerDrawData(const glm::dmat4& gTransfo) const
{
	// Compose the style
	uint32_t status = 0;
	if (m_selected)
		status |= 0x01;
	if (m_isHovered)
		status |= 0x02;
	if (scs::markerStyleDefs.at(scs::MarkerIcon::ViewPoint).showTrueColor)
		status |= 0x04;

	return {
		{ (float)gTransfo[3][0], (float)gTransfo[3][1], (float)gTransfo[3][2] },
		{ m_color.r, m_color.g, m_color.b, m_color.a },
		m_graphicId,
		(uint32_t)scs::MarkerIcon::ViewPoint,
		m_primitiveDef.firstVertex,
		m_primitiveDef.vertexCount,
		status
	};
}

void ViewPointNode::updateMarker()
{
	scs::MarkerStyleDefinition marker_style = scs::markerStyleDefs.at(scs::MarkerIcon::ViewPoint);
	m_showMarkerTrueColor = marker_style.showTrueColor;
	m_primitiveDef = scs::g_shapePrimitives.at(marker_style.shape);
}