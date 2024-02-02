#include "MarkerRenderer.h"
#include "vulkan/VulkanManager.h"
#include "models/project/Marker.h"
#include "models/3d/Primitives.h"
#include "utils/Utils.h"
#include "services/MarkerDefinitions.hpp"
#include "vulkan/VulkanHelperFunctions.h"

#include <iostream>
#include <iomanip>

#include <QtGui/QImage>

#define _USE_MATH_DEFINES
#include <math.h>

static const int UNIFORM_DATA_SIZE = 16 * sizeof(float);

// ++++++++++++++++++++++  Shaders  +++++++++++++++++++++
static uint32_t marker_vert_spv[] =
{
#include "marker.vert.spv"
};

static uint32_t marker_geom_spv[] =
{
#include "marker.geom.spv"
};

static uint32_t marker_frag_spv[] =
{
#include "marker.frag.spv"
};
// ------------------------------------------------------

MarkerRenderer::MarkerRenderer()
{
    VulkanManager& vkm = VulkanManager::getInstance();

    h_device = vkm.getDevice();
    h_pfn = vkm.getDeviceFunctions();
    h_descPool = vkm.getDescriptorPool();
    h_renderPass = vkm.getObjectRenderPass();

    createShaders();
    createSampler();
    createPrimitives();

    createDescriptorSet_camera();
    createDescriptorSet_camera_geom();
    createDescriptorSet_sampler();
    createDescriptorSet_storageBuffers();

    createPipelines();
}

MarkerRenderer::~MarkerRenderer()
{
    cleanup();
}

void MarkerRenderer::createShaders()
{
    //**********************
    m_vsh_marker.createModule(h_device, h_pfn, marker_vert_spv, sizeof(marker_vert_spv));
    if (!m_vsh_marker.isValid())
        check_vk_result(VK_ERROR_INVALID_SHADER_NV, "Missing shaders");

    m_gsh_marker.createModule(h_device, h_pfn, marker_geom_spv, sizeof(marker_geom_spv));
    if (!m_gsh_marker.isValid())
        check_vk_result(VK_ERROR_INVALID_SHADER_NV, "Missing shaders");

    m_fsh_marker.createModule(h_device, h_pfn, marker_frag_spv, sizeof(marker_frag_spv));
    if (!m_fsh_marker.isValid())
        check_vk_result(VK_ERROR_INVALID_SHADER_NV, "Missing shaders");
}

