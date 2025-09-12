#ifndef TORUS_NODE_H
#define TORUS_NODE_H

#include "models/graph/AClippingNode.h"
#include "models/data/Torus/TorusData.h"
#include "models/3d/MeshId.h"

class TorusNode : public AClippingNode, public TorusData
{
public:
    TorusNode(const TorusNode& uidata);
    TorusNode(const double& mainAngle, const double& mainRadius, const double& tubeRadius, const double& insulatedRadius);
    TorusNode();
    ~TorusNode();

    ElementType getType() const override;
    TreeType getDefaultTreeType() const override;

    virtual void pushClippingGeometries(ClippingAssembly& clipAssembly, const TransformationModule& transfo) const override;
    virtual void pushRampGeometries(std::vector<std::shared_ptr<IClippingGeometry>>& retGeom, const TransformationModule& transfo) const override;

    virtual void setDead(bool isDead) override;
    void updateTorusMesh();

    std::unordered_set<Selection> getAcceptableSelections(ManipulationMode mode) const override;

    MeshId getMeshId() const;

    virtual MeshDrawData getMeshDrawData(const glm::dmat4& gTransfo) const override;

private:
    MeshId m_meshId;
};

#endif