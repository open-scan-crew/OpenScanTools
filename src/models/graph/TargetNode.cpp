#include "models/graph/TargetNode.h"

TargetNode::TargetNode(const TargetNode& ref)
    : AGraphNode(ref)
{}

TargetNode::TargetNode(const glm::dvec3& position)
    : AGraphNode()
{
    marker_icon_ = scs::MarkerIcon::Target;
    m_center = position;
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