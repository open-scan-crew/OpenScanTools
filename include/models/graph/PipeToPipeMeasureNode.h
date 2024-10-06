#ifndef PIPE_TO_PIPE_MEASURE_NODE_H
#define PIPE_TO_PIPE_MEASURE_NODE_H

#include "AMeasureNode.h"
#include "models/data/PipeToPipeMeasure/PipeToPipeMeasureData.h"

class PipeToPipeMeasureNode : public AMeasureNode, public PipeToPipeMeasureData
{
public:
	PipeToPipeMeasureNode();
	PipeToPipeMeasureNode(const PipeToPipeMeasureNode& node);

	ElementType getType() const override;
	TreeType getDefaultTreeType() const override;

	virtual std::vector<Measure> getMeasures() const override;

	virtual void getSegmentDrawData(const glm::dmat4& gTransfo, std::vector<SegmentDrawData>& segments) const override;
};

#endif