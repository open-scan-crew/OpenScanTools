#include "PostRenderer.h"
#include "vulkan/VulkanManager.h"
#include "utils/Logger.h"

#include <glm/gtc/matrix_transform.hpp>

static const int UNIFORM_DATA_SIZE = 16 * sizeof(float);

// ********** Shaders Declaration *************

static std::vector<uint32_t> filling_comp_spv = 
{
#include "fillingGap.comp.spv"
};

static std::vector<uint32_t> normal_shading_comp_spv = 
{
#include "normal_shading.comp.spv"
};

static std::vector<uint32_t> normal_colored_comp_spv =
{
#include "normal_colored.comp.spv"
};

static std::vector<uint32_t> transparency_hdr_comp_spv =
{
#include "transparency_hdr.comp.spv"
};

// ********************************************

PostRenderer::PostRenderer()
{
    VulkanManager& vkm = VulkanManager::getInstance();

    h_device = vkm.getDevice();
    h_pfn = vkm.getDeviceFunctions();
    h_descPool = vkm.getDescriptorPool();

    createShaders();
    createDescriptorSetLayout();
    createDescriptorSets();
    createPipelines();
}

PostRenderer::~PostRenderer()
{
    cleanup();
}

inline void PostRenderer::loadShaderSPV(Shader& shader, const std::vector<uint32_t>& code_spv)
{
    shader.createModule(h_device, h_pfn, code_spv.data(), code_spv.size() * 4);
    if (!shader.isValid())
        check_vk_result(VK_ERROR_INVALID_SHADER_NV, "Missing fragment shader");
}

void PostRenderer::createShaders()
{
    loadShaderSPV(m_fillingCompShader, filling_comp_spv);
    loadShaderSPV(m_normalShadingCompShader, normal_shading_comp_spv);
    loadShaderSPV(m_normalColoredCompShader, normal_colored_comp_spv);
    loadShaderSPV(m_transparencyHDRCompShader, transparency_hdr_comp_spv);
}

void PostRenderer::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding layoutBindings =
    {
        4,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        1,
        VK_SHADER_STAGE_COMPUTE_BIT,
        nullptr
    };

    VkDescriptorSetLayoutCreateInfo descLayoutInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        nullptr,
        0, // bits defined by extensions
        sizeof(layoutBindings) / sizeof(VkDescriptorSetLayoutBinding), // bindingCount
        &layoutBindings
    };

    VkResult err = h_pfn->vkCreateDescriptorSetLayout(h_device, &descLayoutInfo, nullptr, &m_descSetLayoutMatrixCompute);
    check_vk_result(err, "Create Descriptor Set Layout");
}

void PostRenderer::createDescriptorSets()
{
    VkDescriptorSetAllocateInfo descSetAllocInfo2 = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        nullptr,
        h_descPool,
        1,
        &m_descSetLayoutMatrixCompute
    };
    VkResult err = h_pfn->vkAllocateDescriptorSets(h_device, &descSetAllocInfo2, &m_descSetMatrixCompute);
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
    descWrite.dstSet = m_descSetMatrixCompute;
    descWrite.dstBinding = 4;

    h_pfn->vkUpdateDescriptorSets(h_device, sizeof(descWrite) / sizeof(VkWriteDescriptorSet), &descWrite, 0, nullptr);
}

void PostRenderer::createPipelines()
{
    VkPipelineCacheCreateInfo pipelineCacheInfo;
    memset(&pipelineCacheInfo, 0, sizeof(pipelineCacheInfo));
    pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VkResult err = h_pfn->vkCreatePipelineCache(h_device, &pipelineCacheInfo, nullptr, &m_pipelineCache);
    check_vk_result(err, "Create Pipeline Cache");

    // First, create the pipeline layouts that will be used by the graphics pipelines
    createPipelineLayouts();

    // Create all the pipelines defined
    createFillingPipeline();
    createNormalPipeline();
    createTransparencyHDRPipeline();
}

