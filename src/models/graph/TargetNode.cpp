#include "models/graph/TargetNode.h"

TargetNode::TargetNode(const TargetNode& ref)
    : AGraphNode(ref)
{}

TargetNode::TargetNode()
    : AGraphNode()
{
    marker_icon_ = scs::MarkerIcon::Target;
}

TargetNode::~TargetNode()
{

}

ElementType TargetNode::getType() const
{
    return ElementType::Target;
}

TreeType TargetNode::getDefaultTreeType() const
{
    return TreeType::RawData;
}