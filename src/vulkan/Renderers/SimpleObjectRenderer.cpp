#include "SimpleObjectRenderer.h"
#include "utils/Utils.h"

#include "vulkan/VulkanManager.h"
#include "vulkan/VulkanHelperFunctions.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

#define SELECTION_COLOR glm::vec3(1.0f, 1.0f, 0.0f)

// ++++++++++++++++++++++  Shaders  +++++++++++++++++++++
static uint32_t mesh_edge_vert_spv[] =
{
#include "mesh_edge.vert.spv"
};

static uint32_t mesh_edge_frag_spv[] =
{
#include "mesh_edge.frag.spv"
};

static uint32_t mesh_face_vert_spv[] =
{
#include "mesh_face.vert.spv"
};

static uint32_t mesh_face_geom_spv[] =
{
#include "mesh_face.geom.spv"
};

static uint32_t mesh_face_normals_vert_spv[] =
{
#include "mesh_face_normals.vert.spv"
};

static uint32_t mesh_face_frag_spv[] =
{
#include "mesh_face.frag.spv"
};

static uint32_t blend_vert_spv[] =
{
#include "blend.vert.spv"
};

static uint32_t blend_frag_spv[] =
{
#include "blend.frag.spv"
};

constexpr uint32_t g_subpass_opaque = 0;
constexpr uint32_t g_subpass_transparent = 1;

std::array<TlTopologyFlags, 6> g_vkTopologyToTl = {
    TL_TOPOLOGY_ALL,
    TL_TOPOLOGY_LINE,
    TL_TOPOLOGY_LINE,
    TL_TOPOLOGY_TRIANGLE,
    TL_TOPOLOGY_TRIANGLE,
    TL_TOPOLOGY_TRIANGLE,
};

// ------------------------------------------------------

SimpleObjectRenderer::SimpleObjectRenderer()
{
    VulkanManager& vkm = VulkanManager::getInstance();

    h_device = vkm.getDevice();
    h_pfn = vkm.getDeviceFunctions();
    h_descPool = vkm.getDescriptorPool();
    h_renderPass = vkm.getObjectRenderPass();

    initRenderer();
}

SimpleObjectRenderer::~SimpleObjectRenderer()
{
    cleanup();
    VulkanManager::getInstance().freeAllocation(m_quadTriangleBuffer);
}

void SimpleObjectRenderer::drawMesh(VkCommandBuffer _cmdBuffer, VkUniformOffset mvpUni, const std::shared_ptr<MeshBuffer>& mesh, uint32_t subpass, TlTopologyFlags allowedTopologies)
{
    bool hasNormals = mesh->hasNormals();
    h_pfn->vkCmdBindVertexBuffers(_cmdBuffer, 0, 1, &mesh->getVkBuffer(), &mesh->getVertexOffset());
    if (hasNormals)
        h_pfn->vkCmdBindVertexBuffers(_cmdBuffer, 1, 1, &mesh->getVkBuffer(), &mesh->getNormalOffset());

    // Standard Draw
    for (const std::pair<VkPrimitiveTopology, std::vector<StandardDraw>>& topoList : mesh->getDrawList())
    {
        if (!bindPipelineResources(_cmdBuffer, topoList.first, subpass, mvpUni, hasNormals, allowedTopologies))
            continue;
        for (const StandardDraw& dr : topoList.second)
        {
            h_pfn->vkCmdDraw(_cmdBuffer, dr.vertexCount, 1, dr.firstVertex, 0);
        }
    }

    h_pfn->vkCmdBindIndexBuffer(_cmdBuffer, mesh->getVkBuffer(), mesh->getIndexOffset(), VK_INDEX_TYPE_UINT32);
    for (const std::pair<VkPrimitiveTopology, std::vector<IndexedDraw>>& topoList : mesh->getIndexedDrawList())
    {
        if (!bindPipelineResources(_cmdBuffer, topoList.first, subpass, mvpUni, hasNormals, allowedTopologies))
            continue;
        for (const IndexedDraw& idr : topoList.second)
        {
            h_pfn->vkCmdDrawIndexed(_cmdBuffer, idr.indexCount, 1, idr.firstIndex, idr.vertexOffset, 0);
        }
    }
}

