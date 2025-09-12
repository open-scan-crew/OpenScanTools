#ifndef SIMPLEOBJECTNODE_H_
#define SIMPLEOBJECTNODE_H_

#include "models/graph/AClippingNode.h"
#include "models/3d/MeshBuffer.h"

class SimpleObjectNode : public AClippingNode
{
public:
    SimpleObjectNode(const SimpleObjectNode& node);
    SimpleObjectNode(); 
    ~SimpleObjectNode();

    GenericMeshId getGenericMeshId() const;

    virtual void setDead(bool isDead) override;
    virtual void addGenericMeshInstance() = 0;

    virtual std::unordered_set<Selection> getAcceptableSelections(ManipulationMode mode) const;

    virtual MeshDrawData getMeshDrawData(const glm::dmat4& gTransfo) const override;

protected:
    GenericMeshId m_meshId;
};

#endif // SIMPLEOBJECTNODE_H_