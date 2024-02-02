#include "models/3d/Graph/SimpleObjectNode.h"
#include "vulkan/MeshManager.h"

SimpleObjectNode::SimpleObjectNode(const SimpleObjectNode& node)
	: AClippingNode(node)
	, m_meshId(node.m_meshId)
{}

SimpleObjectNode::SimpleObjectNode()
	: m_meshId()
{}

SimpleObjectNode::~SimpleObjectNode()
{
	if(!m_isDead)
		MeshManager::getInstance().removeGenericMeshInstance(m_meshId);
}

GenericMeshId SimpleObjectNode::getGenericMeshId() const
{
	return m_meshId;
}

void SimpleObjectNode::setDead(bool isDead)
{
	if (!m_isDead && isDead)
		MeshManager::getInstance().removeGenericMeshInstance(m_meshId);
	if (m_isDead && !isDead)
		addGenericMeshInstance();

	AGraphNode::setDead(isDead);
}

std::unordered_set<Selection> SimpleObjectNode::getAcceptableSelections(const ManipulationMode& mode) const
{
	if (MeshManager::SimpleObjectManipulatorSelections.find(m_meshId.type) != MeshManager::SimpleObjectManipulatorSelections.end()
		&& MeshManager::SimpleObjectManipulatorSelections.at(m_meshId.type).find(mode) != MeshManager::SimpleObjectManipulatorSelections.at(m_meshId.type).end())
		return MeshManager::SimpleObjectManipulatorSelections.at(m_meshId.type).at(mode);
	return {};
}

std::unordered_set<ManipulationMode> SimpleObjectNode::getAcceptableManipulationModes() const
{
	if (MeshManager::SimpleObjectManipulatorSelections.find(m_meshId.type) == MeshManager::SimpleObjectManipulatorSelections.end())
		return {};
	std::unordered_set<ManipulationMode> retvals;

	for (const auto& iterator : MeshManager::SimpleObjectManipulatorSelections.at(m_meshId.type))
		retvals.insert(iterator.first);
	return retvals;
}

MeshDrawData SimpleObjectNode::getMeshDrawData(const glm::dmat4& gTransfo) const
{
	MeshDrawData meshDrawData = AObjectNode::getMeshDrawData(gTransfo);
	meshDrawData.meshBuffer = MeshManager::getInstance().getGenericMesh(m_meshId);
	return meshDrawData;
}
