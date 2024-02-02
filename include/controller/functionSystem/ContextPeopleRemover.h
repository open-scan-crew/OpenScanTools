#ifndef CONTEXT_PEOPLE_REMOVER_H_
#define CONTEXT_PEOPLE_REMOVER_H_

#include "controller/functionSystem/ARayTracingContext.h"
#include "models/OpenScanToolsModelEssentials.h"
#include "models/3d/graph/ClusterNode.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "pointCloudEngine/OctreeRayTracing.h"

class ContextPeopleRemover : public ARayTracingContext
{
public:
	ContextPeopleRemover(const ContextId& id);
	~ContextPeopleRemover();
	ContextState start(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller);
	ContextState launch(Controller& controller);

	bool canAutoRelaunch() const;
	ContextType getType() const override;

private:
	std::vector<SafePtr<ClusterNode>> m_clusters;
	std::vector<ClusterInfo> m_clustersInfo;
	int m_currentIndexCluster, m_totalCluster, m_totalVolume, m_trueDynamic, m_falsePositives;
	std::vector<std::vector<std::vector<int>>> m_boxCoverForAllClusters;
	double m_voxelSize, m_xMin, m_yMin, m_zMin, m_maxSize;
};

#endif // !CONTEXT_PEOPLE_REMOVER_H_

