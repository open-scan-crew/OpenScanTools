#ifndef VIEWPOINTNODE_H_
#define VIEWPOINTNODE_H_

#include "models/graph/AObjectNode.h"
#include "models/data/ViewPoint/ViewPointData.h"

class ViewPointNode : public AObjectNode, public ViewPointData
{
public:
    ViewPointNode();
    ViewPointNode(const ViewPointNode& node);
    ~ViewPointNode();

    ElementType getType() const override;
    TreeType getDefaultTreeType() const override;

    std::unordered_set<Selection> getAcceptableSelections(const ManipulationMode& mode) const override;
    std::unordered_set<ManipulationMode> getAcceptableManipulationModes() const override;
};

#endif // !VIEWPOINTNODE_H_