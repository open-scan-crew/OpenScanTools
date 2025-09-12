#ifndef CLUSTER_NODE_H
#define CLUSTER_NODE_H

#include "models/graph/AGraphNode.h"
#include "models/data/Cluster/ClusterData.h"

class GraphManager;
class IDataDispatcher;

class ClusterNode : public AGraphNode, public ClusterData
{
public:
    ClusterNode(const ClusterNode& data);
    ClusterNode();
    ~ClusterNode();

    virtual ElementType getType() const override;
    virtual bool isAcceptableOwningChild(const SafePtr<AGraphNode>& child) const;

    virtual TreeType getDefaultTreeType() const override;

    virtual std::wstring getComposedName() const override;

    std::unordered_set<Selection> getAcceptableSelections(ManipulationMode mode) const;

public:
    bool m_isMasterCluster;
};

#endif CLUSTER_NODE_H