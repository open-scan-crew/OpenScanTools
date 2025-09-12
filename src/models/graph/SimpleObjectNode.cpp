#include "models/graph/SimpleObjectNode.h"
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
    if(!is_dead_)
        MeshManager::getInstance().removeGenericMeshInstance(m_meshId);
}

GenericMeshId SimpleObjectNode::getGenericMeshId() const
{
    return m_meshId;
}

void SimpleObjectNode::setDead(bool isDead)
{
    if (!is_dead_ && isDead)
        MeshManager::getInstance().removeGenericMeshInstance(m_meshId);
    if (is_dead_ && !isDead)
        addGenericMeshInstance();

    AGraphNode::setDead(isDead);
}

std::unordered_set<Selection> SimpleObjectNode::getAcceptableSelections(ManipulationMode mode) const
{
    if (MeshManager::SimpleObjectManipulatorSelections.find(m_meshId.type) != MeshManager::SimpleObjectManipulatorSelections.end()
        && MeshManager::SimpleObjectManipulatorSelections.at(m_meshId.type).find(mode) != MeshManager::SimpleObjectManipulatorSelections.at(m_meshId.type).end())
        return MeshManager::SimpleObjectManipulatorSelections.at(m_meshId.type).at(mode);
    return {};
}

MeshDrawData SimpleObjectNode::getMeshDrawData(const glm::dmat4& gTransfo) const
{
    MeshDrawData meshDrawData = AGraphNode::getMeshDrawData(gTransfo);
    meshDrawData.meshBuffer = MeshManager::getInstance().getGenericMesh(m_meshId);
    return meshDrawData;
}
