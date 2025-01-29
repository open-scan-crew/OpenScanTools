#ifndef SPHERE_NODE_H
#define SPHERE_NODE_H

#include "models/graph/SimpleObjectNode.h"
//#include "models/data/Piping/StandardRadiusData.h"

class SphereNode : public SimpleObjectNode
{
public:
    SphereNode(const SphereNode& node);
    SphereNode(const double& detectedRadius);
    SphereNode();

    virtual void addGenericMeshInstance() override;

    ElementType getType() const override;
    TreeType getDefaultTreeType() const override;

    virtual void pushClippingGeometries(ClippingAssembly& clipAssembly, const TransformationModule& transfo) const override;
    virtual void pushRampGeometries(std::vector<std::shared_ptr<IClippingGeometry>>& retGeom, const TransformationModule& transfo) const override;

    void setRadius(double radius);
    double getRadius() const;

protected:
    double m_radius;
};
#endif