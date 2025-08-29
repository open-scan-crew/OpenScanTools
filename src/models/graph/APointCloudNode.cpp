#include "models/graph/APointCloudNode.h"
#include "pointCloudEngine/PCE_core.h"
#include "vulkan/VulkanManager.h"

APointCloudNode::APointCloudNode(const APointCloudNode& data)
	: AGraphNode(data)
	, ScanData(data)
{
	// FIXME(robin) - We should get the image count from the framebuffer, it is not always 2
	VulkanManager::getInstance().allocUniform(sizeof(glm::mat4), 2, m_modelUni);
	m_clippable = static_cast<const ScanData&>(data).getClippable();
}

APointCloudNode::APointCloudNode()
{
	VulkanManager::getInstance().allocUniform(sizeof(glm::mat4), 2, m_modelUni);
}

APointCloudNode::~APointCloudNode()
{
	VulkanManager::getInstance().freeUniform(m_modelUni);
	freeScanFile();
}

void APointCloudNode::setScanGuid(tls::ScanGuid scanGuid)
{
	m_scanGuid = scanGuid;

	tls::ScanHeader scanHeader;
	if (tlGetScanHeader(m_scanGuid, scanHeader) == true)
	{
		//A enregistrer dans le json ? Ou on garde ça dans le fichier de scan
		m_pointFormat = scanHeader.format;
		m_NbPoint = scanHeader.pointCount;
		m_sensorModel = scanHeader.sensorModel;
		m_sensorSerialNumber = scanHeader.sensorSerialNumber;
		m_acquisitionTime = scanHeader.acquisitionDate;
	}
	// else, the tls guid automaticaly is zero-initialized
}

void APointCloudNode::uploadUniform(glm::mat4 modelTransfo, uint32_t swapIndex)
{
	VulkanManager::getInstance().loadUniformData(sizeof(glm::mat4), &modelTransfo, 0, swapIndex, m_modelUni);
}

VkUniformOffset APointCloudNode::getUniform(uint32_t swapIndex) const
{
	return m_modelUni[swapIndex];
}