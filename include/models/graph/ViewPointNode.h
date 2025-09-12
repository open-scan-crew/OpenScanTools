#ifndef VIEWPOINTNODE_H_
#define VIEWPOINTNODE_H_

#include "models/graph/AGraphNode.h"
#include "models/data/ViewPoint/ViewPointData.h"

class ViewPointNode : public AGraphNode, public ViewPointData
{
public:
    ViewPointNode();
    ViewPointNode(const ViewPointNode& node);
    ~ViewPointNode();

    ElementType getType() const override;
    TreeType getDefaultTreeType() const override;

    std::unordered_set<Selection> getAcceptableSelections(ManipulationMode mode) const override;
};

#endif // !VIEWPOINTNODE_H_