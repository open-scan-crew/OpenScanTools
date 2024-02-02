#ifndef PIPE_TO_PLANE_MEASURE_NODE_H
#define PIPE_TO_PLANE_MEASURE_NODE_H

#include "AMeasureNode.h"
#include "models/data/PipeToPlaneMeasure/PipeToPlaneMeasureData.h"

class PipeToPlaneMeasureNode : public AMeasureNode, public PipeToPlaneMeasureData
{
public:
    PipeToPlaneMeasureNode(const PipeToPlaneMeasureNode& node);
    PipeToPlaneMeasureNode();

    virtual std::vector<Measure> getMeasures() const override;

    ElementType getType() const override;
    TreeType getDefaultTreeType() const override;

    virtual void getSegmentDrawData(const glm::dmat4& gTransfo, std::vector<SegmentDrawData>& segments) const override;
};

#endif