void MarkerRenderer::createDescriptorSet_camera()
{
    // Descriptor Set Layout Binding
    VkDescriptorSetLayoutBinding layoutBindings[] =
    {
        {
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
    VkResult err = h_pfn->vkCreateDescriptorSetLayout(h_device, &descLayoutInfo, nullptr, &m_dsly_camera);
    check_vk_result(err, "Create Descriptor Set Layout");

    // Descriptor Set Allocation
    VkDescriptorSetAllocateInfo descSetAllocInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        nullptr,
        h_descPool,
        1,
        &m_dsly_camera
    };
    err = h_pfn->vkAllocateDescriptorSets(h_device, &descSetAllocInfo, &m_ds_camera);
    check_vk_result(err, "Allocate Descriptor Sets");

    VulkanManager& vkManager = VulkanManager::getInstance();

    VkBuffer uniBuf = vkManager.getUniformBuffer();

    // Write descriptors for the uniform buffers in the vertex and fragment shaders.
    VkDescriptorBufferInfo projViewUniInfo = {
        uniBuf, // buffer
        0, // offset
        vkManager.getUniformSizeAligned(3 * 64) // range
    };

    VkWriteDescriptorSet descWrite[1] = {};
    descWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descWrite[0].dstSet = m_ds_camera;
    descWrite[0].dstBinding = 0;
    descWrite[0].descriptorCount = 1;
    descWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descWrite[0].pBufferInfo = &projViewUniInfo;

    h_pfn->vkUpdateDescriptorSets(h_device, sizeof(descWrite) / sizeof(descWrite[0]), descWrite, 0, nullptr);
}

void MarkerRenderer::createDescriptorSet_camera_geom()
{
    // Descriptor Set Layout Binding
    VkDescriptorSetLayoutBinding layoutBinding =
    {
        4, // binding
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        1, // descriptor count
        VK_SHADER_STAGE_GEOMETRY_BIT,
        nullptr // used for immutable samplers
    };

    // Descriptor Set Layout Create Info
    VkDescriptorSetLayoutCreateInfo descLayoutInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        nullptr,
        0, // bits defined by extensions
        1, // bindingCount
        &layoutBinding
    };
    VkResult err = h_pfn->vkCreateDescriptorSetLayout(h_device, &descLayoutInfo, nullptr, &m_dsly_camera_geom);
    check_vk_result(err, "Create Descriptor Set Layout");

    // Descriptor Set Allocation
    VkDescriptorSetAllocateInfo descSetAllocInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        nullptr,
        h_descPool,
        1,
        &m_dsly_camera_geom
    };
    err = h_pfn->vkAllocateDescriptorSets(h_device, &descSetAllocInfo, &m_ds_camera_geom);
    check_vk_result(err, "Allocate Descriptor Sets");

    VulkanManager& vkManager = VulkanManager::getInstance();

    VkBuffer uniBuf = vkManager.getUniformBuffer();

    // Write descriptors for the uniform buffers in the vertex and fragment shaders.
    VkDescriptorBufferInfo projViewUniInfo = {
        uniBuf, // buffer
        0, // offset
        vkManager.getUniformSizeAligned(128) // range
    };

    VkWriteDescriptorSet descWrite = {};
    descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descWrite.dstSet = m_ds_camera_geom;
    descWrite.dstBinding = 4;
    descWrite.descriptorCount = 1;
    descWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descWrite.pBufferInfo = &projViewUniInfo;

    h_pfn->vkUpdateDescriptorSets(h_device, 1, &descWrite, 0, nullptr);
}

void MarkerRenderer::createDescriptorSet_sampler()
{
    VkDescriptorSetLayoutBinding layoutBindings[] =
    {
        2,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        1,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        nullptr
    };

    // Descriptor Set Layout Create Info
    VkDescriptorSetLayoutCreateInfo descLayoutInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        nullptr,
        0, // bits defined by extensions
        sizeof(layoutBindings) / sizeof(layoutBindings[0]), // bindingCount
        layoutBindings
    };
    VkResult err = h_pfn->vkCreateDescriptorSetLayout(h_device, &descLayoutInfo, nullptr, &m_dsly_sampler);
    check_vk_result(err, "Create Descriptor Set Layout");

    // Descriptor Set Allocation
    VkDescriptorSetAllocateInfo descSetAllocInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        nullptr,
        h_descPool,
        1,
        &m_dsly_sampler
    };
    err = h_pfn->vkAllocateDescriptorSets(h_device, &descSetAllocInfo, &m_ds_sampler);
    check_vk_result(err, "Allocate Descriptor Sets");

    VulkanManager& vkManager = VulkanManager::getInstance();

    VkDescriptorImageInfo imageInfo = {
        m_textureSampler,       // sampler
        m_textureImageView,     // imageView
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL  // imageLayout
    };

    VkWriteDescriptorSet descWrite = {};
    descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descWrite.dstSet = m_ds_sampler;
    descWrite.dstBinding = 2;
    descWrite.descriptorCount = 1;
    descWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descWrite.pImageInfo = &imageInfo;

    h_pfn->vkUpdateDescriptorSets(h_device, 1, &descWrite, 0, nullptr);
}