void SimpleObjectRenderer::blendTransparentImage(VkCommandBuffer _cmdBuffer, VkDescriptorSet _inputDescSet)
{
    h_pfn->vkCmdBindPipeline(_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineBlend);

    h_pfn->vkCmdBindDescriptorSets(_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineBlendLayout, 0, 1, &_inputDescSet, 0, nullptr);
    VkDeviceSize offset = 0;
    h_pfn->vkCmdBindVertexBuffers(_cmdBuffer, 0, 1, &m_quadTriangleBuffer.buffer, &offset);

    // 2 triangle strip
    h_pfn->vkCmdDraw(_cmdBuffer, 4, 1, 0, 0);
}

void SimpleObjectRenderer::initRenderer()
{
    createDataBuffer();
    createShaders();
    createDescriptorSetLayout();
    createDescriptorSets();
    createPipelineResources();
    // Free some memory
    cleanShaders();
}

void SimpleObjectRenderer::createDataBuffer()
{
    const float vertices[] = {
        -1.f, -1.f,
         1.f, -1.f,
        -1.f,  1.f,
         1.f,  1.f
    };

    VulkanManager& vkm = VulkanManager::getInstance();
    vkm.allocSimpleBuffer(sizeof(vertices), m_quadTriangleBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceSize offset = 0;
    vkm.loadInSimpleBuffer(m_quadTriangleBuffer, sizeof(vertices), vertices, offset, 4);
}

void SimpleObjectRenderer::createShaders()
{
    m_vertFacesShader.createModule(h_device, h_pfn, mesh_face_vert_spv, sizeof(mesh_face_vert_spv));
    m_geomFacesShader.createModule(h_device, h_pfn, mesh_face_geom_spv, sizeof(mesh_face_geom_spv));
    m_vertFacesNormalsShader.createModule(h_device, h_pfn, mesh_face_normals_vert_spv, sizeof(mesh_face_normals_vert_spv));
    m_fragFacesShader.createModule(h_device, h_pfn, mesh_face_frag_spv, sizeof(mesh_face_frag_spv));
    m_vertEdgesShader.createModule(h_device, h_pfn, mesh_edge_vert_spv, sizeof(mesh_edge_vert_spv));
    m_fragEdgesShader.createModule(h_device, h_pfn, mesh_edge_frag_spv, sizeof(mesh_edge_frag_spv));
    m_vertBlendShader.createModule(h_device, h_pfn, blend_vert_spv, sizeof(blend_vert_spv));
    m_fragBlendShader.createModule(h_device, h_pfn, blend_frag_spv, sizeof(blend_frag_spv));
}

void SimpleObjectRenderer::createPipelineResources()
{
    VkPipelineCacheCreateInfo pipelineCacheInfo;
    memset(&pipelineCacheInfo, 0, sizeof(pipelineCacheInfo));
    pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VkResult err = h_pfn->vkCreatePipelineCache(h_device, &pipelineCacheInfo, nullptr, &m_pipelineCache);
    check_vk_result(err, "Create Pipeline Cache");

    createPipelineLayout();
    for (int t = 0; t < 2; ++t)
    {
        bool transparent = (t == 0);
        for (int n = 0; n < 2; ++n)
        {
            bool normals = (n == 0);
            createPipelineFace(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, transparent, normals);
            createPipelineFace(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, transparent, normals);
            createPipelineFace(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, transparent, normals);
        }
    }
    createPipelineEdge(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    createPipelineEdge(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);
    createPipelineBlend();
}

void SimpleObjectRenderer::createDescriptorSetLayout()
{
    VkResult err = VK_SUCCESS;
    VkDescriptorSetLayoutCreateInfo descLayoutInfo = {};
    descLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

    // Face & Edge pipelines
    VkDescriptorSetLayoutBinding layoutBindings[] =
    {
        { // modelViewProj matrix
            0, // binding
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            1, // descriptor count
            VK_SHADER_STAGE_VERTEX_BIT,
            nullptr // used for immutable samplers
        },
        { // modelViewProj matrix
            0, // binding
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            1, // descriptor count
            VK_SHADER_STAGE_GEOMETRY_BIT,
            nullptr // used for immutable samplers
        }
    };

    descLayoutInfo.bindingCount = 1;
    descLayoutInfo.pBindings = &layoutBindings[0];
    err = h_pfn->vkCreateDescriptorSetLayout(h_device, &descLayoutInfo, nullptr, &m_descSetLayout_vf);
    check_vk_result(err, "Create Descriptor Set Layout");

    descLayoutInfo.bindingCount = 1;
    descLayoutInfo.pBindings = &layoutBindings[1];
    err = h_pfn->vkCreateDescriptorSetLayout(h_device, &descLayoutInfo, nullptr, &m_descSetLayout_vgf);
    check_vk_result(err, "Create Descriptor Set Layout");
}

void SimpleObjectRenderer::createDescriptorSets()
{
    VkResult err = VK_SUCCESS;
    // Descriptor Set Allocation
    VkDescriptorSetAllocateInfo descSetAllocInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        nullptr,
        h_descPool,
        1,
        &m_descSetLayout_vgf
    };
    err = h_pfn->vkAllocateDescriptorSets(h_device, &descSetAllocInfo, &m_descSet_vgf);
    check_vk_result(err, "Allocate Descriptor Sets");

    descSetAllocInfo.pSetLayouts = &m_descSetLayout_vf;
    err = h_pfn->vkAllocateDescriptorSets(h_device, &descSetAllocInfo, &m_descSet_vf);
    check_vk_result(err, "Allocate Descriptor Sets");

    VulkanManager& vkManager = VulkanManager::getInstance();

    VkBuffer uniBuf = vkManager.getUniformBuffer();

    // Write descriptors for the uniform buffers in the vertex and fragment shaders.
    VkDescriptorBufferInfo mat4UniInfo[] = {
        {
            uniBuf, // buffer
            0, // offset
            vkManager.getUniformSizeAligned(3 * 64) // range
        }
    };

    VkWriteDescriptorSet descWrite[2];
    memset(descWrite, 0, sizeof(descWrite));
    descWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descWrite[0].dstSet = m_descSet_vf;
    descWrite[0].dstBinding = 0;
    descWrite[0].descriptorCount = 1;
    descWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descWrite[0].pBufferInfo = mat4UniInfo;

    descWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descWrite[1].dstSet = m_descSet_vgf;
    descWrite[1].dstBinding = 0;
    descWrite[1].descriptorCount = 1;
    descWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descWrite[1].pBufferInfo = mat4UniInfo;

    h_pfn->vkUpdateDescriptorSets(h_device, 2, descWrite, 0, nullptr);
}

void SimpleObjectRenderer::createPipelineLayout()
{
    VkResult err = VK_SUCCESS;
    //+++ Push constant +++
    VkPushConstantRange pcr[] =
    {
        {  // tranfo 
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            64
        },
        {  // id , flags, alpha, color
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
    layoutInfo.pSetLayouts = &m_descSetLayout_vf;
    err = h_pfn->vkCreatePipelineLayout(h_device, &layoutInfo, nullptr, &m_pipelineMeshLayout_vf);
    check_vk_result(err, "Create Pipeline Layout [Object]");

    layoutInfo.pSetLayouts = &m_descSetLayout_vgf;
    err = h_pfn->vkCreatePipelineLayout(h_device, &layoutInfo, nullptr, &m_pipelineMeshLayout_vgf);
    check_vk_result(err, "Create Pipeline Layout [Object]");

    VkPushConstantRange pcr_blend =
    {
        VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        64
    };

    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pcr_blend;
    VkDescriptorSetLayout setLayouts[] = { VulkanManager::getDSLayout_inputTransparentLayer() };
    layoutInfo.pSetLayouts = setLayouts;
    err = h_pfn->vkCreatePipelineLayout(h_device, &layoutInfo, nullptr, &m_pipelineBlendLayout);
    check_vk_result(err, "Create Pipeline Layout [Object]");
}

void SimpleObjectRenderer::createPipelineFace(VkPrimitiveTopology _topology, bool _transparent, bool _hasInputNormals)
{
    uint32_t subpass = _transparent ? g_subpass_transparent : g_subpass_opaque;

    // Vertex Input & binding for Normal Points
    std::vector<VkVertexInputBindingDescription> vertexBindingDesc;
    vertexBindingDesc.push_back({
        0, // binding
        3 * sizeof(float),
        VK_VERTEX_INPUT_RATE_VERTEX
        });

    std::vector<VkVertexInputAttributeDescription> vertexAttrDesc;
    vertexAttrDesc.push_back({
        0, // location
        0, // binding
        VK_FORMAT_R32G32B32_SFLOAT,
        0
        });

    if (_hasInputNormals)
    {
        vertexBindingDesc.push_back({
            1, // binding
            3 * sizeof(float),
            VK_VERTEX_INPUT_RATE_VERTEX
            });

        vertexAttrDesc.push_back({
            1,
            1,
            VK_FORMAT_R32G32B32_SFLOAT,
            0
            });
    }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pNext = nullptr;
    vertexInputInfo.flags = 0;
    vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)vertexBindingDesc.size();
    vertexInputInfo.pVertexBindingDescriptions = vertexBindingDesc.data();
    vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)vertexAttrDesc.size();
    vertexInputInfo.pVertexAttributeDescriptions = vertexAttrDesc.data();

    const Shader& vertShader = _hasInputNormals ? m_vertFacesNormalsShader : m_vertFacesShader ;
    if (!vertShader.isValid() || !m_fragFacesShader.isValid() || !m_geomFacesShader.isValid())
    {
        check_vk_result(VK_ERROR_INVALID_SHADER_NV, "Missing shaders");
    }

    VkPipelineShaderStageCreateInfo vertStage = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        VK_SHADER_STAGE_VERTEX_BIT,
        vertShader.module(),
        "main",
        nullptr
    };

    VkPipelineShaderStageCreateInfo geomStage = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        VK_SHADER_STAGE_GEOMETRY_BIT,
        m_geomFacesShader.module(),
        "main",
        nullptr
    };

    VkPipelineShaderStageCreateInfo fragStage = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        m_fragFacesShader.module(),
        "main",
        nullptr
    };

    VkPipelineShaderStageCreateInfo shaderStages_vf[2] = { vertStage, fragStage };
    VkPipelineShaderStageCreateInfo shaderStages_vgf[3] = { vertStage, geomStage, fragStage };

    VkPipelineInputAssemblyStateCreateInfo ia = {};
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = _topology;

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
    rs.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo ms = {};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.sampleShadingEnable = VK_FALSE;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo ds = {};
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable = VK_TRUE; // we want to show only fragments in front of opaque objects
    ds.depthWriteEnable = _transparent ? VK_FALSE : VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    const size_t attCount = 2;
    VkPipelineColorBlendAttachmentState cb_attach[attCount];
    memset(cb_attach, 0, sizeof(cb_attach));

    if (_transparent)
    {
        cb_attach[0] = {
            VK_TRUE,
            VK_BLEND_FACTOR_ONE,
            VK_BLEND_FACTOR_ONE,
            VK_BLEND_OP_ADD,
            VK_BLEND_FACTOR_ONE,
            VK_BLEND_FACTOR_ONE,
            VK_BLEND_OP_ADD,
            0xF
        };
    }
    else
    {
        cb_attach[0] = {
            VK_FALSE,              // blendEnable
            VK_BLEND_FACTOR_ONE,   // srcColorBlendFactor
            VK_BLEND_FACTOR_ZERO,  // dstColorBlendFactor
            VK_BLEND_OP_ADD,       // colorBlendOp
            VK_BLEND_FACTOR_ONE,   // srcAlphaBlendFactor
            VK_BLEND_FACTOR_ZERO,  // dstAlphaBlendFactor
            VK_BLEND_OP_ADD,       // alphaBlendOp
            0xF                    // colorWriteMask
        };
    }

    cb_attach[1].colorWriteMask = 0xF;

    VkPipelineColorBlendStateCreateInfo cb = {};
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.attachmentCount = attCount;
    cb.pAttachments = cb_attach;

    VkDynamicState dynEnable[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dyn = {};
    dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn.dynamicStateCount = sizeof(dynEnable) / sizeof(VkDynamicState);
    dyn.pDynamicStates = dynEnable;


    VkPipelineLayout& pipelineLayout = _hasInputNormals ? m_pipelineMeshLayout_vf : m_pipelineMeshLayout_vgf;

    // Graphics pipeline creation
    VkGraphicsPipelineCreateInfo pipelineInfo = {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        nullptr,               // pNext
        0,                     // flags
        _hasInputNormals ? 2u : 3u, // stageCount
        _hasInputNormals ? shaderStages_vf : shaderStages_vgf, // pStages
        &vertexInputInfo,      // pVertexInputState
        &ia,                   // pInputAssembleState
        nullptr,               // pTesselationState
        &vp,                   // pViewportState
        &rs,                   // pRasterizationState
        &ms,                   // pMultisampleState
        &ds,                   // pDepthStencilState
        &cb,                   // pColorBlendState
        &dyn,                  // pDynamicState
        pipelineLayout,        // layout
        h_renderPass,          // renderPass
        subpass,               // subpass
        VK_NULL_HANDLE,        // basePipelineHandle
        0                      // basePipelineIndex
    };

    VkPipeline pipeline;
    VkResult err = h_pfn->vkCreateGraphicsPipelines(h_device, m_pipelineCache, 1, &pipelineInfo, nullptr, &pipeline);
    check_vk_result(err, "Create Graphic Pipeline (Object opaque faces)");

    m_pipelines.insert({ _topology, subpass, _hasInputNormals, pipeline });
}

