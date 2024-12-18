#include "ComputePrograms.h"
#include "vulkan/VulkanManager.h"
#include "utils/Logger.h"

#include "vulkan/VulkanHelperFunctions.h"

#include <glm/gtc/matrix_transform.hpp>

#define VKLOG LoggerMode::VKLog

static const int UNIFORM_DATA_SIZE = 16 * sizeof(float);

// ********** Shaders Declaration *************

static std::vector<uint32_t> mesh_dist_comp_spv =
{
#include "mesh_dist.comp.spv"
};


// ********************************************

ComputePrograms::ComputePrograms()
{
    VulkanManager& vkm = VulkanManager::getInstance();

    h_device = vkm.getDevice();
    h_pfn = vkm.getDeviceFunctions();
    h_descPool = vkm.getDescriptorPool();

    createDescriptorSetLayout();
    createDescriptorSets();
    createPipelines();
}

ComputePrograms::~ComputePrograms()
{
    cleanup();
}

void ComputePrograms::processMeshDistance(VkCommandBuffer _cmdBuffer, VkUniformOffset _modelUniOffset, VkBuffer meshBuffer, uint32_t _vertexCount)
{
    if (m_pipelines.find(tlb::ComputeType::Distance_point_to_mesh) == m_pipelines.end())
        return;
    VkPipeline pipeline = m_pipelines.at(tlb::ComputeType::Distance_point_to_mesh);
    h_pfn->vkCmdBindPipeline(_cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

    uint32_t offsets[] = { _modelUniOffset };
    h_pfn->vkCmdBindDescriptorSets(_cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 1, &m_descSetModelMatrix, 1, offsets);

    //bindDescriptorBuffers(meshBuffer);

    h_pfn->vkCmdBindDescriptorSets(_cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 1, 1, &m_descSetBuffers, 0, nullptr);

    constexpr uint32_t invocationCount = 16 * 16 * 1;
    uint32_t groupCountX = (_vertexCount + invocationCount - 1) / invocationCount;
    h_pfn->vkCmdDispatch(_cmdBuffer, groupCountX, 1, 1);
}

inline void ComputePrograms::loadShaderSPV(Shader& shader, const std::vector<uint32_t>& code_spv)
{
    shader.createModule(h_device, h_pfn, code_spv.data(), code_spv.size() * 4);
    if (!shader.isValid())
        check_vk_result(VK_ERROR_INVALID_SHADER_NV, "Missing shader");
}

void ComputePrograms::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding lb_model =
    {
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        1,
        VK_SHADER_STAGE_COMPUTE_BIT,
        nullptr
    };

    VkDescriptorSetLayoutBinding lb_buffers[] =
    {
        {
            1,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            1,
            VK_SHADER_STAGE_COMPUTE_BIT,
            nullptr
        },
        {
            2,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            1,
            VK_SHADER_STAGE_COMPUTE_BIT,
            nullptr
        }
    };

    VkDescriptorSetLayoutCreateInfo descLayoutInfo = {};
    descLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descLayoutInfo.bindingCount = sizeof(lb_model) / sizeof(VkDescriptorSetLayoutBinding);
    descLayoutInfo.pBindings = &lb_model;

    VkResult err = h_pfn->vkCreateDescriptorSetLayout(h_device, &descLayoutInfo, nullptr, &m_descSetLayoutModelMatrix);
    check_vk_result(err, "Create Descriptor Set Layout");

    descLayoutInfo.bindingCount = sizeof(lb_buffers) / sizeof(VkDescriptorSetLayoutBinding);
    descLayoutInfo.pBindings = lb_buffers;

    err = h_pfn->vkCreateDescriptorSetLayout(h_device, &descLayoutInfo, nullptr, &m_descSetLayoutBuffers);
    check_vk_result(err, "Create Descriptor Set Layout");
}

void ComputePrograms::createDescriptorSets()
{
    VkDescriptorSetAllocateInfo descSetAllocInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        nullptr,
        h_descPool,
        1,
        &m_descSetLayoutModelMatrix
    };
    VkResult err = h_pfn->vkAllocateDescriptorSets(h_device, &descSetAllocInfo, &m_descSetModelMatrix);
    check_vk_result(err, "Allocate Descriptor Sets");

    VulkanManager& vkManager = VulkanManager::getInstance();

    VkBuffer uniBuf = vkManager.getUniformBuffer();

    // Check the uniform size(range for vulkan).
    // For future use with an array of matrices. The spec guarante that the max is minimum 16384 bytes.
    VkPhysicalDeviceLimits devLimits = vkManager.getPhysicalDeviceLimits();
    if (vkManager.getUniformSizeAligned(64) > devLimits.maxUniformBufferRange)
        Logger::log(VK_LOG) << "Warning: the uniform range is too large for the device." << Logger::endl;

    // Write descriptors for the uniform buffers in the vertex and fragment shaders.
    VkDescriptorBufferInfo mat4UniInfo = {
        uniBuf, // buffer
        0, // offset
        vkManager.getUniformSizeAligned(64) // range
    };

    VkWriteDescriptorSet descWrite = {};
    descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descWrite.descriptorCount = 1;
    descWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descWrite.pBufferInfo = &mat4UniInfo;
    descWrite.dstSet = m_descSetModelMatrix;
    descWrite.dstBinding = 0;

    h_pfn->vkUpdateDescriptorSets(h_device, sizeof(descWrite) / sizeof(VkWriteDescriptorSet), &descWrite, 0, nullptr);

    //a*****************************a

    VkDescriptorSetAllocateInfo allocInfo_buffers = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        nullptr,
        h_descPool,
        1,
        &m_descSetLayoutBuffers
    };
    err = h_pfn->vkAllocateDescriptorSets(h_device, &allocInfo_buffers, &m_descSetBuffers);
    check_vk_result(err, "Allocate Descriptor Sets");
}

