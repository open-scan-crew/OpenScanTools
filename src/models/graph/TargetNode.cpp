#include "models/graph/TargetNode.h"

TargetNode::TargetNode(const TargetNode& ref)
    : AGraphNode(ref)
{}

TargetNode::TargetNode(const glm::dvec3& _position, Color32 _color)
    : AGraphNode()
{
    marker_icon_ = scs::MarkerIcon::Target;
    m_center = _position;
    m_color = _color;
    //m_graphicId = INVALID_PICKING_ID;
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