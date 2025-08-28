#include "models/graph/AObjectNode.h"

AObjectNode::AObjectNode()
	: AGraphNode()
{}

AObjectNode::AObjectNode(const AObjectNode& node)
	: AGraphNode(node)
{}

AObjectNode::~AObjectNode()
{
}

AGraphNode::Type AObjectNode::getGraphType() const
{
	return Type::Object;
}

bool AObjectNode::isHovered() const
{
	return m_isHovered;
}

void AObjectNode::setHovered(bool hovered)
{
	m_isHovered = hovered;
}

bool AObjectNode::isAcceptingManipulatorMode(const ManipulationMode& mode) const
{
	return !getAcceptableSelections(mode).empty();
}

std::unordered_set<Selection> AObjectNode::getAcceptableSelections(const ManipulationMode& mode) const
{
	return std::unordered_set<Selection>();
}

std::unordered_set<ManipulationMode> AObjectNode::getAcceptableManipulationModes() const
{
	return std::unordered_set<ManipulationMode>();
}

MeshDrawData AObjectNode::getMeshDrawData(const glm::dmat4& gTransfo) const
{
	MeshDrawData meshDrawData;
	meshDrawData.color = getColor().toVector();
	meshDrawData.graphicId = getGraphicId();
	meshDrawData.isHovered = isHovered();
	meshDrawData.isSelected = isSelected();
	meshDrawData.transfo = gTransfo;
	return meshDrawData;
}