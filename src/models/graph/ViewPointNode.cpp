#include "models/graph/ViewPointNode.h"

ViewPointNode::ViewPointNode()
	: AGraphNode()
	, ViewPointData()
{
	Data::marker_icon_ = scs::MarkerIcon::ViewPoint;
}

ViewPointNode::ViewPointNode(const ViewPointNode& node)
	: AGraphNode(node)
	, ViewPointData(node)
{
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
