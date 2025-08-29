#ifndef A_CLIPPING_NODE_H
#define A_CLIPPING_NODE_H

#include "models/graph/AGraphNode.h"
#include "models/data/Clipping/ClippingData.h"
#include "models/data/Clipping/ClippingGeometry.h"

class AClippingNode : public AGraphNode, public ClippingData
{
public:
	AClippingNode(const AClippingNode& node);
	AClippingNode();

	virtual void setDefaultData(const Controller& controller) override;

	virtual void pushClippingGeometries(ClippingAssembly& clipAssembly, const TransformationModule& transfo) const;
	virtual void pushRampGeometries(std::vector<std::shared_ptr<IClippingGeometry>>& retGeom, const TransformationModule& transfo) const;
};
#endif