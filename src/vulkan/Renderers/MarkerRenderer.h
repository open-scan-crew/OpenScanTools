#ifndef MARKER_RENDERER_H
#define MARKER_RENDERER_H

#include "vulkan/Shader.h"
#include "vulkan/vulkan.h"
#include "vulkan/VkUniform.h"
#include "pointCloudEngine/SmartBuffer.h"
#include "models/3d/MarkerDrawData.h"
#include "models/data/Marker.h"

#include "glm/glm.hpp"

class VulkanDeviceFunctions;
class AObjectNode;

class MarkerRenderer
{
public:
    MarkerRenderer();
    ~MarkerRenderer();

    static MarkerDrawData getMarkerDrawData(const glm::dmat4& gTransfo, const AObjectNode& _obj);
    static MarkerDrawData getTargetDrawData(const glm::dmat4& gTransfo, const AGraphNode& _node);
    // General function
    void drawMarkerData(VkCommandBuffer _cmdBuffer, SimpleBuffer drawDataBuf, uint32_t firstMarker, uint32_t markerCount, VkUniformOffset viewProjUniOffset, VkDescriptorSet depthDescSet);

    void setScaleConstants(VkCommandBuffer _cmdBuffer, float nearScale, float farScale, float nearDist, float farDist);

    void setBordersColorConstant(VkCommandBuffer _cmdBuffer, glm::vec4 neutral, glm::vec4 hover, glm::vec4 select);
    void setDepthRenderConstant(VkCommandBuffer _cmdBuffer, bool _useDepthRender);
    void setViewportSizeConstants(VkCommandBuffer _cmdBuffer, VkExtent2D extent);


private:
    void cleanup();

    void createShaders();

    void createDescriptorSet_camera();
    void createDescriptorSet_camera_geom();
    void createDescriptorSet_sampler();
    void createDescriptorSet_storageBuffers();

    void createPipelines();
    void createPipelineLayout_marker_dyn();
    void createGraphicPipeline_marker_dyn();

    void createPrimitives();
    void createSampler();

    // Handles to resources NOT owned by the Renderer --> DO NOT DESTROY
    VkDevice h_device = VK_NULL_HANDLE;
    VulkanDeviceFunctions* h_pfn = nullptr;
    VkDescriptorPool h_descPool = VK_NULL_HANDLE;
    VkRenderPass h_renderPass = VK_NULL_HANDLE;

    // Resources owned by the Renderer
    // Shader vk_vsh
    Shader m_vsh_marker;
    Shader m_gsh_marker;
    Shader m_fsh_marker;

    // VkDescriptorSetLayout
    VkDescriptorSetLayout m_dsly_camera = VK_NULL_HANDLE;
    VkDescriptorSet m_ds_camera;
    VkDescriptorSetLayout m_dsly_camera_geom = VK_NULL_HANDLE;
    VkDescriptorSet m_ds_camera_geom;
    VkDescriptorSetLayout m_dsly_sampler = VK_NULL_HANDLE;
    VkDescriptorSet m_ds_sampler;
    VkDescriptorSetLayout m_dsly_storageBuffers = VK_NULL_HANDLE;
    VkDescriptorSet m_ds_storageBuffers;

    // Pipelines
    VkPipelineLayout m_pply_marker_dyn = VK_NULL_HANDLE;
    VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;
    VkPipeline m_pp_marker_dyn;

    // Primitives buffer
    SimpleBuffer m_vertPosBuf;
    SimpleBuffer m_vertUVBuf;

    // Markers Textures
    VkImage m_textureImage = VK_NULL_HANDLE;
    VkImageView m_textureImageView = VK_NULL_HANDLE;
    VkSampler m_textureSampler = VK_NULL_HANDLE;
    VkDeviceMemory m_textureMemory = VK_NULL_HANDLE;
};

#endif // MARKER_RENDERER_H