#ifndef MEASURE_RENDERER_H
#define MEASURE_RENDERER_H

#include "vulkan/Shader.h"
#include "vulkan/vulkan.h"
#include "pointCloudEngine/SmartBuffer.h"
#include "models/3d/SegmentDrawData.h"

#include <glm/glm.hpp>

class VulkanDeviceFunctions;

class MeasureRenderer
{
public:
    MeasureRenderer();
    ~MeasureRenderer() { cleanup(); };

private:
    void cleanup();

    void createShaders();
    void createPipelineResources();

    void createDescriptorSetLayout();
    void createDescriptorSets();
    void createPipelineLayout();
    void createPipelines();

    // Destruction helper functions
    void destroyPipeline(VkPipeline& pipeline);

public:
    void drawMeasures(VkCommandBuffer _cmdBuffer, uint32_t vpUniOffset, VkBuffer segmentBuffer, uint32_t segmentCount);

    void setPointSize(VkCommandBuffer _cmdBuffer, float size);
    void setStripCount(VkCommandBuffer _cmdBuffer, float count);
    void setLightRay(VkCommandBuffer _cmdBuffer, glm::vec3 direction);
    void setScreenParameters(VkCommandBuffer _cmdBuffer, float pixelHAt1m, float nearZ, float farZ);
    void setMeasureShowMask(VkCommandBuffer _cmdBuffer, uint32_t mask);
    void setObjectFlags(VkCommandBuffer _cmdBuffer, bool isHovered, bool isSelected, bool isActivated, bool isExterior);

private:
    // Resources NOT owned by the Renderer --> DO NOT DESTROY HERE
    VkDevice h_device = VK_NULL_HANDLE;
    VulkanDeviceFunctions* h_pfn = nullptr;
    VkDescriptorPool h_descPool = VK_NULL_HANDLE;
    VkRenderPass h_renderPass = VK_NULL_HANDLE;

    // Resources owned by the Renderer
    Shader m_vertShader_line;
    Shader m_geomShader_line;
    Shader m_fragShader_line;
    Shader m_vertShader_point;
    Shader m_geomShader_point;
    Shader m_fragShader_point;

    VkDescriptorSetLayout m_descSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet m_descSet;

    VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;

    VkPipeline m_pipeline_line = VK_NULL_HANDLE;
    VkPipeline m_pipeline_point = VK_NULL_HANDLE;

    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
};

#endif