void SimpleObjectRenderer::createPipelineEdge(VkPrimitiveTopology _topology)
{
    // Vertex Input & binding for Normal Points
    VkVertexInputBindingDescription vertexBindingDesc[] = {
        {
            0, // binding
            3 * sizeof(float),
            VK_VERTEX_INPUT_RATE_VERTEX
        }
    };

    VkVertexInputAttributeDescription vertexAttrDesc[] = {
        { // position
            0, // location
            0, // binding
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

    if (!m_vertEdgesShader.isValid() ||
        !m_fragEdgesShader.isValid())
    {
        check_vk_result(VK_ERROR_INVALID_SHADER_NV, "Missing shaders");
    }

    VkPipelineShaderStageCreateInfo shaderStages[2] = {
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_VERTEX_BIT,
            m_vertEdgesShader.module(),
            "main",
            nullptr
        },
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            m_fragEdgesShader.module(),
            "main",
            nullptr
        }
    };

    VkPipelineInputAssemblyStateCreateInfo ia = {};
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = _topology;

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
    rs.lineWidth = 2.0f;

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
        m_pipelineMeshLayout_vf,     // layout
        h_renderPass,                // renderPass
        g_subpass_opaque,            // subpass
        VK_NULL_HANDLE,              // basePipelineHandle
        0                            // basePipelineIndex
    };

    VkPipeline pipeline;
    VkResult err = h_pfn->vkCreateGraphicsPipelines(h_device, m_pipelineCache, 1, &pipelineInfo, nullptr, &pipeline);
    check_vk_result(err, "Create Graphic Pipeline (Object)");

    m_pipelines.insert({ _topology, g_subpass_opaque, false, pipeline });
}

