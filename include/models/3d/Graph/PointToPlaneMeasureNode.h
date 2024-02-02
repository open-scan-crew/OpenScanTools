#ifndef POINTOPLANEMEASURE_NODE_H_
#define POINTOPLANEMEASURE_NODE_H_

#include "AMeasureNode.h"
#include "models/data/PointToPlaneMeasure/PointToPlaneMeasureData.h"

class PointToPlaneMeasureNode : public AMeasureNode, public PointToPlaneMeasureData
{
public:
    PointToPlaneMeasureNode();
    PointToPlaneMeasureNode(const PointToPlaneMeasureNode& node);


    virtual std::vector<Measure> getMeasures() const override;

    ElementType getType() const override;
    TreeType getDefaultTreeType() const override;

    void updatePointToPlaneMeasure(const PointToPlaneMeasureNode& node);

    virtual void getSegmentDrawData(const glm::dmat4& gTransfo, std::vector<SegmentDrawData>& segments) const override;
};

#endif // !POINTOPLANEMEASURE_NODE_H_