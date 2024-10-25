#ifndef POINT_TO_PIPE_MEASURE_NODE_H
#define POINT_TO_PIPE_MEASURE_NODE_H

#include "AMeasureNode.h"
#include "models/data/PointToPipeMeasure/PointToPipeMeasureData.h"

class PointToPipeMeasureNode : public AMeasureNode, public PointToPipeMeasureData
{
public:
    PointToPipeMeasureNode();
    PointToPipeMeasureNode(const PointToPipeMeasureNode& node);

    virtual std::vector<Measure> getMeasures() const override;

    ElementType getType() const override;
    TreeType getDefaultTreeType() const override;

    virtual void getSegmentDrawData(const glm::dmat4& gTransfo, std::vector<SegmentDrawData>& segments) const override;
};

#endif