void SimpleObjectRenderer::createPipelineBlend()
{
    // Vertex Input & binding for Normal Points
    VkVertexInputBindingDescription vertexBindingDesc[] = {
        {
            0, // binding
            2 * sizeof(float),
            VK_VERTEX_INPUT_RATE_VERTEX
        }
    };

    VkVertexInputAttributeDescription vertexAttrDesc[] = {
        { // position
            0, // location
            0, // binding
            VK_FORMAT_R32G32_SFLOAT,
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

    if (!m_vertBlendShader.isValid() ||
        !m_fragBlendShader.isValid())
    {
        check_vk_result(VK_ERROR_INVALID_SHADER_NV, "Missing shaders");
    }

    VkPipelineShaderStageCreateInfo shaderStages[2] = {
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_VERTEX_BIT,
            m_vertBlendShader.module(),
            "main",
            nullptr
        },
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            m_fragBlendShader.module(),
            "main",
            nullptr
        }
    };

    VkPipelineInputAssemblyStateCreateInfo ia = {};
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

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
    rs.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo ms = {};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.sampleShadingEnable = VK_FALSE;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo ds = {};
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable = VK_FALSE;
    ds.depthWriteEnable = VK_FALSE;
    ds.depthCompareOp = VK_COMPARE_OP_ALWAYS;

    VkPipelineColorBlendAttachmentState cbAttach = {
        VK_TRUE,              // blendEnable
        VK_BLEND_FACTOR_SRC_ALPHA,           // srcColorBlendFactor
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, // dstColorBlendFactor
        VK_BLEND_OP_ADD,                     // colorBlendOp
        VK_BLEND_FACTOR_ONE,                 // srcAlphaBlendFactor
        VK_BLEND_FACTOR_ONE,                 // dstAlphaBlendFactor
        VK_BLEND_OP_ADD,                     // alphaBlendOp
        0xF                                  // colorWriteMask
    };

    VkPipelineColorBlendStateCreateInfo cb = {};
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.attachmentCount = 1;
    cb.pAttachments = &cbAttach;

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
        m_pipelineBlendLayout,       // layout
        h_renderPass,                // renderPass
        2,                           // subpass
        VK_NULL_HANDLE,              // basePipelineHandle
        0                            // basePipelineIndex
    };

    VkResult err = h_pfn->vkCreateGraphicsPipelines(h_device, m_pipelineCache, 1, &pipelineInfo, nullptr, &m_pipelineBlend);
    check_vk_result(err, "Create Graphic Pipeline (Object)");
}

