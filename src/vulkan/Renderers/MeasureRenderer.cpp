#include "MeasureRenderer.h"
#include "vulkan/VulkanManager.h"

// ++++++++++++++++++++++  Shaders  +++++++++++++++++++++

static const uint32_t measure_line_vert_spv[] =
{
#include "measure_line.vert.spv"
};

static const uint32_t measure_line_geom_spv[] =
{
#include "measure_line.geom.spv"
};

static const uint32_t measure_line_frag_spv[] =
{
#include "measure_line.frag.spv"
};

static const uint32_t measure_point_vert_spv[] =
{
#include "measure_point.vert.spv"
};
static const uint32_t measure_point_geom_spv[] =
{
#include "measure_point.geom.spv"
};

static const uint32_t measure_point_frag_spv[] =
{
#include "measure_point.frag.spv"
};

// ------------------------------------------------------

MeasureRenderer::MeasureRenderer()
{
    VulkanManager& vkm = VulkanManager::getInstance();

    h_device = vkm.getDevice();
    h_pfn = vkm.getDeviceFunctions();
    h_descPool = vkm.getDescriptorPool();
    h_renderPass = vkm.getObjectRenderPass();

    createShaders();
    createDescriptorSetLayout();
    createDescriptorSets();
    createPipelineResources();
}

void MeasureRenderer::drawMeasures(VkCommandBuffer _cmdBuffer, uint32_t vpUniOffset, VkBuffer segmentBuffer, uint32_t segmentCount)
{
    if (segmentCount == 0)
        return;

    // Common commands
    h_pfn->vkCmdBindDescriptorSets(_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descSet, 1, &vpUniOffset);

    VkDeviceSize offset = 0;
    h_pfn->vkCmdBindVertexBuffers(_cmdBuffer, 0, 1, &segmentBuffer, &offset);

    // Draw Lines
    h_pfn->vkCmdBindPipeline(_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_line);
    h_pfn->vkCmdDraw(_cmdBuffer, segmentCount, 1, 0, 0);

    // Draw Points
    h_pfn->vkCmdBindPipeline(_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_point);
    h_pfn->vkCmdDraw(_cmdBuffer, segmentCount, 1, 0, 0);
}

void MeasureRenderer::createShaders()
{
    m_vertShader_line.createModule(h_device, h_pfn, measure_line_vert_spv, sizeof(measure_line_vert_spv));
    m_geomShader_line.createModule(h_device, h_pfn, measure_line_geom_spv, sizeof(measure_line_geom_spv));
    m_fragShader_line.createModule(h_device, h_pfn, measure_line_frag_spv, sizeof(measure_line_frag_spv));
    m_vertShader_point.createModule(h_device, h_pfn, measure_point_vert_spv, sizeof(measure_point_vert_spv));
    m_geomShader_point.createModule(h_device, h_pfn, measure_point_geom_spv, sizeof(measure_point_geom_spv));
    m_fragShader_point.createModule(h_device, h_pfn, measure_point_frag_spv, sizeof(measure_point_frag_spv));
}

void MeasureRenderer::createPipelineResources()
{
    VkPipelineCacheCreateInfo pipelineCacheInfo;
    memset(&pipelineCacheInfo, 0, sizeof(pipelineCacheInfo));
    pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VkResult err = h_pfn->vkCreatePipelineCache(h_device, &pipelineCacheInfo, nullptr, &m_pipelineCache);
    check_vk_result(err, "Create Pipeline Cache");

    createPipelineLayout();
    createPipelines();
}

void MeasureRenderer::createDescriptorSetLayout()
{
    // Descriptor Set Layout Binding
    VkDescriptorSetLayoutBinding layoutBindings[] =
    {
        { // view * proj matrix
            0, // binding
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            1, // descriptor count
            VK_SHADER_STAGE_VERTEX_BIT,
            nullptr // used for immutable samplers
        }
    };

    // Descriptor Set Layout Create Info
    VkDescriptorSetLayoutCreateInfo descLayoutInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        nullptr,
        0, // bits defined by extensions
        sizeof(layoutBindings) / sizeof(layoutBindings[0]), // bindingCount
        layoutBindings
    };
    VkResult err = h_pfn->vkCreateDescriptorSetLayout(h_device, &descLayoutInfo, nullptr, &m_descSetLayout);
    check_vk_result(err, "Create Descriptor Set Layout");
}

