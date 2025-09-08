#include "models/graph/PointCloudNode.h"
#include "pointCloudEngine/PCE_core.h"
#include "vulkan/VulkanManager.h"

PointCloudNode::PointCloudNode(const PointCloudNode& data)
    : AGraphNode(data)
    , ScanData(data)
{
    // FIXME(robin) - We should get the image count from the framebuffer, it is not always 2
    VulkanManager::getInstance().allocUniform(sizeof(glm::mat4), 2, m_modelUni);
}

PointCloudNode::PointCloudNode(bool is_object)
{
    is_object_ = is_object;
    Data::marker_icon_ = is_object ? scs::MarkerIcon::PCO : scs::MarkerIcon::Scan_Base;
    VulkanManager::getInstance().allocUniform(sizeof(glm::mat4), 2, m_modelUni);
}

PointCloudNode::~PointCloudNode()
{
    VulkanManager::getInstance().freeUniform(m_modelUni);
    freeScanFile();
}

ElementType PointCloudNode::getType() const
{
    return (is_object_ ? ElementType::PCO : ElementType::Scan);
}

TreeType PointCloudNode::getDefaultTreeType() const
{
    return (is_object_ ? TreeType::Pco : TreeType::Scan);
}

std::wstring PointCloudNode::getComposedName() const
{
    if (is_object_)
    {
        return AGraphNode::getComposedName();
    }
    else
    {
        return m_name;
    }
}

void PointCloudNode::setScanGuid(tls::ScanGuid scanGuid)
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

std::unordered_set<Selection> PointCloudNode::getAcceptableSelections(const ManipulationMode& mode) const
{
    switch (mode)
    {
    case ManipulationMode::Translation:
    case ManipulationMode::Rotation:
        return { Selection::X, Selection::Y, Selection::Z };
    case ManipulationMode::Extrusion:
        if (is_object_)
            return { Selection::X, Selection::Y, Selection::Z,
                     Selection::_X, Selection::_Y, Selection::_Z };
        else
            return {};
    default:
        return {};
    }
}

std::unordered_set<ManipulationMode> PointCloudNode::getAcceptableManipulationModes() const
{
    return { ManipulationMode::Translation, ManipulationMode::Rotation };
}

void PointCloudNode::uploadUniform(glm::mat4 modelTransfo, uint32_t swapIndex)
{
    VulkanManager::getInstance().loadUniformData(sizeof(glm::mat4), &modelTransfo, 0, swapIndex, m_modelUni);
}

VkUniformOffset PointCloudNode::getUniform(uint32_t swapIndex) const
{
    return m_modelUni[swapIndex];
}