VkPipeline SimpleObjectRenderer::getPipeline(VkPrimitiveTopology topology, uint32_t subpass, bool useNormals, TlTopologyFlags allowedTopologies)
{
    assert(g_vkTopologyToTl.size() > (size_t)topology);

    // Line topologies do not use the normals
    bool normalsInput = useNormals && (g_vkTopologyToTl[topology] == TL_TOPOLOGY_TRIANGLE);

    // Filter the topologies
    if ((g_vkTopologyToTl[topology] & allowedTopologies) == 0)
        return VK_NULL_HANDLE;

    // Retrieve the pipeline from the set
    PipelineDef dummyDef = { topology, subpass, normalsInput, VK_NULL_HANDLE };
    if (m_pipelines.find(dummyDef) != m_pipelines.end())
        return m_pipelines.find(dummyDef)->pipeline;
    else
        return VK_NULL_HANDLE;
}

bool SimpleObjectRenderer::bindPipelineResources(VkCommandBuffer cmdBuffer, VkPrimitiveTopology topology, uint32_t subpass, VkUniformOffset mvpUni, bool useNormals, TlTopologyFlags allowedTopologies)
{
    VkPipeline vkPipe = getPipeline(topology, subpass, useNormals, allowedTopologies);
    if (vkPipe == VK_NULL_HANDLE)
        return false;
    h_pfn->vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipe);

    uint32_t offsets[] = { mvpUni };
    if (useNormals || g_vkTopologyToTl[topology] == TL_TOPOLOGY_LINE)
    {
        h_pfn->vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineMeshLayout_vf, 0, 1, &m_descSet_vf, 1, offsets);
    }
    else
    {
        h_pfn->vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineMeshLayout_vgf, 0, 1, &m_descSet_vgf, 1, offsets);
    }

    return true;
}