void MeasureRenderer::createDescriptorSets()
{
    // Descriptor Set Allocation
    VkDescriptorSetAllocateInfo descSetAllocInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        nullptr,
        h_descPool,
        1,
        &m_descSetLayout
    };
    VkResult err = h_pfn->vkAllocateDescriptorSets(h_device, &descSetAllocInfo, &m_descSet);
    check_vk_result(err, "Allocate Descriptor Sets");

    VulkanManager& vkManager = VulkanManager::getInstance();

    VkBuffer uniBuf = vkManager.getUniformBuffer();

    // Write descriptors for the uniform buffers in the vertex and fragment shaders.
    VkDescriptorBufferInfo mat4UniInfo = {
        uniBuf, // buffer
        0, // offset
        vkManager.getUniformSizeAligned(3 * 64) // range
    };

    VkWriteDescriptorSet descWrite = {};
    descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descWrite.dstSet = m_descSet;
    descWrite.dstBinding = 0;
    descWrite.descriptorCount = 1;
    descWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descWrite.pBufferInfo = &mat4UniInfo;

    h_pfn->vkUpdateDescriptorSets(h_device, 1, &descWrite, 0, nullptr);
}

void MeasureRenderer::createPipelineLayout()
{
    //+++ Push constant +++
    VkPushConstantRange pcr[] =
    {
        {  // point size, pixelHeight
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            8
        },
        {  // show mask
            VK_SHADER_STAGE_GEOMETRY_BIT,
            8,
            8
        },
        {  // stripe count
            VK_SHADER_STAGE_FRAGMENT_BIT,
            16,
            32
        }
    };

    VkPipelineLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.pushConstantRangeCount = sizeof(pcr) / sizeof(pcr[0]);
    layoutInfo.pPushConstantRanges = pcr;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &m_descSetLayout;
    VkResult err = h_pfn->vkCreatePipelineLayout(h_device, &layoutInfo, nullptr, &m_pipelineLayout);
    check_vk_result(err, "Create Pipeline Layout [Object]");
}


