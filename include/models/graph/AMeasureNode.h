#ifndef A_MEASURE_NODE_H
#define A_MEASURE_NODE_H

#include "AClippingNode.h"
#include "models/3d/Measures.h"
#include "models/3d/SegmentDrawData.h"

class AMeasureNode : public AClippingNode
{
public:
    AMeasureNode();
    AMeasureNode(const AMeasureNode& node);
    ~AMeasureNode();

    std::unordered_set<Selection> getAcceptableSelections(const ManipulationMode& mode) const override;
    std::unordered_set<ManipulationMode> getAcceptableManipulationModes() const override;

    virtual std::vector<Measure> getMeasures() const;

    virtual void getSegmentDrawData(const glm::dmat4& gTransfo, std::vector<SegmentDrawData>& segments) const = 0;

    virtual void pushClippingGeometries(ClippingAssembly& clipAssembly, const TransformationModule& transfo) const override;
    //virtual void pushRampGeometries(std::vector<std::shared_ptr<IClippingGeometry>>& retGeom, const TransformationModule& transfo) const override;
};

#endif