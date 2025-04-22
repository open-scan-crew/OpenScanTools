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

    std::unordered_set<Selection> getAcceptableSelections(const ManipulationMode& mode) const override;
    std::unordered_set<ManipulationMode> getAcceptableManipulationModes() const override;

    // Use this function to also change the marker color
    virtual void setColor(const Color32& color) override;

    virtual void setDefaultData(const Controller& controller) override;


};

#endif // !TAG_NODE_H_
