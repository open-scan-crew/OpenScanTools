#include "models/3d/Graph/AMetaObjectNode.h"

AMetaObjectNode::AMetaObjectNode(const glm::dmat4& transformation, AGraphNode* parent, const std::deque<AGraphNode*>& childs)
	: AGraphNode(transformation, parent, childs)
	, m_isVisible(true)
{}

AMetaObjectNode::AMetaObjectNode(const glm::dvec4& position, const double& phi, const double& theta, const glm::dvec3& scale, AGraphNode* parent, std::deque<AGraphNode*> childs)
	: AGraphNode(position, phi, theta, scale, parent, childs)
	, m_isVisible(true)
{}

const bool AMetaObjectNode::isVisible() const
{
	return m_isVisible;
}

void AMetaObjectNode::setVisible(const bool& visible)
{
	m_isVisible = visible;
}