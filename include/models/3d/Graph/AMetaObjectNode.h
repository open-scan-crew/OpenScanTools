#ifndef AMETA_OBJECT_NODE_H_
#define AMETA_OBJECT_NODE_H_

#include "AGraphNode.h"

class AMetaObjectNode : public AGraphNode
{
public:
	AMetaObjectNode(const glm::dmat4& transformation = glm::dmat4(1.0), AGraphNode* parent = nullptr, const std::deque<AGraphNode*>& childs = {});
	AMetaObjectNode(const glm::dvec4& position, const double& phi, const double& theta, const glm::dvec3& scale = glm::dvec3(1.0, 1.0, 1.0), AGraphNode* parent = nullptr, std::deque<AGraphNode*> childs = {});

	const bool isVisible() const;
	void setVisible(const bool& visible);

	virtual GraphNodeType getType() const = 0;

protected:
	bool m_isVisible;

};
#endif // !AMETA_OBJECT_NODE_H_