void ComputePrograms::createPipelines()
{
    VkPipelineCacheCreateInfo pipelineCacheInfo;
    memset(&pipelineCacheInfo, 0, sizeof(pipelineCacheInfo));
    pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VkResult err = h_pfn->vkCreatePipelineCache(h_device, &pipelineCacheInfo, nullptr, &m_pipelineCache);
    check_vk_result(err, "Create Pipeline Cache");

    // First, create the pipeline layouts that will be used by the graphics pipelines
    createPipelineLayouts();

    // Create the pipelines for all the programs defined
    for (const ProgramDef& programDef : m_programDefs)
    {
        createPipeline(programDef);
    }
}

void ComputePrograms::createPipelineLayouts()
{
    VkResult err;

    VkPushConstantRange pcr[] =
    {
        {
            VK_SHADER_STAGE_COMPUTE_BIT,
            0,
            64
        }
    };

    VkDescriptorSetLayout layouts[] = { m_descSetLayoutBuffers, m_descSetLayoutModelMatrix };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = sizeof(pcr) / sizeof(VkPushConstantRange);
    pipelineLayoutInfo.pPushConstantRanges = pcr;
    pipelineLayoutInfo.setLayoutCount = sizeof(layouts) / sizeof(VkDescriptorSetLayout);
    pipelineLayoutInfo.pSetLayouts = layouts;
    err = h_pfn->vkCreatePipelineLayout(h_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
    check_vk_result(err, "Create Pipeline Layout");
}


void ComputePrograms::createPipeline(const ProgramDef& program)
{
    Shader compShader;
    loadShaderSPV(compShader, program.shader_spv);

    VkPipelineShaderStageCreateInfo compStageInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        VK_SHADER_STAGE_COMPUTE_BIT,
        compShader.module(),
        "main",
        nullptr
    };

    // Graphics pipeline creation
    VkComputePipelineCreateInfo info = {
        VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        nullptr,                  // pNext
        0,                        // flags
        compStageInfo,            // stage
        m_pipelineLayout,         // layout
        VK_NULL_HANDLE,           // basePipelineHandle
        0,                        // basePipelineIndex
    };

    VkPipeline pipeline;
    VkResult err = h_pfn->vkCreateComputePipelines(h_device, m_pipelineCache, 1, &info, nullptr, &pipeline);
    check_vk_result(err, "Create Compute Pipeline");

    assert(m_pipelines.find(program.type) != m_pipelines.end());
    m_pipelines.insert({ program.type, pipeline });

    // Shader’s destructor free resources.
}


void ComputePrograms::cleanup()
{
    //*** Common Resources ***
    tls::vk::destroyPipelineCache(*h_pfn, h_device, m_pipelineCache);

    tls::vk::destroyDescriptorSetLayout(*h_pfn, h_device, m_descSetLayoutModelMatrix);
    tls::vk::destroyDescriptorSetLayout(*h_pfn, h_device, m_descSetLayoutBuffers);

    tls::vk::destroyPipelineLayout(*h_pfn, h_device, m_pipelineLayout);

    //*** Pipelines ***
    for (auto pipelineKey : m_pipelines)
    {
        tls::vk::destroyPipeline(*h_pfn, h_device, pipelineKey.second);
    }
    m_pipelines.clear();
}

void ComputePrograms::bindDescriptorBuffers(VkBuffer meshBuffer, VkDeviceSize meshBufferSize, VkBuffer outputBuffer, VkDeviceSize outputBufferSize)
{
    VkDescriptorBufferInfo depthInfo[] = {
        {
            meshBuffer,
            0,
            meshBufferSize
        },
        {
            outputBuffer,
            0,
            outputBufferSize
        }
    };

    VkWriteDescriptorSet writeDesc = {};
    writeDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDesc.dstSet = m_descSetBuffers;
    writeDesc.dstBinding = 1;
    writeDesc.descriptorCount = 2;
    writeDesc.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDesc.pBufferInfo = depthInfo;

    h_pfn->vkUpdateDescriptorSets(h_device, 1, &writeDesc, 0, nullptr);
}

/* From shader code :
   layout(offset = 0) uint vertexCount;
   layout(offset = 4) uint vertexOffset;
   layout(offset = 8) int topology;
   layout(offset = 16) vec3 point;
   */
void ComputePrograms::setMeshConstants(VkCommandBuffer _cmdBuffer, uint32_t vertexCount, uint32_t vertexOffset, VkPrimitiveTopology topology)
{
    /*
    struct PackConstants {
        uint32_t vertexCount;
        uint32_t vertexOffset;
        int topoCode;
    };*/
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, 4, &vertexCount);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 4, 4, &vertexOffset);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 8, 4, &topology);
}

void ComputePrograms::setPointConstant(VkCommandBuffer _cmdBuffer, glm::vec3 point)
{
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 16, 12, &point);
}