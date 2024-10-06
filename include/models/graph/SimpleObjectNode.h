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

	virtual std::unordered_set<Selection> getAcceptableSelections(const ManipulationMode& mode) const;
	virtual std::unordered_set<ManipulationMode> getAcceptableManipulationModes() const override;

	virtual MeshDrawData getMeshDrawData(const glm::dmat4& gTransfo) const override;

protected:
	GenericMeshId m_meshId;
};

#endif // SIMPLEOBJECTNODE_H_