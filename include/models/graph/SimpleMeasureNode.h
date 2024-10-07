#ifndef SIMPLE_MEASURE_NODE_H
#define SIMPLE_MEASURE_NODE_H

#include "AMeasureNode.h"
#include "models/data/SimpleMeasure/SimpleMeasureData.h"

class MeasureStorage;

class SimpleMeasureNode : public AMeasureNode, public SimpleMeasureData
{
public:
    SimpleMeasureNode();
    SimpleMeasureNode(const SimpleMeasureNode& node);
    //~SimpleMeasureNode();

    virtual std::vector<Measure> getMeasures() const override;

    ElementType getType() const override;
    TreeType getDefaultTreeType() const override;

    TransformationModule updateClipping();

    virtual void getSegmentDrawData(const glm::dmat4& gTransfo, std::vector<SegmentDrawData>& segments) const override;
};

#endif