void MarkerRenderer::createDescriptorSet_storageBuffers()
{
    VkDescriptorSetLayoutBinding layoutBindings[] = {
        {
            5,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            1, // descriptor count
            VK_SHADER_STAGE_GEOMETRY_BIT,
            nullptr
        },
        {
            6,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            1, // descriptor count
            VK_SHADER_STAGE_GEOMETRY_BIT,
            nullptr
        }
    };

    VkDescriptorSetLayoutCreateInfo descLayoutInfo = {
    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    nullptr,
    0, // bits defined by extensions
    sizeof(layoutBindings) / sizeof(layoutBindings[0]), // bindingCount
    layoutBindings
    };
    VkResult err = h_pfn->vkCreateDescriptorSetLayout(h_device, &descLayoutInfo, nullptr, &m_dsly_storageBuffers);
    check_vk_result(err, "Create Descriptor Set Layout");

    VkDescriptorSetAllocateInfo ds_allocInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        nullptr,
        h_descPool,
        1,
        &m_dsly_storageBuffers
    };
    err = h_pfn->vkAllocateDescriptorSets(h_device, &ds_allocInfo, &m_ds_storageBuffers);
    check_vk_result(err, "Allocate Descriptor Sets");

    // Update Descriptor Set
    VkDescriptorBufferInfo storageBufferInfo[] = {
        {
            m_vertPosBuf.buffer, // buffer
            0,  // offset
            m_vertPosBuf.size // range
        },
        {
            m_vertUVBuf.buffer,
            0,
            m_vertUVBuf.size
        }
    };

    VkWriteDescriptorSet descWrite[2] = {};
    descWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descWrite[0].dstSet = m_ds_storageBuffers;
    descWrite[0].dstBinding = 5;
    descWrite[0].descriptorCount = 1;
    descWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descWrite[0].pBufferInfo = storageBufferInfo;

    descWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descWrite[1].dstSet = m_ds_storageBuffers;
    descWrite[1].dstBinding = 6;
    descWrite[1].descriptorCount = 1;
    descWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descWrite[1].pBufferInfo = &storageBufferInfo[1];

    h_pfn->vkUpdateDescriptorSets(h_device, sizeof(descWrite) / sizeof(descWrite[0]), descWrite, 0, nullptr);
}

void MarkerRenderer::createPipelines()
{
    VkPipelineCacheCreateInfo pipelineCacheInfo = {};
    pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VkResult err = h_pfn->vkCreatePipelineCache(h_device, &pipelineCacheInfo, nullptr, &m_pipelineCache);
    check_vk_result(err, "Create Pipeline Cache");

    createPipelineLayout_marker_dyn();
    createGraphicPipeline_marker_dyn();
}

void MarkerRenderer::createPipelineLayout_marker_dyn()
{
    VkPushConstantRange pcr[] =
    {
        {
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            16 // scale near/far + near/far
        },
        {
            VK_SHADER_STAGE_FRAGMENT_BIT,
            16,
            64 // borderColor + depth render + extent
        }
    };

    VkDescriptorSetLayout pDescSetLayout[] = { m_dsly_camera, m_dsly_sampler, VulkanManager::getDSLayout_inputDepth(), m_dsly_camera_geom, m_dsly_storageBuffers };
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = sizeof(pcr) / sizeof(pcr[0]);
    pipelineLayoutInfo.pPushConstantRanges = pcr;
    pipelineLayoutInfo.setLayoutCount = sizeof(pDescSetLayout) / sizeof(VkDescriptorSetLayout);
    pipelineLayoutInfo.pSetLayouts = pDescSetLayout;
    VkResult err = h_pfn->vkCreatePipelineLayout(h_device, &pipelineLayoutInfo, nullptr, &m_pply_marker_dyn);
    check_vk_result(err, "Create Pipeline Layout");
}


