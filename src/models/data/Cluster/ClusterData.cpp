#include "models/data/Cluster/ClusterData.h"

ClusterData::ClusterData()
{ }

ClusterData::~ClusterData()
{ }

void ClusterData::copyCluster(const ClusterData& data)
{
	m_clusterTreeType = data.getClusterTreeType();
}

void ClusterData::setTreeType(TreeType type)
{
	m_clusterTreeType = type;
}

TreeType ClusterData::getClusterTreeType() const
{
	return m_clusterTreeType;
}