void MeasureRenderer::createPipelines()
{
    // ********************* Vertex Input *********************

    // Vertex Input & binding for lines
    VkVertexInputBindingDescription vib_line[] = {
        {
            0, // binding
            6 * sizeof(float) + 3 * sizeof(uint32_t),
            VK_VERTEX_INPUT_RATE_VERTEX
        }
    };

    VkVertexInputAttributeDescription via_line[] = {
        { // position_0
            0, // location
            0, // binding
            VK_FORMAT_R32G32B32_SFLOAT,
            0  // offset
        },
        { // position_1
            1,
            0,
            VK_FORMAT_R32G32B32_SFLOAT,
            3 * sizeof(float)
        },
        { // color
            2,
            0,
            VK_FORMAT_R8G8B8A8_UINT,
            6 * sizeof(float)
        },
        { // markerID
            3,
            0,
            VK_FORMAT_R32_UINT,
            7 * sizeof(float)
        },
        { // showMask
            4,
            0,
            VK_FORMAT_R32_UINT,
            8 * sizeof(float)
        }
    };

    VkPipelineVertexInputStateCreateInfo vertexInputLine;
    vertexInputLine.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputLine.pNext = nullptr;
    vertexInputLine.flags = 0;
    vertexInputLine.vertexBindingDescriptionCount = sizeof(vib_line) / sizeof(vib_line[0]);
    vertexInputLine.pVertexBindingDescriptions = vib_line;
    vertexInputLine.vertexAttributeDescriptionCount = sizeof(via_line) / sizeof(via_line[0]);
    vertexInputLine.pVertexAttributeDescriptions = via_line;

    // Vertex Input & binding for points
    VkVertexInputBindingDescription vib_point[] = {
        {
            0, // binding
            6 * sizeof(float) + 3 * sizeof(uint32_t), // stride
            VK_VERTEX_INPUT_RATE_VERTEX
        }
    };

    VkVertexInputAttributeDescription via_point[] = {
        { // position_0
            0, // location
            0, // binding
            VK_FORMAT_R32G32B32_SFLOAT,
            0  // offset
        },
        { // position_1
            1,
            0,
            VK_FORMAT_R32G32B32_SFLOAT,
            3 * sizeof(float)
        },
        { // color
            2,
            0,
            VK_FORMAT_R8G8B8A8_UINT,
            6 * sizeof(float)
        },
        { // markerID
            3,
            0,
            VK_FORMAT_R32_UINT,
            7 * sizeof(float)
        },
        { // showMask
            4,
            0,
            VK_FORMAT_R32_UINT,
            8 * sizeof(float)
        }
    };

    VkPipelineVertexInputStateCreateInfo vertexInputPoint;
    vertexInputPoint.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputPoint.pNext = nullptr;
    vertexInputPoint.flags = 0;
    vertexInputPoint.vertexBindingDescriptionCount = sizeof(vib_point) / sizeof(vib_point[0]);
    vertexInputPoint.pVertexBindingDescriptions = vib_point;
    vertexInputPoint.vertexAttributeDescriptionCount = sizeof(via_point) / sizeof(via_point[0]);
    vertexInputPoint.pVertexAttributeDescriptions = via_point;


    // **************** Shaders ***********************

    if (!m_vertShader_line.isValid() ||
        !m_geomShader_line.isValid() ||
        !m_fragShader_line.isValid())
    {
        check_vk_result(VK_ERROR_INVALID_SHADER_NV, "Missing shaders");
    }

    VkPipelineShaderStageCreateInfo shaderStagesLine[3] = {
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_VERTEX_BIT,
            m_vertShader_line.module(),
            "main",
            nullptr
        },
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_GEOMETRY_BIT,
            m_geomShader_line.module(),
            "main",
            nullptr
        },
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            m_fragShader_line.module(),
            "main",
            nullptr
        }
    };

    if (!m_vertShader_point.isValid() ||
        !m_fragShader_point.isValid())
    {
        check_vk_result(VK_ERROR_INVALID_SHADER_NV, "Missing shaders");
    }

    VkPipelineShaderStageCreateInfo shaderStagesPoint[3] = {
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_VERTEX_BIT,
            m_vertShader_point.module(),
            "main",
            nullptr
        },
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_GEOMETRY_BIT,
            m_geomShader_point.module(),
            "main",
            nullptr
        },
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            m_fragShader_point.module(),
            "main",
            nullptr
        }
    };

    VkPipelineInputAssemblyStateCreateInfo ia_line = {};
    ia_line.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia_line.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

    VkPipelineInputAssemblyStateCreateInfo ia_point = {};
    ia_point.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia_point.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

    VkPipelineRasterizationStateCreateInfo rs_line = {};
    rs_line.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs_line.polygonMode = VK_POLYGON_MODE_LINE; // {FILL, LINE, POINT}
    rs_line.cullMode = VK_CULL_MODE_NONE; // we want the back face as well
    rs_line.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rs_line.lineWidth = 3.0f;

    VkPipelineRasterizationStateCreateInfo rs_point = {};
    rs_point.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs_point.polygonMode = VK_POLYGON_MODE_POINT; // {FILL, LINE, POINT}
    rs_point.cullMode = VK_CULL_MODE_NONE; // we want the back face as well
    rs_point.frontFace = VK_FRONT_FACE_CLOCKWISE;

    // The viewport and scissor will be set dynamically via vkCmdSetViewport/Scissor.
    // This way the pipeline does not need to be touched when resizing the window.
    VkPipelineViewportStateCreateInfo vp = {};
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    vp.scissorCount = 1;

    VkPipelineMultisampleStateCreateInfo ms = {};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.sampleShadingEnable = VK_FALSE;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo ds = {};
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable = VK_TRUE;
    ds.depthWriteEnable = VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    const size_t attCount = 2;
    VkPipelineColorBlendAttachmentState cbAttach[attCount] = {
        {
            VK_FALSE,              // blendEnable
            VK_BLEND_FACTOR_ONE,   // srcColorBlendFactor
            VK_BLEND_FACTOR_ZERO,  // dstColorBlendFactor
            VK_BLEND_OP_ADD,       // colorBlendOp
            VK_BLEND_FACTOR_ONE,   // srcAlphaBlendFactor
            VK_BLEND_FACTOR_ZERO,  // dstAlphaBlendFactor
            VK_BLEND_OP_ADD,       // alphaBlendOp
            0xF                    // colorWriteMask
        },
        {
            VK_FALSE,
            VK_BLEND_FACTOR_ONE,
            VK_BLEND_FACTOR_ZERO,
            VK_BLEND_OP_ADD,
            VK_BLEND_FACTOR_ONE,
            VK_BLEND_FACTOR_ZERO,
            VK_BLEND_OP_ADD,
            0xF
        }
    };

    VkPipelineColorBlendStateCreateInfo cb = {};
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.attachmentCount = 2;
    cb.pAttachments = cbAttach;

    VkDynamicState dynEnable[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dyn = {};
    dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn.dynamicStateCount = sizeof(dynEnable) / sizeof(VkDynamicState);
    dyn.pDynamicStates = dynEnable;


    // Graphics pipeline creation
    VkGraphicsPipelineCreateInfo pipelineInfo = {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        nullptr,                     // pNext
        0,                           // flags
        3,                           // stageCount
        shaderStagesLine,            // pStages
        &vertexInputLine,            // pVertexInputState
        &ia_line,                    // pInputAssembleState
        nullptr,                     // pTesselationState
        &vp,                         // pViewportState
        &rs_line,                    // pRasterizationState
        &ms,                         // pMultisampleState
        &ds,                         // pDepthStencilState
        &cb,                         // pColorBlendState
        &dyn,                        // pDynamicState
        m_pipelineLayout,            // layout
        h_renderPass,                // renderPass
        0,                           // subpass
        VK_NULL_HANDLE,              // basePipelineHandle
        0                            // basePipelineIndex
    };

    VkResult err = h_pfn->vkCreateGraphicsPipelines(h_device, m_pipelineCache, 1, &pipelineInfo, nullptr, &m_pipeline_line);
    check_vk_result(err, "Create Graphic Pipeline (Object)");

    pipelineInfo.stageCount = 3;
    pipelineInfo.pStages = shaderStagesPoint;
    pipelineInfo.pVertexInputState = &vertexInputPoint;
    pipelineInfo.pInputAssemblyState = &ia_point;
    pipelineInfo.pRasterizationState = &rs_point;

    err = h_pfn->vkCreateGraphicsPipelines(h_device, m_pipelineCache, 1, &pipelineInfo, nullptr, &m_pipeline_point);
    check_vk_result(err, "Create Graphic Pipeline (Object)");
}

