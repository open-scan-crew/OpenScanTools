#ifndef TARGET_NODE_H
#define TARGET_NODE_H

#include "models/graph/AObjectNode.h"

class TargetNode : public AObjectNode
{
public:
    TargetNode(const TargetNode&);
    TargetNode(const glm::dvec3& _position, Color32 _color);
    ~TargetNode();

    ElementType getType() const override;
    TreeType getDefaultTreeType() const override;
};

#endif
