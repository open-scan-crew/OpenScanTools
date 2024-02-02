#include "models/3d/Graph/ColumnTiltMeasureNode.h"
#include "services/MarkerDefinitions.hpp"

ColumnTiltMeasureNode::ColumnTiltMeasureNode(const ColumnTiltMeasureNode& node)
	: AObjectNode(node)
	, ColumnTiltMeasureData(node)
{
	assert(m_geometricParent);
	m_center = ColumnTiltMeasureData::m_tp;
	updateMarker();
}

ColumnTiltMeasureNode::ColumnTiltMeasureNode()
{
	setName(TEXT_DEFAULT_NAME_COLUMN_TILT.toStdWString());
	updateMarker();
}

ColumnTiltMeasureNode::~ColumnTiltMeasureNode()
{
}

ElementType ColumnTiltMeasureNode::getType() const
{
	return ElementType::ColumnTiltMeasure;
}

TreeType ColumnTiltMeasureNode::getDefaultTreeType() const
{
	return TreeType::Measures;
}

std::unordered_set<Selection> ColumnTiltMeasureNode::getAcceptableSelections(const ManipulationMode& mode) const
{
    return {};
}

std::unordered_set<ManipulationMode> ColumnTiltMeasureNode::getAcceptableManipulationModes() const
{
	return {};
}

void ColumnTiltMeasureNode::setColor(const Color32& color)
{
    if (color == m_color)
        return;
	AObjectNode::setColor(color);
	updateMarker();
}

scs::MarkerIcon ColumnTiltMeasureNode::getIconType() const
{
	return scs::MarkerIcon::ColumnTilt;
}

MarkerDrawData ColumnTiltMeasureNode::getMarkerDrawData(const glm::dmat4& gTransfo) const
{
	// Compose the style
	uint32_t status = 0;
	if (m_selected)
		status |= 0x01;
	if (m_isHovered)
		status |= 0x02;
	if (scs::markerStyleDefs.at(scs::MarkerIcon::ColumnTilt).showTrueColor)
		status |= 0x04;

	return {
		{ (float)gTransfo[3][0], (float)gTransfo[3][1], (float)gTransfo[3][2] },
		{ m_color.r, m_color.g, m_color.b, m_color.a },
		m_graphicId,
		(uint32_t)scs::MarkerIcon::ColumnTilt,
		m_primitiveDef.firstVertex,
		m_primitiveDef.vertexCount,
		status
	};
}

void ColumnTiltMeasureNode::setTopPoint(const Pos3D& tp)
{
	ColumnTiltMeasureData::setTopPoint(tp);
	setPosition(tp);
}

void ColumnTiltMeasureNode::updateMarker()
{
	scs::MarkerStyleDefinition marker_style = scs::markerStyleDefs.at(scs::MarkerIcon::ColumnTilt);
	m_showMarkerTrueColor = marker_style.showTrueColor;
	m_primitiveDef = scs::g_shapePrimitives.at(marker_style.shape);
}
