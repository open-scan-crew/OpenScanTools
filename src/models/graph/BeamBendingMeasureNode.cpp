#include "models/graph/BeamBendingMeasureNode.h"
#include "gui/texts/DefaultNameTexts.hpp"

BeamBendingMeasureNode::BeamBendingMeasureNode(const BeamBendingMeasureNode& node)
	: AObjectNode(node)
	, BeamBendingMeasureData(node)
{
	assert(m_geometricParent);
	m_center = BeamBendingMeasureData::m_maxBendPos;
}

BeamBendingMeasureNode::BeamBendingMeasureNode()
{
	setName(TEXT_DEFAULT_NAME_BEAM_BENDING.toStdWString());
	Data::marker_icon_ = scs::MarkerIcon::BeamBending;
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

void BeamBendingMeasureNode::setMaxBendingPos(const Pos3D& pos)
{
	BeamBendingMeasureData::setMaxBendingPos(pos);
	setPosition(pos);
}

