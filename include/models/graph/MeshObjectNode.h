#ifndef MESH_OBJECT_NODE_H
#define MESH_OBJECT_NODE_H

#include "models/graph/AGraphNode.h"
#include "models/data/MeshObject/MeshObjectData.h"

class MeshObjectNode : public AGraphNode, public MeshObjectData
{
public:
    MeshObjectNode(const MeshObjectNode& data);
    MeshObjectNode();
    ~MeshObjectNode();

    ElementType getType() const override;
    TreeType getDefaultTreeType() const override;

    virtual void setDead(bool isDead) override;

    std::unordered_set<Selection> getAcceptableSelections(ManipulationMode mode) const override;

    virtual MeshDrawData getMeshDrawData(const glm::dmat4& gTransfo) const override;

private:
    void addMeshInstance();
};

#endif
