#include "models/3d/Graph/BeamBendingMeasureNode.h"
#include "services/MarkerDefinitions.hpp"

BeamBendingMeasureNode::BeamBendingMeasureNode(const BeamBendingMeasureNode& node)
	: AObjectNode(node)
	, BeamBendingMeasureData(node)
{
	assert(m_geometricParent);
	m_center = BeamBendingMeasureData::m_maxBendPos;
	updateMarker();
}

BeamBendingMeasureNode::BeamBendingMeasureNode()
{
	setName(TEXT_DEFAULT_NAME_BEAM_BENDING.toStdWString());
	updateMarker();
}

BeamBendingMeasureNode::~BeamBendingMeasureNode()
{
}

ElementType BeamBendingMeasureNode::getType() const
{
	return ElementType::BeamBendingMeasure;
}

TreeType BeamBendingMeasureNode::getDefaultTreeType() const
{
	return TreeType::Measures;
}

std::unordered_set<Selection> BeamBendingMeasureNode::getAcceptableSelections(const ManipulationMode& mode) const
{
    return {};
}

std::unordered_set<ManipulationMode> BeamBendingMeasureNode::getAcceptableManipulationModes() const
{
	return {};
}

void BeamBendingMeasureNode::setColor(const Color32& color)
{
    if (color == m_color)
        return;
	Data::setColor(color);
	updateMarker();
}

scs::MarkerIcon BeamBendingMeasureNode::getIconType() const
{
	return scs::MarkerIcon::BeamBending;
}

MarkerDrawData BeamBendingMeasureNode::getMarkerDrawData(const glm::dmat4& gTransfo) const
{
	// Compose the style
	uint32_t status = 0;
	if (m_selected)
		status |= 0x01;
	if (m_isHovered)
		status |= 0x02;
	if (scs::markerStyleDefs.at(scs::MarkerIcon::BeamBending).showTrueColor)
		status |= 0x04;

	return {
		{ (float)gTransfo[3][0], (float)gTransfo[3][1], (float)gTransfo[3][2] },
		{ m_color.r, m_color.g, m_color.b, m_color.a },
		m_graphicId,
		(uint32_t)scs::MarkerIcon::BeamBending,
		m_primitiveDef.firstVertex,
		m_primitiveDef.vertexCount,
		status
	};
}

void BeamBendingMeasureNode::setMaxBendingPos(const Pos3D& pos)
{
	BeamBendingMeasureData::setMaxBendingPos(pos);
	setPosition(pos);
}

void BeamBendingMeasureNode::updateMarker()
{
	scs::MarkerStyleDefinition marker_style = scs::markerStyleDefs.at(scs::MarkerIcon::BeamBending);
	m_showMarkerTrueColor = marker_style.showTrueColor;
	m_primitiveDef = scs::g_shapePrimitives.at(marker_style.shape);
}
