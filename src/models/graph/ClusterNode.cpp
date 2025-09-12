#include "models/graph/ClusterNode.h"
#include "models/3d/ManipulationTypes.h"
#include "gui/texts/DefaultNameTexts.hpp"

ClusterNode::ClusterNode(const ClusterNode& node)
    : AGraphNode(node)
    , ClusterData(node)
    , m_isMasterCluster(false)
{
}

ClusterNode::ClusterNode()
    : m_isMasterCluster(false)
{
    setName(TEXT_DEFAULT_NAME_CLUSTER.toStdWString());
}

ClusterNode::~ClusterNode()
{}

ElementType ClusterNode::getType() const
{
    return m_isMasterCluster ? ElementType::MasterCluster : ElementType::Cluster;
}

bool ClusterNode::isAcceptableOwningChild(const SafePtr<AGraphNode>& child) const
{
    ElementType type;
    TreeType childDefaultTreeType;
    {
        ReadPtr<AGraphNode> rChild = child.cget();
        if (!rChild)
            return false;
        if (rChild->getId() == getId())
            return false;
        type = rChild->getType();
        childDefaultTreeType = rChild->getDefaultTreeType();
    }

    switch (m_clusterTreeType)
    {
        case TreeType::Hierarchy:
        {
            if (type == ElementType::Cluster && childDefaultTreeType != TreeType::Hierarchy)
                return false;
            return true;
        }
        break;
        case TreeType::Piping:
        {
            if (type == ElementType::Cylinder
                || type == ElementType::Torus)
                return true;
        }
        break;
        default:
        {
            if (childDefaultTreeType == m_clusterTreeType)
                return true;
        }
        break;
    }

    if (m_clusterTreeType == TreeType::Hierarchy)
        return true;

    return false;
}

TreeType ClusterNode::getDefaultTreeType() const
{
    return getClusterTreeType();
}

std::wstring ClusterNode::getComposedName() const
{
    return m_name;
}

std::unordered_set<Selection> ClusterNode::getAcceptableSelections(ManipulationMode mode) const
{
    switch (mode)
    {
    case ManipulationMode::Translation:
    case ManipulationMode::Rotation:
        return { Selection::X, Selection::Y, Selection::Z };
    }
    return {};
}
