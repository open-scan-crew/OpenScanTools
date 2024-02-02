#ifndef CYLINDER_NODE_H
#define CYLINDER_NODE_H

#include "models/3d/Graph/SimpleObjectNode.h"
#include "models/data/Piping/StandardRadiusData.h"

class CylinderNode : public SimpleObjectNode, public StandardRadiusData
{
public:
    CylinderNode(const CylinderNode& node);
    CylinderNode(const double& detectedRadius);
    CylinderNode();

    ElementType getType() const override;
    TreeType getDefaultTreeType() const override;

    virtual void pushClippingGeometries(ClippingAssembly& clipAssembly, const TransformationModule& transfo) const override;
    virtual void pushRampGeometries(std::vector<std::shared_ptr<IClippingGeometry>>& retGeom, const TransformationModule& transfo) const override;

    virtual void setDefaultData(const Controller& controller) override;
    virtual void addGenericMeshInstance() override;

    double getLength() const;
    double getRadius() const;
    void setLength(double length);
    void updateScale();

    void addScale(const glm::dvec3& addScale) override;

protected:
    double m_length = 1.0;
};
#endif