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

void PointCloudNode::setTlsFilePath(const std::filesystem::path& scanPath, bool init_position)
{
    tlGetScanGuid(scanPath, m_scanGuid);
    setName(scanPath.stem().wstring());
    backup_file_path_ = scanPath;

    tls::ScanHeader scanHeader;
    if (init_position && tlGetScanHeader(m_scanGuid, scanHeader))
    {
        setPosition(glm::dvec3(scanHeader.transfo.translation[0], scanHeader.transfo.translation[1], scanHeader.transfo.translation[2]));
        setRotation({ scanHeader.transfo.quaternion[3], scanHeader.transfo.quaternion[0], scanHeader.transfo.quaternion[1], scanHeader.transfo.quaternion[2] });
    }
}

std::filesystem::path PointCloudNode::getTlsFilePath() const
{
    std::filesystem::path file_path;
    tlGetCurrentScanPath(m_scanGuid, file_path);
    return file_path;
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