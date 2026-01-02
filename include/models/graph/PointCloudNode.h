#ifndef POINT_CLOUD_NODE_H
#define POINT_CLOUD_NODE_H

#include "models/graph/AGraphNode.h"
#include "models/data/Scan/ScanData.h"
#include "vulkan/VkUniform.h"

class PointCloudNode : public AGraphNode, public ScanData
{
public:
    PointCloudNode(const PointCloudNode& data);
    PointCloudNode(bool is_object);

    ~PointCloudNode();

    ElementType getType() const override;
    TreeType getDefaultTreeType() const override;

    std::wstring getComposedName() const override;

    void setTlsFilePath(const std::filesystem::path& file_path, bool init_position, const tls::ScanGuid& scan_guid = tls::ScanGuid());
    std::filesystem::path getTlsFilePath() const;

    void setManipulable(bool is_manipulable);

    bool isManipulable(ManipulationMode mode) const override;
    std::unordered_set<Selection> getAcceptableSelections(ManipulationMode mode) const override;

    void uploadUniform(glm::mat4 modelTransfo, uint32_t swapIndex);
    VkUniformOffset getUniform(uint32_t swapIndex) const;

protected:
    bool is_manipulable_;
    VkMultiUniform m_modelUni;
};

#endif //! POINT_CLOUD_NODE_H