void MarkerRenderer::createGraphicPipeline_marker_dyn()
{
    // Vertex Input & binding for Normal Points
    VkVertexInputBindingDescription vertexBindingDesc[] = {
        {
            0,
            8 * sizeof(float),
            VK_VERTEX_INPUT_RATE_VERTEX
        }
    };

    VkVertexInputAttributeDescription vertexAttrDesc[] = {
        { // markerPos
            0,
            0,
            VK_FORMAT_R32G32B32_SFLOAT,
            0
        },
        { // markerColor
            1,
            0,
            VK_FORMAT_R8G8B8A8_UINT,
            3 * sizeof(float)
        },
        { // graphicID
            2,
            0,
            VK_FORMAT_R32_UINT,
            4 * sizeof(float)
        },
        { // textureID
            3,
            0,
            VK_FORMAT_R32_UINT,
            5 * sizeof(float)
        },
        { // firstVertex
            4,
            0,
            VK_FORMAT_R16_UINT,
            6 * sizeof(float)
        },
        { // vertexCount
            5,
            0,
            VK_FORMAT_R16_UINT,
            6 * sizeof(float) + sizeof(uint16_t)
        },
        { // style
            6,
            0,
            VK_FORMAT_R32_UINT,
            7 * sizeof(float)
        }
    };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pNext = nullptr;
    vertexInputInfo.flags = 0;
    vertexInputInfo.vertexBindingDescriptionCount = sizeof(vertexBindingDesc) / sizeof(vertexBindingDesc[0]);
    vertexInputInfo.pVertexBindingDescriptions = vertexBindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = sizeof(vertexAttrDesc) / sizeof(vertexAttrDesc[0]);
    vertexInputInfo.pVertexAttributeDescriptions = vertexAttrDesc;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

    VkPipelineRasterizationStateCreateInfo rasterizationState = {};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL; // {FILL, LINE, POINT}
    rasterizationState.cullMode = VK_CULL_MODE_NONE;
    rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizationState.lineWidth = 1.0f;

    // The viewport and scissor will be set dynamically via vkCmdSetViewport / Scissor.
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
    ds.depthTestEnable = VK_FALSE;
    ds.depthWriteEnable = VK_FALSE;

    const size_t cbAttachCount = 2;
    // NOTE - it may be usefull to use the "independent blending" feature if 
    //        we want to avoid strange id calculation due to an alpha != {0, 1}
    VkPipelineColorBlendAttachmentState cbAttach[cbAttachCount] = {
        {
            VK_TRUE,                             // blendEnable
            VK_BLEND_FACTOR_ONE,                 // srcColorBlendFactor
            VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, // dstColorBlendFactor
            VK_BLEND_OP_ADD,                     // colorBlendOp
            VK_BLEND_FACTOR_ONE,                 // srcAlphaBlendFactor  // NOT USE ?
            VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, // dstAlphaBlendFactor
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
        },
    };

    VkPipelineColorBlendStateCreateInfo colorBlend = {};
    colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlend.attachmentCount = cbAttachCount;
    colorBlend.pAttachments = cbAttach;

    VkDynamicState dynEnable[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dyn;
    memset(&dyn, 0, sizeof(dyn));
    dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn.dynamicStateCount = sizeof(dynEnable) / sizeof(VkDynamicState);
    dyn.pDynamicStates = dynEnable;

    VkPipelineShaderStageCreateInfo vertStage = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        VK_SHADER_STAGE_VERTEX_BIT,
        m_vsh_marker.module(),
        "main",
        nullptr
    };

    VkPipelineShaderStageCreateInfo geomStage = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        VK_SHADER_STAGE_GEOMETRY_BIT,
        m_gsh_marker.module(),
        "main",
        nullptr
    };

    VkPipelineShaderStageCreateInfo fragStage = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        m_fsh_marker.module(),
        "main",
        nullptr
    };

    constexpr int stageCount = 3;
    VkPipelineShaderStageCreateInfo stages[stageCount] = { vertStage, geomStage, fragStage };

    // Graphics pipeline creation
    VkGraphicsPipelineCreateInfo info = {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        nullptr,                 // pNext
        0,                       // flags
        stageCount,              // stageCount
        stages,                  // pStages
        &vertexInputInfo,        // pVertexInputState
        &inputAssembly,          // pInputAssembleState
        nullptr,                 // pTesselationState
        &vp,                     // pViewportState
        &rasterizationState,     // pRasterizationState
        &ms,                     // pMultisampleState
        &ds,                     // pDepthStencilState
        &colorBlend,             // pColorBlendState
        &dyn,                    // pDynamicState
        m_pply_marker_dyn,       // layout
        h_renderPass,            // renderPass
        3,                       // subpass
        VK_NULL_HANDLE,          // basePipelineHandle
        0                        // basePipelineIndex
    };
    VkResult err = h_pfn->vkCreateGraphicsPipelines(h_device, m_pipelineCache, 1, &info, nullptr, &m_pp_marker_dyn);
    check_vk_result(err, "Create Graphic Pipeline (markers)");
}

