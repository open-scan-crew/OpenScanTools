#ifndef TARGET_NODE_H
#define TARGET_NODE_H

#include "models/graph/AGraphNode.h"

class TargetNode : public AGraphNode
{
public:
    TargetNode(const TargetNode&);
    TargetNode();
    ~TargetNode();

    ElementType getType() const override;
    TreeType getDefaultTreeType() const override;
};

#endif