void SimpleObjectRenderer::cleanup()
{
    using namespace tls::vk;
    //*** Common Resources ***
    destroyPipelineCache(*h_pfn, h_device, m_pipelineCache);

    destroyPipelineLayout(*h_pfn, h_device, m_pipelineMeshLayout_vf);
    destroyPipelineLayout(*h_pfn, h_device, m_pipelineMeshLayout_vgf);
    destroyPipelineLayout(*h_pfn, h_device, m_pipelineBlendLayout);
    destroyDescriptorSetLayout(*h_pfn, h_device, m_descSetLayout_vf);
    destroyDescriptorSetLayout(*h_pfn, h_device, m_descSetLayout_vgf);

    //*** Shaders ***
    cleanShaders();

    //*** Pipelines ***
    for (PipelineDef def : m_pipelines)
        tls::vk::destroyPipeline(*h_pfn, h_device, def.pipeline);
    m_pipelines.clear();
    destroyPipeline(*h_pfn, h_device, m_pipelineBlend);
}

void SimpleObjectRenderer::cleanShaders()
{
    m_vertFacesShader.reset();
    m_geomFacesShader.reset();
    m_vertFacesNormalsShader.reset();
    m_fragFacesShader.reset();
    m_vertEdgesShader.reset();
    m_fragEdgesShader.reset();
    m_vertBlendShader.reset();
    m_fragBlendShader.reset();
}

