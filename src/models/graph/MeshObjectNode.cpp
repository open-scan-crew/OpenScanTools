#include "models/graph/MeshObjectNode.h"
#include "vulkan/MeshManager.h"
#include "models/3d/ManipulationTypes.h"
#include "gui/texts/DefaultNameTexts.hpp"

MeshObjectNode::MeshObjectNode(const MeshObjectNode& node)
	: AGraphNode(node)
	, MeshObjectData(node)
{

}

MeshObjectNode::MeshObjectNode()
{
	setName(TEXT_DEFAULT_NAME_MESH.toStdWString());
	Data::marker_icon_ = scs::MarkerIcon::MeshObject;
}

MeshObjectNode::~MeshObjectNode()
{
	if (!is_dead_)
		MeshManager::getInstance().removeMeshInstance(m_meshId);
}

ElementType MeshObjectNode::getType() const
{
	return ElementType::MeshObject;
}

TreeType MeshObjectNode::getDefaultTreeType() const
{
	return TreeType::MeshObjects;
}

void MeshObjectNode::setDead(bool isDead)
{
	if (!is_dead_ && isDead)
		MeshManager::getInstance().removeMeshInstance(m_meshId);
	if(is_dead_ && !isDead)
		addMeshInstance();
	AGraphNode::setDead(isDead);

}

void MeshObjectNode::addMeshInstance()
{
	MeshManager& meshManager = MeshManager::getInstance();
	if (!meshManager.isMeshLoaded(m_meshId))
		meshManager.reloadMeshFile(*this);
	else
		meshManager.addMeshInstance(m_meshId);
}

std::unordered_set<Selection> MeshObjectNode::getAcceptableSelections(const ManipulationMode& mode) const
{
	switch (mode)
	{
	case ManipulationMode::Translation:
	case ManipulationMode::Rotation:
	case ManipulationMode::Scale:
		return { Selection::X, Selection::Y, Selection::Z };
	case ManipulationMode::Extrusion:
	default:
		return {};
	}
}

std::unordered_set<ManipulationMode> MeshObjectNode::getAcceptableManipulationModes() const
{
	return { ManipulationMode::Translation, ManipulationMode::Rotation, ManipulationMode::Scale };
}

MeshDrawData MeshObjectNode::getMeshDrawData(const glm::dmat4& gTransfo) const
{
	MeshDrawData meshDrawData = AGraphNode::getMeshDrawData(gTransfo);
	meshDrawData.meshBuffer = MeshManager::getInstance().getMesh(m_meshId).m_mesh;
	return meshDrawData;
}
