#ifndef TAG_NODE_H
#define TAG_NODE_H

#include "models/graph/AClippingNode.h"
#include "models/data/Tag/TagData.h"

class TagNode : public AClippingNode, public TagData
{
public:
    TagNode(const TagNode&);
    TagNode();
    ~TagNode();

    ElementType getType() const override;
    TreeType getDefaultTreeType() const override;

    virtual void pushClippingGeometries(ClippingAssembly& clipAssembly, const TransformationModule& transfo) const override;
    virtual void pushRampGeometries(std::vector<std::shared_ptr<IClippingGeometry>>& retGeom, const TransformationModule& transfo) const override;

    std::unordered_set<Selection> getAcceptableSelections(ManipulationMode mode) const override;

    virtual void setDefaultData(const Controller& controller) override;
};

#endif // !TAG_NODE_H_
