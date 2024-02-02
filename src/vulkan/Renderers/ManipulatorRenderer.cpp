#include "vulkan/Renderers/ManipulatorRenderer.h"
#include "vulkan/VulkanManager.h"
#include "vulkan/MeshManager.h"
#include "models/3d/MeshBuffer.h"


// ++++++++++++++++++++++  Shaders  +++++++++++++++++++++
static uint32_t manipulators_vert_spv[] =
{
#include "manipulators.vert.spv"
};

static uint32_t manipulators_frag_spv[] =
{
#include "manipulators.frag.spv"
};

// ------------------------------------------------------

ManipulatorRenderer::ManipulatorRenderer()
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

ManipulatorRenderer::~ManipulatorRenderer()
{
    cleanup();
}

void ManipulatorRenderer::draw(const glm::mat4& tranformation, const glm::dvec3& color, Selection select, VkCommandBuffer _cmdBuffer, VkUniformOffset mvpUni, std::shared_ptr<MeshBuffer> mesh)
{
    uint32_t offsets[] = { mvpUni };
    h_pfn->vkCmdBindDescriptorSets(_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descSet, 1, offsets);
    h_pfn->vkCmdBindPipeline(_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    VkDeviceSize offset = 0;
    h_pfn->vkCmdBindVertexBuffers(_cmdBuffer, 0, 1, &(mesh->getVkBuffer()), &offset);
    h_pfn->vkCmdBindVertexBuffers(_cmdBuffer, 1, 1, &(mesh->getVkBuffer()), &(mesh->getNormalOffset()));
    h_pfn->vkCmdBindIndexBuffer(_cmdBuffer, mesh->getVkBuffer(), mesh->getIndexOffset(), VK_INDEX_TYPE_UINT32);
    setTranformationMatrix(_cmdBuffer, tranformation);
    setColor(_cmdBuffer, color);
    if (select != Selection::None)
        setManipulatorId(_cmdBuffer, uint32_t(select) + RESERVED_DATA_ID_MASK);
    for (const std::pair<VkPrimitiveTopology, std::vector<IndexedDraw>>& drawList : mesh->getIndexedDrawList())
    {
        if (drawList.first != VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            continue;
        for (const IndexedDraw& iDraw : drawList.second)
        {
            h_pfn->vkCmdDrawIndexed(_cmdBuffer, iDraw.indexCount, 1, iDraw.firstIndex, iDraw.vertexOffset, 0);
        }
    }
}

void ManipulatorRenderer::createShaders()
{
    m_vertShader.createModule(h_device, h_pfn, manipulators_vert_spv, sizeof(manipulators_vert_spv));
    m_fragShader.createModule(h_device, h_pfn, manipulators_frag_spv, sizeof(manipulators_frag_spv));
}

void ManipulatorRenderer::createPipelineResources()
{
    VkPipelineCacheCreateInfo pipelineCacheInfo;
    memset(&pipelineCacheInfo, 0, sizeof(pipelineCacheInfo));
    pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VkResult err = h_pfn->vkCreatePipelineCache(h_device, &pipelineCacheInfo, nullptr, &m_pipelineCache);
    check_vk_result(err, "Create Pipeline Cache");

    createPipelineLayout();
    createPipeline();
}

void ManipulatorRenderer::createDescriptorSetLayout()
{
    // Descriptor Set Layout Binding
    VkDescriptorSetLayoutBinding layoutBindings[] =
    {
        { // modelViewProj matrix
            0, // binding
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            1, // descriptor count
            VK_SHADER_STAGE_VERTEX_BIT,
            nullptr // used for immutable samplers
        }
    };

    // Descriptor Set Layout Create Info
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

void ManipulatorRenderer::createDescriptorSets()
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

    VkWriteDescriptorSet descWrite[1];
    memset(descWrite, 0, sizeof(descWrite));
    for (uint32_t i = 0; i < sizeof(descWrite) / sizeof(descWrite[0]); ++i)
    {
        descWrite[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite[i].dstSet = m_descSet;
        descWrite[i].dstBinding = i;
        descWrite[i].descriptorCount = 1;
        descWrite[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descWrite[i].pBufferInfo = &mat4UniInfo;
    }

    h_pfn->vkUpdateDescriptorSets(h_device, 1, descWrite, 0, nullptr);
}

void ManipulatorRenderer::createPipelineLayout()
{
    //+++ Push constant +++
    VkPushConstantRange pcr[] =
    {
        {  // tranfo 
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            64
        },
        {  // id + color + camera + screen stuff
            VK_SHADER_STAGE_FRAGMENT_BIT,
            64,
            64
        }
    };

    VkPipelineLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.pushConstantRangeCount = sizeof(pcr) / sizeof(pcr[0]);
    layoutInfo.pPushConstantRanges = pcr;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &m_descSetLayout;
    VkResult err = h_pfn->vkCreatePipelineLayout(h_device, &layoutInfo, nullptr, &m_pipelineLayout);
    check_vk_result(err, "Create Pipeline Layout [Manipulator]");
}

void ManipulatorRenderer::createPipeline()
{
    // Vertex Input & binding for Normal Points
    VkVertexInputBindingDescription vertexBindingDesc[] = {
        {
            0, // binding
            3 * sizeof(float),
            VK_VERTEX_INPUT_RATE_VERTEX
        },
        {
            1, // binding
            3 * sizeof(float),
            VK_VERTEX_INPUT_RATE_VERTEX
        },
    };

    VkVertexInputAttributeDescription vertexAttrDesc[] = {
        { // position
            0, // location
            0, // binding
            VK_FORMAT_R32G32B32_SFLOAT,
            0  // offset
        },
          { // normal
            1, // location
            1, // binding
            VK_FORMAT_R32G32B32_SFLOAT,
            0  // offset
        }
    };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pNext = nullptr;
    vertexInputInfo.flags = 0;
    vertexInputInfo.vertexBindingDescriptionCount = sizeof(vertexBindingDesc) / sizeof(vertexBindingDesc[0]);
    vertexInputInfo.pVertexBindingDescriptions = vertexBindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = sizeof(vertexAttrDesc) / sizeof(vertexAttrDesc[0]);
    vertexInputInfo.pVertexAttributeDescriptions = vertexAttrDesc;

    if (!m_vertShader.isValid() ||
        !m_fragShader.isValid())
    {
        check_vk_result(VK_ERROR_INVALID_SHADER_NV, "Missing shaders");
    }

    VkPipelineShaderStageCreateInfo vertStage = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        VK_SHADER_STAGE_VERTEX_BIT,
        m_vertShader.module(),
        "main",
        nullptr
    };

    VkPipelineShaderStageCreateInfo fragStage = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        m_fragShader.module(),
        "main",
        nullptr
    };

    VkPipelineShaderStageCreateInfo shaderStages[2] = { vertStage, fragStage };

    VkPipelineInputAssemblyStateCreateInfo ia = {};
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // The viewport and scissor will be set dynamically via vkCmdSetViewport/Scissor.
    // This way the pipeline does not need to be touched when resizing the window.
    VkPipelineViewportStateCreateInfo vp = {};
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    vp.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rs = {};
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.polygonMode = VK_POLYGON_MODE_FILL; // {FILL, LINE, POINT}
    rs.cullMode = VK_CULL_MODE_NONE; // we want the back face as well
    rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rs.lineWidth = 3.0f;

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
            VK_TRUE,              // blendEnable
            VK_BLEND_FACTOR_ONE,                 // srcColorBlendFactor
            VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, // dstColorBlendFactor
            VK_BLEND_OP_ADD,                     // colorBlendOp
            VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA, // srcAlphaBlendFactor  // NOT USE ? can be ONE ?
            VK_BLEND_FACTOR_ONE,                 // dstAlphaBlendFactor
            VK_BLEND_OP_ADD,                     // alphaBlendOp
            0xF                                  // colorWriteMask
        },
        {
            VK_FALSE,
            VK_BLEND_FACTOR_ONE,
            VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            VK_BLEND_OP_ADD,
            VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
            VK_BLEND_FACTOR_ONE,
            VK_BLEND_OP_ADD,
            0xF
        }
    };

    VkPipelineColorBlendStateCreateInfo cb = {};
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    // TODO - change the attachment count if we want to write the "marker index" output
    cb.attachmentCount = attCount;
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
        2,                           // stageCount
        shaderStages,                // pStages
        &vertexInputInfo,            // pVertexInputState
        &ia,                         // pInputAssembleState
        nullptr,                     // pTesselationState
        &vp,                         // pViewportState
        &rs,                         // pRasterizationState
        &ms,                         // pMultisampleState
        &ds,                         // pDepthStencilState
        &cb,                         // pColorBlendState
        &dyn,                        // pDynamicState
        m_pipelineLayout,            // layout
        h_renderPass,                // renderPass
        5,                           // subpass
        VK_NULL_HANDLE,              // basePipelineHandle
        0                            // basePipelineIndex
    };

    VkResult err = h_pfn->vkCreateGraphicsPipelines(h_device, m_pipelineCache, 1, &pipelineInfo, nullptr, &m_pipeline);
    check_vk_result(err, "Create Graphic Pipeline (Object)");
}

void ManipulatorRenderer::destroyPipeline(VkPipeline& pipeline)
{
    if (pipeline) {
        h_pfn->vkDestroyPipeline(h_device, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }
}

void ManipulatorRenderer::cleanup()
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
    m_vertShader.reset();
    m_fragShader.reset();

    //*** Pipelines ***
    destroyPipeline(m_pipeline);
}

void ManipulatorRenderer::setTranformationMatrix(VkCommandBuffer _cmdBuffer, const glm::mat4& transfo)
{
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, 64, &transfo[0]);
}

void ManipulatorRenderer::setColor(VkCommandBuffer _cmdBuffer, const glm::vec3& color)
{
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 80, 12, &color[0]);
}

void  ManipulatorRenderer::setManipulatorId(VkCommandBuffer _cmdBuffer, uint32_t id)
{
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 64, 4, &id);
}