void PostRenderer::createPipelineLayouts()
{
    VkResult err;

    VkPushConstantRange pcr_2[] =
    {
        {
            VK_SHADER_STAGE_COMPUTE_BIT,
            0,
            64
        }
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = sizeof(pcr_2) / sizeof(VkPushConstantRange);
    pipelineLayoutInfo.pPushConstantRanges = pcr_2;
    VkDescriptorSetLayout setLayouts[] = { VulkanManager::getDSLayout_fillingSamplers(), VulkanManager::getDSLayout_finalOutput() };
    pipelineLayoutInfo.setLayoutCount = sizeof(setLayouts) / sizeof(VkDescriptorSetLayout);
    pipelineLayoutInfo.pSetLayouts = setLayouts;
    err = h_pfn->vkCreatePipelineLayout(h_device, &pipelineLayoutInfo, nullptr, &m_fillingPipelineLayout);
    check_vk_result(err, "Create Pipeline Layout");

    VkDescriptorSetLayout DSLayout_normal[] = { VulkanManager::getDSLayout_fillingSamplers(), VulkanManager::getDSLayout_finalOutput(), m_descSetLayoutMatrixCompute };
    pipelineLayoutInfo.setLayoutCount = sizeof(DSLayout_normal) / sizeof(VkDescriptorSetLayout);
    pipelineLayoutInfo.pSetLayouts = DSLayout_normal;
    err = h_pfn->vkCreatePipelineLayout(h_device, &pipelineLayoutInfo, nullptr, &m_normalPipelineLayout);
    check_vk_result(err, "Create Pipeline Layout");

    VkDescriptorSetLayout setLayouts_tHDR[] = { VulkanManager::getDSLayout_fillingSamplers() };
    pipelineLayoutInfo.setLayoutCount = sizeof(setLayouts_tHDR) / sizeof(VkDescriptorSetLayout);
    pipelineLayoutInfo.pSetLayouts = setLayouts_tHDR;
    err = h_pfn->vkCreatePipelineLayout(h_device, &pipelineLayoutInfo, nullptr, &m_transparencyHDRPipelineLayout);
    check_vk_result(err, "Create Pipeline Layout");
}


void PostRenderer::createFillingPipeline()
{
    VkPipelineShaderStageCreateInfo compStageInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        VK_SHADER_STAGE_COMPUTE_BIT,
        m_fillingCompShader.module(),
        "main",
        nullptr
    };

    // Graphics pipeline creation
    VkComputePipelineCreateInfo info = {
        VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        nullptr,                  // pNext
        0,                        // flags
        compStageInfo,            // stage
        m_fillingPipelineLayout,  // layout
        VK_NULL_HANDLE,           // basePipelineHandle
        0,                        // basePipelineIndex
    };

    VkResult err = h_pfn->vkCreateComputePipelines(h_device, m_pipelineCache, 1, &info, nullptr, &m_fillingPipeline);
    check_vk_result(err, "Create Compute Pipeline");
}

void PostRenderer::createNormalPipeline()
{
    VkPipelineShaderStageCreateInfo compStageInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        VK_SHADER_STAGE_COMPUTE_BIT,
        m_normalShadingCompShader.module(),
        "main",
        nullptr
    };

    // Graphics pipeline creation
    VkComputePipelineCreateInfo info = {
        VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        nullptr,                  // pNext
        0,                        // flags
        compStageInfo,            // stage
        m_normalPipelineLayout,   // layout
        VK_NULL_HANDLE,           // basePipelineHandle
        0,                        // basePipelineIndex
    };

    VkResult err = h_pfn->vkCreateComputePipelines(h_device, m_pipelineCache, 1, &info, nullptr, &m_normalShadingPipeline);
    check_vk_result(err, "Create Compute Pipeline");

    compStageInfo.module = m_normalColoredCompShader.module();
    info.stage = compStageInfo;
    err = h_pfn->vkCreateComputePipelines(h_device, m_pipelineCache, 1, &info, nullptr, &m_normalColoredPipeline);
    check_vk_result(err, "Create Compute Pipeline");
}

void PostRenderer::createTransparencyHDRPipeline()
{
    VkPipelineShaderStageCreateInfo compStageInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        VK_SHADER_STAGE_COMPUTE_BIT,
        m_transparencyHDRCompShader.module(),
        "main",
        nullptr
    };

    // Graphics pipeline creation
    VkComputePipelineCreateInfo info = {
        VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        nullptr,                  // pNext
        0,                        // flags
        compStageInfo,            // stage
        m_transparencyHDRPipelineLayout,   // layout
        VK_NULL_HANDLE,           // basePipelineHandle
        0,                        // basePipelineIndex
    };

    VkResult err = h_pfn->vkCreateComputePipelines(h_device, m_pipelineCache, 1, &info, nullptr, &m_transparencyHDRPipeline);
    check_vk_result(err, "Create Compute Pipeline");
}

