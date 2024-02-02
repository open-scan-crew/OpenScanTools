#ifndef CLUSTER_NODE_H
#define CLUSTER_NODE_H

#include "models/3d/Graph/AObjectNode.h"
#include "models/data/Cluster/ClusterData.h"

class GraphManager;
class IDataDispatcher;

class ClusterNode : public AObjectNode, public ClusterData
{
public:
	ClusterNode(const ClusterNode& data);
	ClusterNode();
	~ClusterNode();

	virtual ElementType getType() const override;
	virtual bool isAcceptableOwningChild(const SafePtr<AGraphNode>& child) const;

	virtual TreeType getDefaultTreeType() const override;

	virtual std::wstring getComposedName() const override;

	std::unordered_set<Selection> getAcceptableSelections(const ManipulationMode& mode) const;
	std::unordered_set<ManipulationMode> getAcceptableManipulationModes() const;

	static bool setGeometricChildren(const SafePtr<ClusterNode>& parentCluster, const std::unordered_set<SafePtr<AObjectNode>>& children);

public:
	bool m_isMasterCluster;
};

#endif CLUSTER_NODE_H