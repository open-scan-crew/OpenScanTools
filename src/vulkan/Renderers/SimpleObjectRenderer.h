#ifndef SIMPLE_OBJECT_RENDERER_H
#define SIMPLE_OBJECT_RENDERER_H

#include "vulkan/Shader.h"
#include "vulkan/vulkan.h"
#include "pointCloudEngine/SmartBuffer.h"
#include "models/3d/MeshBuffer.h"
#include "vulkan/VkUniform.h"

#include <glm/glm.hpp>
#include <unordered_set>
#include <memory>


struct PipelineDef
{
    VkPrimitiveTopology topology;
    uint32_t subpass;
    bool hasNormalsInput;
    VkPipeline pipeline;
};

namespace std {
    template <>
    class hash<PipelineDef> {
    public:
        size_t operator()(const PipelineDef& def) const noexcept
        {
            size_t result = (size_t)def.topology;
            result += (size_t)def.subpass << 16;
            result += (size_t)def.hasNormalsInput << 32;
            return result;
        }
    };

    template <>
    class equal_to<PipelineDef> {
    public:
        constexpr bool operator()(const PipelineDef& lhs, const PipelineDef& rhs) const
        {
            return (lhs.topology == rhs.topology) &&
                (lhs.subpass == rhs.subpass) &&
                (lhs.hasNormalsInput == rhs.hasNormalsInput);
        }
    };
}

struct MeshPushConstant
{
    glm::mat4 transfo;
    uint32_t id;
    float alpha;
    bool hovered;
    bool selected;
    glm::vec3 color;
};

class SimpleObjectRenderer
{
public:
    SimpleObjectRenderer();
    ~SimpleObjectRenderer();

    void drawMesh(VkCommandBuffer _cmdBuffer, VkUniformOffset mvpUni, const std::shared_ptr<MeshBuffer>& meshBuffer, uint32_t subpass, TlTopologyFlags allowedTopologies = TL_TOPOLOGY_ALL);

    void blendTransparentImage(VkCommandBuffer _cmdBuffer, VkDescriptorSet _inputDescSet);

    void setAlphaValue(VkCommandBuffer _cmdBuffer, float alpha);
    void setAlphaBlendValue(VkCommandBuffer _cmdBuffer, float alpha);
    void setColor(VkCommandBuffer _cmdBuffer, const glm::vec3& color);
    void setTransformationMatrix(VkCommandBuffer _cmdBuffer, const glm::mat4& transfo);
    void setObjectId(VkCommandBuffer _cmdBuffer, uint32_t id);
    void setObjectFlags(VkCommandBuffer _cmdBuffer, bool isHovered, bool isSelected);
    void setAllConstants(VkCommandBuffer _cmdBuffer, MeshPushConstant& constants);

protected:
    void initRenderer();

    void cleanup();
    void cleanShaders();

    void createDataBuffer();
    void createShaders();
    void createPipelineResources();

    void createDescriptorSetLayout();
    void createDescriptorSets();
    void createPipelineLayout();
    void createPipelineFace(VkPrimitiveTopology topology, bool transparent, bool hasInputNormals);
    void createPipelineEdge(VkPrimitiveTopology topology);
    void createPipelineBlend();

    VkPipeline getPipeline(VkPrimitiveTopology topology, uint32_t subpass, bool normals, TlTopologyFlags allowedTopologies);
    bool bindPipelineResources(VkCommandBuffer cmdBuf, VkPrimitiveTopology topology, uint32_t subpass, VkUniformOffset mvpUni, bool useNormals, TlTopologyFlags allowedTopologies);

protected:
    // Resources NOT owned by the Renderer --> DO NOT DESTROY HERE
    VkDevice h_device = VK_NULL_HANDLE;
    VulkanDeviceFunctions* h_pfn = nullptr;
    VkDescriptorPool h_descPool = VK_NULL_HANDLE;
    VkRenderPass h_renderPass = VK_NULL_HANDLE;

    SimpleBuffer m_quadTriangleBuffer;

    // Resources owned by the Renderer
    Shader m_vertFacesShader;
    Shader m_geomFacesShader;
    Shader m_vertFacesNormalsShader;
    Shader m_fragFacesShader;
    Shader m_vertEdgesShader;
    Shader m_fragEdgesShader;
    Shader m_vertBlendShader;
    Shader m_fragBlendShader;

    VkDescriptorSetLayout m_descSetLayout_vf = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descSetLayout_vgf = VK_NULL_HANDLE;
    VkDescriptorSet m_descSet_vf = VK_NULL_HANDLE;
    VkDescriptorSet m_descSet_vgf = VK_NULL_HANDLE;

    VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;
    std::unordered_set<PipelineDef> m_pipelines;
    VkPipeline m_pipelineBlend = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineMeshLayout_vf = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineMeshLayout_vgf = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineBlendLayout = VK_NULL_HANDLE;
};

#endif