// Must be called inside a renderpass
void PostRenderer::setViewportAndScissor(int32_t _xPos, int32_t _yPos, uint32_t _width, uint32_t _height, VkCommandBuffer _cmdBuffer)
{
    VkViewport viewport = {
    float(_xPos), float(_yPos),
    float(_width), float(_height),
    0, 1
    };
    h_pfn->vkCmdSetViewport(_cmdBuffer, 0, 1, &viewport);

    VkRect2D scissor = {
        { _xPos, _yPos },
        { _width, _height }
    };
    h_pfn->vkCmdSetScissor(_cmdBuffer, 0, 1, &scissor);
}

void PostRenderer::processPointFilling(VkCommandBuffer _cmdBuffer, VkDescriptorSet descSetSamplers, VkDescriptorSet descSetOutput, VkExtent2D _extent)
{
    // Bind the pipeline
    h_pfn->vkCmdBindPipeline(_cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_fillingPipeline);

    VkDescriptorSet descSets[] = { descSetSamplers, descSetOutput };
    h_pfn->vkCmdBindDescriptorSets(_cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_fillingPipelineLayout, 0, 2, descSets, 0, nullptr);

    h_pfn->vkCmdDispatch(_cmdBuffer, (_extent.width + 15) / 16, (_extent.height + 15) / 16, 1);
}

void PostRenderer::processNormalShading(VkCommandBuffer _cmdBuffer, VkUniformOffset matrixUniOffset, VkDescriptorSet descSetColor, VkDescriptorSet descSetOutput, VkExtent2D _extent)
{
    // Bind the pipeline
    h_pfn->vkCmdBindPipeline(_cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_normalShadingPipeline);

    VkDescriptorSet descSets[] = { descSetColor, descSetOutput };
    h_pfn->vkCmdBindDescriptorSets(_cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_normalPipelineLayout, 0, 2, descSets, 0, nullptr);

    uint32_t offsets[] = { matrixUniOffset };
    h_pfn->vkCmdBindDescriptorSets(_cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_normalPipelineLayout, 2, 1, &m_descSetMatrixCompute, 1, offsets);

    h_pfn->vkCmdDispatch(_cmdBuffer, (_extent.width + 15) / 16, (_extent.height + 15) / 16, 1);
}

void PostRenderer::processNormalColored(VkCommandBuffer _cmdBuffer, VkUniformOffset matrixUniOffset, VkDescriptorSet descSetColor, VkDescriptorSet descSetOutput, VkExtent2D _extent)
{
    // Bind the pipeline
    h_pfn->vkCmdBindPipeline(_cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_normalColoredPipeline);

    VkDescriptorSet descSets[] = { descSetColor, descSetOutput };
    h_pfn->vkCmdBindDescriptorSets(_cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_normalPipelineLayout, 0, 2, descSets, 0, nullptr);

    uint32_t offsets[] = { matrixUniOffset };
    h_pfn->vkCmdBindDescriptorSets(_cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_normalPipelineLayout, 2, 1, &m_descSetMatrixCompute, 1, offsets);

    h_pfn->vkCmdDispatch(_cmdBuffer, (_extent.width + 15) / 16, (_extent.height + 15) / 16, 1);
}

void PostRenderer::processTransparencyHDR(VkCommandBuffer _cmdBuffer, VkDescriptorSet descSetColor, VkExtent2D _extent)
{
    // Bind the pipeline
    h_pfn->vkCmdBindPipeline(_cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_transparencyHDRPipeline);

    VkDescriptorSet descSets[] = { descSetColor };
    h_pfn->vkCmdBindDescriptorSets(_cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_transparencyHDRPipelineLayout, 0, 1, descSets, 0, nullptr);

    h_pfn->vkCmdDispatch(_cmdBuffer, (_extent.width + 15) / 16, (_extent.height + 15) / 16, 1);
}

void PostRenderer::setConstantZRange(float nearZ, float farZ, VkCommandBuffer _cmdBuffer)
{
    float nearFar[2] = { nearZ, farZ };
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_fillingPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(nearFar), nearFar);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_normalPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(nearFar), nearFar);
}