void MeasureRenderer::destroyPipeline(VkPipeline& pipeline)
{
    if (pipeline) {
        h_pfn->vkDestroyPipeline(h_device, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }
}

void MeasureRenderer::cleanup()
{
    //*** Common Resources ***
    if (m_pipelineCache) {
        h_pfn->vkDestroyPipelineCache(h_device, m_pipelineCache, nullptr);
        m_pipelineCache = VK_NULL_HANDLE;
    }

    if (m_pipelineLayout) {
        h_pfn->vkDestroyPipelineLayout(h_device, m_pipelineLayout, nullptr);
        m_pipelineLayout = VK_NULL_HANDLE;
    }

    if (m_descSetLayout) {
        h_pfn->vkDestroyDescriptorSetLayout(h_device, m_descSetLayout, nullptr);
        m_descSetLayout = VK_NULL_HANDLE;
    }

    //*** Shaders ***
    m_vertShader_line.reset();
    m_fragShader_line.reset();
    m_vertShader_point.reset();
    m_fragShader_point.reset();

    //*** Pipelines ***
    destroyPipeline(m_pipeline_line);
    destroyPipeline(m_pipeline_point);
}

void MeasureRenderer::setPointSize(VkCommandBuffer _cmdBuffer, float size)
{
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, 4, &size);
}

void MeasureRenderer::setStripCount(VkCommandBuffer _cmdBuffer, float _value)
{
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 28, 4, &_value);
}

void MeasureRenderer::setLightRay(VkCommandBuffer _cmdBuffer, glm::vec3 _direction)
{
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 16, 12, &_direction);
}

void MeasureRenderer::setScreenParameters(VkCommandBuffer _cmdBuffer, float pixelHAt1m, float nearZ, float farZ)
{
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 4, 4, &pixelHAt1m);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 36, 4, &nearZ);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 40, 4, &farZ);
}

void MeasureRenderer::setMeasureShowMask(VkCommandBuffer _cmdBuffer, uint32_t mask)
{
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, VK_SHADER_STAGE_GEOMETRY_BIT, 8, 4, &mask);
}

void MeasureRenderer::setObjectFlags(VkCommandBuffer _cmdBuffer, bool isHovered, bool isSelected, bool isActivated, bool isExterior)
{
    uint32_t flags = 0;
    flags |= isHovered ? 0x01 : 0;
    flags |= isSelected ? 0x02 : 0;
    flags |= isActivated ? 0x04 : 0;
    flags |= isExterior ? 0x08 : 0;
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 44, 4, &flags);

}