#ifndef CLUSTERDATA_H_
#define CLUSTERDATA_H_

#include "models/Types.hpp"

class ClusterData
{
public:
	ClusterData();
	~ClusterData();

	void copyCluster(const ClusterData& data);

	void setTreeType(TreeType type);

	TreeType getClusterTreeType() const;


protected:
	TreeType m_clusterTreeType = TreeType::RawData;
};

#endif // !SETTERCLUSTERDATA_H_