#include "models/graph/ColumnTiltMeasureNode.h"
#include "gui/texts/DefaultNameTexts.hpp"

ColumnTiltMeasureNode::ColumnTiltMeasureNode(const ColumnTiltMeasureNode& node)
	: AObjectNode(node)
	, ColumnTiltMeasureData(node)
{
	assert(m_geometricParent);
	m_center = ColumnTiltMeasureData::m_tp;
}

ColumnTiltMeasureNode::ColumnTiltMeasureNode()
{
	setName(TEXT_DEFAULT_NAME_COLUMN_TILT.toStdWString());
	Data::marker_icon_ = scs::MarkerIcon::ColumnTilt;
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

void ColumnTiltMeasureNode::setTopPoint(const Pos3D& tp)
{
	ColumnTiltMeasureData::setTopPoint(tp);
	setPosition(tp);
}