void MarkerRenderer::createPrimitives()
{
    //******* Load the vertices in the GPU buffers *********//
    uint32_t vertexOffset = 0;
    std::vector<glm::vec2> positions;
    std::vector<glm::vec2> uvs;
    // Generate all different alignment
    for (int sh = 0; sh < (int)scs::MarkerShape::Max_Enum; ++sh)
    {
        uint32_t vertexCount = 0;
        // without arrow
        scs::primitives::generateMarkerShape(positions, uvs, (scs::MarkerShape)sh);

        vertexCount = (uint32_t)positions.size() - vertexOffset;
        scs::PrimitiveDef prim = scs::g_shapePrimitives.at((scs::MarkerShape)sh);
        assert(prim.firstVertex == vertexOffset && prim.vertexCount == vertexCount);
        vertexOffset += vertexCount;
    }

    //******* Allocate buffers for the primitives with enough space *******//
    VulkanManager& vkm = VulkanManager::getInstance();
    VkDeviceSize posSize = positions.size() * sizeof(glm::vec2);
    vkm.allocSimpleBuffer(posSize, m_vertPosBuf, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    VkDeviceSize uvSize = uvs.size() * sizeof(glm::vec2);
    vkm.allocSimpleBuffer(uvSize, m_vertUVBuf, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    VkDeviceSize posBufOffset = 0;
    VkDeviceSize uvBufOffset = 0;
    vkm.loadInSimpleBuffer(m_vertPosBuf, positions.size() * sizeof(glm::vec2), positions.data(), posBufOffset, 4);
    vkm.loadInSimpleBuffer(m_vertUVBuf, uvs.size() * sizeof(glm::vec2), uvs.data(), uvBufOffset, 4);
}

void MarkerRenderer::createSampler()
{
    // TODO - How to check the sizes of the images
    uint32_t layerCount = (uint32_t)scs::MarkerIcon::Max_Enum;

    if (VulkanManager::getInstance().createTextureArray(100, 100, layerCount, m_textureImage, m_textureImageView, m_textureSampler, m_textureMemory) == false)
    {
        Logger::log(LoggerMode::VKLog) << "Failed to create texture array." << Logger::endl;
        return;
    }

    // Load texture for each layer
    for (auto iconRes : scs::markerStyleDefs)
    {
        QImage iconImage(iconRes.second.qresource);
        QImage::Format format = iconImage.format();
        const uchar* imageBits = iconImage.constBits();

        if (format == QImage::Format::Format_Indexed8)
        {
            uchar* bitsRGBA = new uchar[100 * 100 * 4];
            QVector<uint> table = iconImage.colorTable();
            for (int row = 0; row < 100; ++row)
            {
                for (int col = 0; col < 100; ++col)
                {
                    unsigned int pixOffset = row * 100 + col;
                    memcpy(bitsRGBA + 4 * pixOffset, table.data() + imageBits[pixOffset], 4);
                }
            }
            if (VulkanManager::getInstance().uploadTextureArray(bitsRGBA, iconImage.width(), iconImage.height(), m_textureImage, (uint32_t)iconRes.first, layerCount) == false)
                Logger::log(LoggerMode::VKLog) << "Failed to upload texture layer for icon " << iconRes.second.qresource.toStdString() << Logger::endl;
            delete bitsRGBA;
        }
        else
        {
            if (VulkanManager::getInstance().uploadTextureArray(imageBits, iconImage.width(), iconImage.height(), m_textureImage, (uint32_t)iconRes.first, layerCount) == false)
            {
                Logger::log(LoggerMode::VKLog) << "Failed to upload texture layer for icon " << iconRes.second.qresource.toStdString() << Logger::endl;
            }
        }
    }
}

void MarkerRenderer::cleanup()
{
    //*** DescriptorSets ***
    tls::vk::freeDescriptorSet(*h_pfn, h_device, h_descPool, m_ds_camera);
    tls::vk::freeDescriptorSet(*h_pfn, h_device, h_descPool, m_ds_camera_geom);
    tls::vk::freeDescriptorSet(*h_pfn, h_device, h_descPool, m_ds_sampler);
    tls::vk::freeDescriptorSet(*h_pfn, h_device, h_descPool, m_ds_storageBuffers);

    tls::vk::destroyDescriptorSetLayout(*h_pfn, h_device, m_dsly_camera);
    tls::vk::destroyDescriptorSetLayout(*h_pfn, h_device, m_dsly_camera_geom);
    tls::vk::destroyDescriptorSetLayout(*h_pfn, h_device, m_dsly_sampler);
    tls::vk::destroyDescriptorSetLayout(*h_pfn, h_device, m_dsly_storageBuffers);

    //*** Shaders ***
    m_vsh_marker.reset();
    m_gsh_marker.reset();
    m_fsh_marker.reset();

    //*** Pipelines ***
    tls::vk::destroyPipelineCache(*h_pfn, h_device, m_pipelineCache);
    tls::vk::destroyPipelineLayout(*h_pfn, h_device, m_pply_marker_dyn);
    tls::vk::destroyPipeline(*h_pfn, h_device, m_pp_marker_dyn);

    //*** Externaly allocated
    VulkanManager::getInstance().freeAllocation(m_vertPosBuf);
    VulkanManager::getInstance().freeAllocation(m_vertUVBuf);

    // Texture resources
    tls::vk::destroyImage(*h_pfn, h_device, m_textureImage);
    tls::vk::destroyImageView(*h_pfn, h_device, m_textureImageView);
    tls::vk::destroySampler(*h_pfn, h_device, m_textureSampler);
    tls::vk::freeMemory(*h_pfn, h_device, m_textureMemory);
}

void MarkerRenderer::drawMarkerData(VkCommandBuffer _cmdBuffer, SimpleBuffer drawDataBuf, uint32_t firstMarker, uint32_t markerCount, VkUniformOffset viewProjUniOffset, VkDescriptorSet depthDescSet)
{
    if (m_vertPosBuf.buffer == VK_NULL_HANDLE)
        return;

    if (markerCount == 0)
        return;

    h_pfn->vkCmdBindPipeline(_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pp_marker_dyn);

    // Bind the descriptor set
    VkDescriptorSet descSets_A[] = { m_ds_camera, m_ds_sampler, depthDescSet };
    h_pfn->vkCmdBindDescriptorSets(_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pply_marker_dyn, 0, 3, descSets_A, 1, &viewProjUniOffset);
    VkDescriptorSet descSets_B[] = { m_ds_camera_geom, m_ds_storageBuffers };
    h_pfn->vkCmdBindDescriptorSets(_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pply_marker_dyn, 3, 2, descSets_B, 1, &viewProjUniOffset);

    VkDeviceSize offset = 0;
    h_pfn->vkCmdBindVertexBuffers(_cmdBuffer, 0, 1, &drawDataBuf.buffer, &offset);

    h_pfn->vkCmdDraw(_cmdBuffer, markerCount, 1, firstMarker, 0);
}

void MarkerRenderer::setScaleConstants(VkCommandBuffer _cmdBuffer, float nearScale, float farScale, float nearDist, float farDist)
{
    // Scale
    float packedData[4] = { nearScale, farScale, nearDist, farDist };
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pply_marker_dyn, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(packedData), packedData);
}

void MarkerRenderer::setBordersColorConstant(VkCommandBuffer _cmdBuffer, glm::vec4 neutral, glm::vec4 hover, glm::vec4 select)
{
    // Select color
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pply_marker_dyn, VK_SHADER_STAGE_FRAGMENT_BIT, 16, 16, &neutral);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pply_marker_dyn, VK_SHADER_STAGE_FRAGMENT_BIT, 48, 16, &hover);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pply_marker_dyn, VK_SHADER_STAGE_FRAGMENT_BIT, 64, 16, &select);
}

void MarkerRenderer::setDepthRenderConstant(VkCommandBuffer _cmdBuffer, bool _useDepthRender)
{
    int value = (int)_useDepthRender;
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pply_marker_dyn, VK_SHADER_STAGE_FRAGMENT_BIT, 32, 4, &value);
}

void MarkerRenderer::setViewportSizeConstants(VkCommandBuffer _cmdBuffer, VkExtent2D _extent)
{
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pply_marker_dyn, VK_SHADER_STAGE_FRAGMENT_BIT, 36, 8, &_extent);
}
