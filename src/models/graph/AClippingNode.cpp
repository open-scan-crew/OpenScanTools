#include "models/graph/AClippingNode.h"

#include "controller/Controller.h"

AClippingNode::AClippingNode(const AClippingNode& node)
	: AObjectNode(node)
	, ClippingData(node)
{}

AClippingNode::AClippingNode()
{}

void AClippingNode::setDefaultData(const Controller& controller)
{
	ClippingData::setDefaultData(controller.cgetContext(), { getType() });
	AObjectNode::setDefaultData(controller);
}

void AClippingNode::pushClippingGeometries(ClippingAssembly& clipAssembly, const TransformationModule& transfo) const
{
	return;
}

void AClippingNode::pushRampGeometries(std::vector<std::shared_ptr<IClippingGeometry>>& retGeom, const TransformationModule& transfo) const
{
	return;
}

