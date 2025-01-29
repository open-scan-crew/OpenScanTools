#ifndef MESH_OBJECT_NODE_H
#define MESH_OBJECT_NODE_H

#include "models/graph/AObjectNode.h"
#include "models/data/MeshObject/MeshObjectData.h"

class MeshObjectNode : public AObjectNode, public MeshObjectData
{
public:
	MeshObjectNode(const MeshObjectNode& data);
	MeshObjectNode();
	~MeshObjectNode();

	ElementType getType() const override;
	TreeType getDefaultTreeType() const override;

	virtual void setDead(bool isDead) override;

	std::unordered_set<Selection> getAcceptableSelections(const ManipulationMode& mode) const override;
	std::unordered_set<ManipulationMode> getAcceptableManipulationModes() const override;

	virtual MeshDrawData getMeshDrawData(const glm::dmat4& gTransfo) const override;

private:
	void addMeshInstance();
};

#endif
