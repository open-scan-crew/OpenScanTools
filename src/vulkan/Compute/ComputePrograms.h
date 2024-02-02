#ifndef COMPUTE_PROGRAMS_H
#define COMPUTE_PROGRAMS_H

#include "vulkan/Shader.h"
#include "vulkan/vulkan.h"
#include "vulkan/VkUniform.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <vector>
#include <unordered_map>
#include <set>
#include <string>

class VulkanDeviceFunctions;


namespace tlb
{
    enum class ComputeType
    {
        Distance_point_to_mesh,
        Distance_mesh_to_mesh,
        //...
    };
}

class ComputePrograms
{
public:
    ComputePrograms();
    ~ComputePrograms();

    void processMeshDistance(VkCommandBuffer _cmdBuffer, VkUniformOffset modelUniOffset, VkBuffer meshBuffer, uint32_t vertexCount);
    void bindDescriptorBuffers(VkBuffer meshBuffer, VkDeviceSize meshBufferSizeVkBuffer, VkBuffer outputBuffer, VkDeviceSize outputBufferSize);

    void setMeshConstants(VkCommandBuffer _cmdBuffer, uint32_t vertexCount, uint32_t vertexOffset, VkPrimitiveTopology topology);
    void setPointConstant(VkCommandBuffer _cmdBuffer, glm::vec3 point);

    void cleanup();

private:
    struct ProgramDef {
        tlb::ComputeType type;
        std::string name; // For debbuging and knowing what it does
        std::vector<uint32_t>& shader_spv; // Link to the static spv include
        // Constants defs
        // ...
    };


    //void createShaders();
    void createDescriptorSetLayout();
    void createDescriptorSets();
    void createPipelines();
    void createPipelineLayouts();
    void createPipeline(const ProgramDef& program);

    void loadShaderSPV(Shader& shader, const std::vector<uint32_t>& code_spv);

private:
    // Resources NOT owned by the Renderer --> DO NOT DESTROY HERE
    VkDevice h_device = VK_NULL_HANDLE;
    VulkanDeviceFunctions* h_pfn = nullptr;
    VkDescriptorPool h_descPool = VK_NULL_HANDLE;

    // Resources owned by the Renderer
    VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;

    VkDescriptorSetLayout m_descSetLayoutModelMatrix = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descSetLayoutBuffers = VK_NULL_HANDLE;
    VkDescriptorSet m_descSetModelMatrix;
    VkDescriptorSet m_descSetBuffers;

    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

    std::vector<ProgramDef> m_programDefs;

    std::unordered_map<tlb::ComputeType, VkPipeline> m_pipelines;
};

#endif