void SimpleObjectRenderer::setTransformationMatrix(VkCommandBuffer _cmdBuffer, const glm::mat4& transfo)
{
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineMeshLayout_vf, VK_SHADER_STAGE_VERTEX_BIT, 0, 64, &transfo);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineMeshLayout_vgf, VK_SHADER_STAGE_VERTEX_BIT, 0, 64, &transfo);
}

void SimpleObjectRenderer::setObjectId(VkCommandBuffer _cmdBuffer, uint32_t id)
{
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineMeshLayout_vf, VK_SHADER_STAGE_FRAGMENT_BIT, 64, 4, &id);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineMeshLayout_vgf, VK_SHADER_STAGE_FRAGMENT_BIT, 64, 4, &id);
}

void SimpleObjectRenderer::setAlphaValue(VkCommandBuffer _cmdBuffer, float alpha)
{
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineMeshLayout_vf, VK_SHADER_STAGE_FRAGMENT_BIT, 72, 4, &alpha);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineMeshLayout_vgf, VK_SHADER_STAGE_FRAGMENT_BIT, 72, 4, &alpha);
}

void SimpleObjectRenderer::setAlphaBlendValue(VkCommandBuffer _cmdBuffer, float alpha)
{
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineBlendLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4, &alpha);
}

void SimpleObjectRenderer::setObjectFlags(VkCommandBuffer _cmdBuffer, bool isHovered, bool isSelected)
{
    uint32_t flags = 0;
    flags |= isHovered ? 0x01 : 0;
    flags |= isSelected ? 0x02 : 0;
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineMeshLayout_vf, VK_SHADER_STAGE_FRAGMENT_BIT, 68, 4, &flags);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineMeshLayout_vgf, VK_SHADER_STAGE_FRAGMENT_BIT, 68, 4, &flags);
}

void SimpleObjectRenderer::setColor(VkCommandBuffer _cmdBuffer, const glm::vec3& color)
{
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineMeshLayout_vf, VK_SHADER_STAGE_FRAGMENT_BIT, 80, 12, &color);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineMeshLayout_vgf, VK_SHADER_STAGE_FRAGMENT_BIT, 80, 12, &color);
}