void PostRenderer::setConstantScreenSize(VkExtent2D screenSize, glm::vec2 pixelSize, VkCommandBuffer _cmdBuffer)
{
    // NOTE - The pixels are always squared in our application, so dimension size is enough.
    // NOTE - But the general case of a non-squared pixel is covered in the normal shader because it does not cost much and we need the precise position of each pixels. 
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_fillingPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 16, 4, &pixelSize.x);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_fillingPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 8, sizeof(screenSize), &screenSize);

    h_pfn->vkCmdPushConstants(_cmdBuffer, m_normalPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 24, sizeof(pixelSize), &pixelSize);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_normalPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 8, sizeof(screenSize), &screenSize);
}

void PostRenderer::setConstantScreenOffset(glm::vec2 _offset, VkCommandBuffer _cmdBuffer)
{
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_normalPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 16, sizeof(glm::vec2), &_offset);
}

void PostRenderer::setConstantProjMode(bool isPerspective, VkCommandBuffer _cmdBuffer)
{
    int mode = isPerspective ? 0x0 : 0x1;
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_fillingPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 20, 4, &mode);

    h_pfn->vkCmdPushConstants(_cmdBuffer, m_normalPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 32, 4, &mode);
}

void PostRenderer::setConstantTexelThreshold(int texelThreshold, VkCommandBuffer _cmdBuffer)
{
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_fillingPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 56, 4, &texelThreshold);
}

void PostRenderer::setConstantLighting(const PostRenderingNormals& lighting, VkCommandBuffer _cmdBuffer) const
{
    int tone = lighting.inverseTone ? 1 : 0;
    int blend = lighting.blendColor ? 1 : 0;
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_normalPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 36, 4, &tone);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_normalPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 40, 4, &lighting.normalStrength);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_normalPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 48, 4, &lighting.gloss);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_normalPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 52, 4, &blend);
}

void PostRenderer::setConstantHDR(float opacity, bool substract, bool noFlash, VkExtent2D screenSize, Color32 background, VkCommandBuffer _cmdBuffer)
{
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_transparencyHDRPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, 4, &opacity);
    int mode = substract ? 1 : 0;
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_transparencyHDRPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 4, 4, &mode);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_transparencyHDRPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 8, 8, &screenSize);
    float background_f[3] = { background.Red_f(), background.Green_f(), background.Blue_f() };
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_transparencyHDRPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 16, 12, background_f);
    int iNoFlash = noFlash ? 1 : 0;
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_transparencyHDRPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 28, 4, &iNoFlash);
}

void PostRenderer::cleanup()
{
    //*** Common Resources ***
    if (m_pipelineCache) {
        h_pfn->vkDestroyPipelineCache(h_device, m_pipelineCache, nullptr);
        m_pipelineCache = VK_NULL_HANDLE;
    }

    if (m_descSetLayoutMatrixCompute) {
        h_pfn->vkDestroyDescriptorSetLayout(h_device, m_descSetLayoutMatrixCompute, nullptr);
        m_descSetLayoutMatrixCompute = VK_NULL_HANDLE;
    }

    if (m_fillingPipelineLayout) {
        h_pfn->vkDestroyPipelineLayout(h_device, m_fillingPipelineLayout, nullptr);
        m_fillingPipelineLayout = VK_NULL_HANDLE;
    }

    if (m_normalPipelineLayout) {
        h_pfn->vkDestroyPipelineLayout(h_device, m_normalPipelineLayout, nullptr);
        m_normalPipelineLayout = VK_NULL_HANDLE;
    }

    if (m_transparencyHDRPipelineLayout) {
        h_pfn->vkDestroyPipelineLayout(h_device, m_transparencyHDRPipelineLayout, nullptr);
        m_transparencyHDRPipelineLayout = VK_NULL_HANDLE;
    }

    //*** Pipelines ***
    if (m_fillingPipeline)
    {
        h_pfn->vkDestroyPipeline(h_device, m_fillingPipeline, nullptr);
        m_fillingPipeline = VK_NULL_HANDLE;
    }

    if (m_normalShadingPipeline)
    {
        h_pfn->vkDestroyPipeline(h_device, m_normalShadingPipeline, nullptr);
        m_normalShadingPipeline = VK_NULL_HANDLE;
    }

    if (m_normalColoredPipeline)
    {
        h_pfn->vkDestroyPipeline(h_device, m_normalColoredPipeline, nullptr);
        m_normalColoredPipeline = VK_NULL_HANDLE;
    }

    if (m_transparencyHDRPipeline) {
        h_pfn->vkDestroyPipeline(h_device, m_transparencyHDRPipeline, nullptr);
        m_transparencyHDRPipeline = VK_NULL_HANDLE;
    }
}