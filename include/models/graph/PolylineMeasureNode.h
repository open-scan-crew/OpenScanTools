#ifndef POLYLINE_MEASURE_NODE_H
#define POLYLINE_MEASURE_NODE_H

#include "AMeasureNode.h"
#include "models/data/PolylineMeasure/PolylineMeasureData.h"

class MeasureStorage;

class PolylineMeasureNode : public AMeasureNode, public PolylineMeasureData
{
public:
    PolylineMeasureNode();
    PolylineMeasureNode(const PolylineMeasureNode& node);

    virtual std::vector<Measure> getMeasures() const override;

    ElementType getType() const override;
    TreeType getDefaultTreeType() const override;

    virtual void getSegmentDrawData(const glm::dmat4& gTransfo, std::vector<SegmentDrawData>& segments) const override;
};

#endif
