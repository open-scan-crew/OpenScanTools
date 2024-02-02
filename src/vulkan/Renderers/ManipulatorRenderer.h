#ifndef MANIPULATOR_RENDERER_H
#define MANIPULATOR_RENDERER_H

#include "vulkan/Shader.h"
#include "vulkan/vulkan.h"
#include "models/3d/MeasureData.h"
#include "vulkan/VkUniform.h"
#include "models/3d/ManipulationTypes.h"

#include <glm/glm.hpp>

class MeshBuffer;

class ManipulatorRenderer
{
public:
    ManipulatorRenderer();
    ~ManipulatorRenderer();

    void draw(const glm::mat4& tranformation, const glm::dvec3& color, Selection select, VkCommandBuffer _cmdBuffer, VkUniformOffset mvpUni, std::shared_ptr<MeshBuffer> mesh);

protected:
    void cleanup();

    virtual void createShaders();
    virtual void createPipelineResources();

    virtual void createDescriptorSetLayout();
    virtual void createDescriptorSets();
    virtual void createPipelineLayout();
    virtual void createPipeline();

    // Destruction helper functions
    void destroyPipeline(VkPipeline& pipeline);

    void setColor(VkCommandBuffer _cmdBuffer, const glm::vec3& color);
    void setTranformationMatrix(VkCommandBuffer _cmdBuffer, const glm::mat4& transfo);
    void setManipulatorId(VkCommandBuffer _cmdBuffer, uint32_t id);

protected:
    // Resources NOT owned by the Renderer --> DO NOT DESTROY HERE
    VkDevice h_device = VK_NULL_HANDLE;
    VulkanDeviceFunctions* h_pfn = nullptr;
    VkDescriptorPool h_descPool = VK_NULL_HANDLE;
    VkRenderPass h_renderPass = VK_NULL_HANDLE;

    // Resources owned by the Renderer
    Shader m_vertShader;
    Shader m_geomShader;
    Shader m_fragShader;

    VkDescriptorSetLayout m_descSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet m_descSet;

    VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
};

#endif //! MANIPULATOR_RENDERER_H_
