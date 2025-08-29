#ifndef APOINT_CLOUD_NODE_H
#define APOINT_CLOUD_NODE_H

#include "models/graph/AGraphNode.h"
#include "tls_def.h"
#include "models/data/Scan/ScanData.h"
#include "vulkan/VkUniform.h"

class APointCloudNode : public AGraphNode, public ScanData
{
public:
	APointCloudNode(const APointCloudNode& data);
	APointCloudNode();

	~APointCloudNode();

	void setScanGuid(tls::ScanGuid scanGuid);

	void uploadUniform(glm::mat4 modelTransfo, uint32_t swapIndex);
	VkUniformOffset getUniform(uint32_t swapIndex) const;

protected:
	VkMultiUniform			m_modelUni;
};

#endif //! APOINT_CLOUD_NODE_H_