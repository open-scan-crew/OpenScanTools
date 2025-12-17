#include "Renderer.h"
#include "vulkan/VulkanManager.h"
#include "utils/Logger.h"

#include "models/3d/UniformClippingData.h"

#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

static const int UNIFORM_DATA_SIZE = 16 * sizeof(float);
static constexpr VkShaderStageFlags POINT_STAGE_FLAGS = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

// ********** Shaders Declaration *************

static uint32_t point_flat_vert_spv[] =
{
#include "point_flat.vert.spv"
};

static uint32_t point_flat_clip_vert_spv[] =
{
#include "point_flat_clip.vert.spv"
};

static uint32_t point_i_standard_vert_spv[] =
{
#include "point_i_standard.vert.spv"
};

static uint32_t point_i_standard_clip_vert_spv[] =
{
#include "point_i_standard_clip.vert.spv"
};

static uint32_t point_i_fake_color_vert_spv[] =
{
#include "point_i_fake_color.vert.spv"
};

static uint32_t point_i_fake_color_clip_vert_spv[] =
{
#include "point_i_fake_color_clip.vert.spv"
};

static uint32_t point_i_colored_vert_spv[] =
{
#include "point_i_colored.vert.spv"
};

static uint32_t point_i_colored_clip_vert_spv[] =
{
#include "point_i_colored_clip.vert.spv"
};

static uint32_t point_i_ramp_dist_vert_spv[] =
{
#include "point_i_ramp_dist.vert.spv"
};

static uint32_t point_i_ramp_dist_clip_vert_spv[] =
{
#include "point_i_ramp_dist_clip.vert.spv"
};

static uint32_t point_i_ramp_dist_flat_vert_spv[] =
{
#include "point_i_ramp_dist_flat.vert.spv"
};

static uint32_t point_i_ramp_dist_flat_clip_vert_spv[] =
{
#include "point_i_ramp_dist_flat_clip.vert.spv"
};

static uint32_t point_rgb_standard_vert_spv[] =
{
#include "point_rgb_standard.vert.spv"
};

static uint32_t point_rgb_standard_clip_vert_spv[] =
{
#include "point_rgb_standard_clip.vert.spv"
};

static uint32_t point_rgb_colored_vert_spv[] =
{
#include "point_rgb_colored.vert.spv"
};

static uint32_t point_rgb_colored_clip_vert_spv[] =
{
#include "point_rgb_colored_clip.vert.spv"
};

static uint32_t point_rgb_ramp_dist_vert_spv[] =
{
#include "point_rgb_ramp_dist.vert.spv"
};

static uint32_t point_rgb_ramp_dist_clip_vert_spv[] =
{
#include "point_rgb_ramp_dist_clip.vert.spv"
};

static uint32_t point_rgb_ramp_dist_flat_vert_spv[] =
{
#include "point_rgb_ramp_dist_flat.vert.spv"
};

static uint32_t point_rgb_ramp_dist_flat_clip_vert_spv[] =
{
#include "point_rgb_ramp_dist_flat_clip.vert.spv"
};

static uint32_t point_rgbi_standard_vert_spv[] =
{
#include "point_rgbi_standard.vert.spv"
};

static uint32_t point_rgbi_standard_clip_vert_spv[] =
{
#include "point_rgbi_standard_clip.vert.spv"
};

static uint32_t point_frag_spv[] =
{
#include "point.frag.spv"
};

static uint32_t point_geom_clip_spv[] =
{
#include "point_clip.geom.spv"
};

// ********************************************

Renderer::ShaderDefinition::ShaderDefinition(const ShaderType& type, const std::string& file, const std::string& entryPoint)
    : _type(type)
    , _file(file)
    , _entryPoint(entryPoint)
{}

Renderer::RenderingDefinition::RenderingDefinition(const RenderMode& renderMode, const tls::PointFormat& pointFormat)
    : _renderMode(renderMode)
    , _format(pointFormat)
{ }

Renderer::Renderer()
{
    VulkanManager& vkm = VulkanManager::getInstance();

    h_device = vkm.getDevice();
    h_pfn = vkm.getDeviceFunctions();
    h_descPool = vkm.getDescriptorPool();
    // Enable different format as render passes output
    h_renderPasses.insert({ VK_FORMAT_B8G8R8A8_UNORM, vkm.getPCRenderPass(VK_FORMAT_B8G8R8A8_UNORM) });
    h_renderPasses.insert({ VK_FORMAT_R16G16B16A16_SFLOAT, vkm.getPCRenderPass(VK_FORMAT_R16G16B16A16_SFLOAT) });

    m_vertexShadersSrc = {
        { RenderMode::Intensity, {
            { tls::PointFormat::TL_POINT_XYZ_I, { point_i_standard_vert_spv, sizeof(point_i_standard_vert_spv), point_i_standard_clip_vert_spv, sizeof(point_i_standard_clip_vert_spv), true }},
            { tls::PointFormat::TL_POINT_XYZ_RGB, { point_rgb_standard_vert_spv, sizeof(point_rgb_standard_vert_spv), point_rgb_standard_clip_vert_spv, sizeof(point_rgb_standard_clip_vert_spv), true }},
            { tls::PointFormat::TL_POINT_XYZ_I_RGB, { point_i_standard_vert_spv, sizeof(point_i_standard_vert_spv), point_i_standard_clip_vert_spv, sizeof(point_i_standard_clip_vert_spv), true }}}
        },
        { RenderMode::Fake_Color, {
            { tls::PointFormat::TL_POINT_XYZ_I, { point_i_fake_color_vert_spv, sizeof(point_i_fake_color_vert_spv), point_i_fake_color_clip_vert_spv, sizeof(point_i_fake_color_clip_vert_spv), true }},
            { tls::PointFormat::TL_POINT_XYZ_RGB, { point_rgb_standard_vert_spv, sizeof(point_rgb_standard_vert_spv), point_rgb_standard_clip_vert_spv, sizeof(point_rgb_standard_clip_vert_spv), true }},
            { tls::PointFormat::TL_POINT_XYZ_I_RGB, { point_i_fake_color_vert_spv, sizeof(point_i_fake_color_vert_spv), point_i_fake_color_clip_vert_spv, sizeof(point_i_fake_color_clip_vert_spv), true }}}
        },
        { RenderMode::RGB, {
            { tls::PointFormat::TL_POINT_XYZ_I, { point_i_standard_vert_spv, sizeof(point_i_standard_vert_spv), point_i_standard_clip_vert_spv, sizeof(point_i_standard_clip_vert_spv), true }},
            { tls::PointFormat::TL_POINT_XYZ_RGB, { point_rgb_standard_vert_spv, sizeof(point_rgb_standard_vert_spv), point_rgb_standard_clip_vert_spv, sizeof(point_rgb_standard_clip_vert_spv), true }},
            { tls::PointFormat::TL_POINT_XYZ_I_RGB, { point_rgb_standard_vert_spv, sizeof(point_rgb_standard_vert_spv), point_rgb_standard_clip_vert_spv, sizeof(point_rgb_standard_clip_vert_spv), true }}}
        },
        { RenderMode::IntensityRGB_Combined, {
            { tls::PointFormat::TL_POINT_XYZ_I, { point_i_standard_vert_spv, sizeof(point_i_standard_vert_spv), point_i_standard_clip_vert_spv, sizeof(point_i_standard_clip_vert_spv), true }},
            { tls::PointFormat::TL_POINT_XYZ_RGB, { point_rgb_standard_vert_spv, sizeof(point_rgb_standard_vert_spv), point_rgb_standard_clip_vert_spv, sizeof(point_rgb_standard_clip_vert_spv), true }},
            { tls::PointFormat::TL_POINT_XYZ_I_RGB, { point_rgbi_standard_vert_spv, sizeof(point_rgbi_standard_vert_spv), point_rgbi_standard_clip_vert_spv, sizeof(point_rgbi_standard_clip_vert_spv), true }}}
        },
        { RenderMode::Grey_Colored, {
            { tls::PointFormat::TL_POINT_XYZ_I, { point_i_colored_vert_spv, sizeof(point_i_colored_vert_spv), point_i_colored_clip_vert_spv, sizeof(point_i_colored_clip_vert_spv), true }},
            { tls::PointFormat::TL_POINT_XYZ_RGB, { point_rgb_colored_vert_spv, sizeof(point_rgb_colored_vert_spv), point_rgb_colored_clip_vert_spv, sizeof(point_rgb_colored_clip_vert_spv), true}},
            { tls::PointFormat::TL_POINT_XYZ_I_RGB, { point_i_colored_vert_spv, sizeof(point_i_colored_vert_spv), point_i_colored_clip_vert_spv, sizeof(point_i_colored_clip_vert_spv), true }}}
        },
        { RenderMode::Grey_Distance_Ramp, {
            { tls::PointFormat::TL_POINT_XYZ_I, { point_i_ramp_dist_vert_spv, sizeof(point_i_ramp_dist_vert_spv), point_i_ramp_dist_clip_vert_spv, sizeof(point_i_ramp_dist_clip_vert_spv), true}},
            { tls::PointFormat::TL_POINT_XYZ_RGB, { point_rgb_ramp_dist_vert_spv, sizeof(point_rgb_ramp_dist_vert_spv), point_rgb_ramp_dist_clip_vert_spv, sizeof(point_rgb_ramp_dist_clip_vert_spv), true}},
            { tls::PointFormat::TL_POINT_XYZ_I_RGB, { point_i_ramp_dist_vert_spv, sizeof(point_i_ramp_dist_vert_spv), point_i_ramp_dist_clip_vert_spv, sizeof(point_i_ramp_dist_clip_vert_spv), true}}}
        },
        { RenderMode::Flat, {
            { tls::PointFormat::TL_POINT_XYZ_I, { point_flat_vert_spv, sizeof(point_flat_vert_spv), point_flat_clip_vert_spv, sizeof(point_flat_clip_vert_spv), true}},
            { tls::PointFormat::TL_POINT_XYZ_RGB, { point_flat_vert_spv, sizeof(point_flat_vert_spv), point_flat_clip_vert_spv, sizeof(point_flat_clip_vert_spv), true}},
            { tls::PointFormat::TL_POINT_XYZ_I_RGB, { point_flat_vert_spv, sizeof(point_flat_vert_spv), point_flat_clip_vert_spv, sizeof(point_flat_clip_vert_spv), true}}}
        },
        { RenderMode::Flat_Distance_Ramp, {
            { tls::PointFormat::TL_POINT_XYZ_I, { point_i_ramp_dist_flat_vert_spv, sizeof(point_i_ramp_dist_flat_vert_spv), point_i_ramp_dist_flat_clip_vert_spv, sizeof(point_i_ramp_dist_flat_clip_vert_spv), true}},
            { tls::PointFormat::TL_POINT_XYZ_RGB, { point_rgb_ramp_dist_flat_vert_spv, sizeof(point_rgb_ramp_dist_flat_vert_spv), point_rgb_ramp_dist_flat_clip_vert_spv, sizeof(point_rgb_ramp_dist_flat_clip_vert_spv), true}},
            { tls::PointFormat::TL_POINT_XYZ_I_RGB, { point_i_ramp_dist_flat_vert_spv, sizeof(point_i_ramp_dist_flat_vert_spv), point_i_ramp_dist_flat_clip_vert_spv, sizeof(point_i_ramp_dist_flat_clip_vert_spv), true}}}
        }
    };

    createShaders();
    createDescriptorSetLayout();
    createDescriptorSets();
    createPipelines();
}

Renderer::~Renderer()
{
    cleanup();
}

void Renderer::createShaders()
{
    for (const auto& it_mode : m_vertexShadersSrc)
    {
        for (const auto& it_format : it_mode.second)
        {
            std::string key = getShaderKey(it_mode.first, it_format.first, false);
            if (m_shadersByFiles.find(key) == m_shadersByFiles.end())
            {
                m_shadersByFiles[key].createModule(h_device, h_pfn, it_format.second.noClip, it_format.second.noClipSize);
                if (!m_shadersByFiles[key].isValid())
                    check_vk_result(VK_ERROR_INVALID_SHADER_NV, "Missing Vertex shader");
            }

            key = getShaderKey(it_mode.first, it_format.first, true);
            if (m_shadersByFiles.find(key) == m_shadersByFiles.end())
            {
                m_shadersByFiles[key].createModule(h_device, h_pfn, it_format.second.withClip, it_format.second.withClipSize);
                if (!m_shadersByFiles[key].isValid())
                    check_vk_result(VK_ERROR_INVALID_SHADER_NV, "Missing Vertex shader");
            }
        }
    }
    m_shaderFrag.createModule(h_device, h_pfn, point_frag_spv, sizeof(point_frag_spv));
    if (!m_shaderFrag.isValid())
        check_vk_result(VK_ERROR_INVALID_SHADER_NV, "Missing fragment shader");

    m_shaderGeomClip.createModule(h_device, h_pfn, point_geom_clip_spv, sizeof(point_geom_clip_spv));
    if (!m_shaderGeomClip.isValid())
        check_vk_result(VK_ERROR_INVALID_SHADER_NV, "Missing geometry clipping shader");
}

std::string Renderer::getShaderKey(RenderMode renderMode, tls::PointFormat format, bool clip) const
{
    std::string key = "";

    switch (renderMode)
    {
    case (Intensity):
        key += "I";
        break;
    case (RGB):
        key += "RGB";
        break;
    case (IntensityRGB_Combined):
        key += "RGBI";
        break;
    case (Grey_Colored):
        key += "Colored";
        break;
    case (Grey_Distance_Ramp):
        key += "Ramp_dist";
        break;
    case (Flat):
        key += "Flat";
        break;
    case (Fake_Color):
        key += "Fake_Color";
        break;
    case (Flat_Distance_Ramp):
        key += "Flat_Ramp_dist";
        break;
    }

    key += "_for_";

    switch (format)
    {
    case (tls::TL_POINT_XYZ_I):
        key += "Point_I";
        break;
    case (tls::TL_POINT_XYZ_RGB):
        key += "Point_RGB";
        break;
    case (tls::TL_POINT_XYZ_I_RGB):
        key += "Point_RGBI";
        break;
    }

    if (clip)
        key += "_clip";
    return key;
}

void Renderer::createDescriptorSetLayout()
{
    // Descriptor Set Layout Binding
    VkDescriptorSetLayoutBinding layoutBindings[] =
    {
        { // proj & view
            0, // binding
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            1, // descriptor count
            VK_SHADER_STAGE_VERTEX_BIT,
            nullptr // used for immutable samplers
        },
        { // model (scan)
            1,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            1,
            VK_SHADER_STAGE_VERTEX_BIT,
            nullptr
        }
    };

    // DescriptorÂ Set Layout Create Info
    VkDescriptorSetLayoutCreateInfo descLayoutInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        nullptr,
        0, // bits defined by extensions
        sizeof(layoutBindings) / sizeof(layoutBindings[0]), // bindingCount
        layoutBindings
    };
    VkResult err = h_pfn->vkCreateDescriptorSetLayout(h_device, &descLayoutInfo, nullptr, &m_descSetLayout);
    check_vk_result(err, "Create Descriptor Set Layout");

    // Descriptor Set Layout Binding
    VkDescriptorSetLayoutBinding layoutBindings_cb[] =
    {
        { // proj & view
            0, // binding
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            1, // descriptor count
            VK_SHADER_STAGE_VERTEX_BIT,
            nullptr // used for immutable samplers
        },
        { // model (scan)
            1,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            1,
            VK_SHADER_STAGE_VERTEX_BIT,
            nullptr
        },
        { // proj & view
            2, // binding
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            1, // descriptor count
            VK_SHADER_STAGE_GEOMETRY_BIT,
            nullptr // used for immutable samplers
        },
        { // clipping box normalized & ramp params
            3,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            1,
            VK_SHADER_STAGE_GEOMETRY_BIT,
            nullptr
        }
    };

    // DescriptorÂ Set Layout Create Info
    VkDescriptorSetLayoutCreateInfo descLayoutInfo_cb = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        nullptr,
        0, // bits defined by extensions
        sizeof(layoutBindings_cb) / sizeof(layoutBindings_cb[0]), // bindingCount
        layoutBindings_cb
    };
    err = h_pfn->vkCreateDescriptorSetLayout(h_device, &descLayoutInfo_cb, nullptr, &m_descSetLayout_cb);
    check_vk_result(err, "Create Descriptor Set Layout (clipping box)");
}

void Renderer::createDescriptorSets()
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

    // Descriptor Set Allocation
    descSetAllocInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        nullptr,
        h_descPool,
        1,
        &m_descSetLayout_cb
    };
    err = h_pfn->vkAllocateDescriptorSets(h_device, &descSetAllocInfo, &m_descSet_cb);
    check_vk_result(err, "Allocate Descriptor Sets (clipping box)");

    VulkanManager& vkManager = VulkanManager::getInstance();

    VkBuffer uniBuf = vkManager.getUniformBuffer();

    // Check the uniform size(range for vulkan).
    // For future use with an array of matrices. The spec guarante that the max is minimum 16384 bytes.
    VkPhysicalDeviceLimits devLimits = vkManager.getPhysicalDeviceLimits();
    VkDeviceSize maxUniSize = MAX_ACTIVE_CLIPPING * sizeof(UniformClippingData);
    if (maxUniSize > devLimits.maxUniformBufferRange)
        Logger::log(VK_LOG) << "Warning: the uniform range is too large for the device." << Logger::endl;

    // Write descriptors for the uniform buffers in the vertex and fragment shaders.
    VkDescriptorBufferInfo bufferInfo[4] = {
        {
            uniBuf, // buffer
            0, // offset
            vkManager.getUniformSizeAligned(64 * 3) // range
        }, {
            uniBuf,
            0,
            vkManager.getUniformSizeAligned(64)
        }, {
            uniBuf,
            0,
            vkManager.getUniformSizeAligned(64 * 3)
        }, {
            uniBuf,
            0,
            MAX_ACTIVE_CLIPPING * (sizeof(UniformClippingData))
        }
    };

    VkWriteDescriptorSet descWrite[4];
    memset(descWrite, 0, sizeof(descWrite));
    for (uint32_t i = 0; i < sizeof(descWrite) / sizeof(descWrite[0]); ++i)
    {
        descWrite[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite[i].dstSet = m_descSet;
        descWrite[i].dstBinding = i;
        descWrite[i].descriptorCount = 1;
        descWrite[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descWrite[i].pBufferInfo = (bufferInfo + i);
    }
    h_pfn->vkUpdateDescriptorSets(h_device, 2, descWrite, 0, nullptr);

    for (uint32_t i = 0; i < sizeof(descWrite) / sizeof(descWrite[0]); ++i)
    {
        descWrite[i].dstSet = m_descSet_cb;
    }
    // Change the range for the clipping boxes matrices uniform array
    h_pfn->vkUpdateDescriptorSets(h_device, 4, descWrite, 0, nullptr);
}

void Renderer::createPipelines()
{
    VkPipelineCacheCreateInfo pipelineCacheInfo;
    memset(&pipelineCacheInfo, 0, sizeof(pipelineCacheInfo));
    pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VkResult err = h_pfn->vkCreatePipelineCache(h_device, &pipelineCacheInfo, nullptr, &m_pipelineCache);
    check_vk_result(err, "Create Pipeline Cache");

    // First, create the pipeline layouts that will be used by the graphics pipelines
    createPointPipelineLayout();

    // Create all the pipelines defined
    std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now();
    createGraphicPipelines();
    double time_pipelines = std::chrono::duration<double, std::ratio<1, 1000>>(std::chrono::steady_clock::now() - tp).count();
    Logger::log(LoggerMode::VKLog) << m_pipelines.size() << " pipelines generated in " << time_pipelines << " msecs." << Logger::endl;
}

void Renderer::createPointPipelineLayout()
{
    VkResult err; 

    //+++ Push constant map in the vertex shader +++
    // NAME        | offset | size
    //-------------+--------+-----------------------
    // ptSize      | 0      | 4
    // transparency| 4      | 4
    // contrast    | 8      | 4
    // brightness  | 12     | 4
    // saturation  | 16     | 4
    // luminance   | 20     | 4
    // blending    | 24     | 4
    // rampMin     | 28     | 4
    // rampMax     | 32     | 4
    // rampSteps   | 36     | 4
    // ptColor     | 48     | 12
    //-------------+---------------------------------
    VkPushConstantRange pcr[] =
    {
        {
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            64
        },
        {
            VK_SHADER_STAGE_GEOMETRY_BIT,
            64,
            64
        },
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = sizeof(pcr) / sizeof(pcr[0]);
    pipelineLayoutInfo.pPushConstantRanges = pcr;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_descSetLayout;
    err = h_pfn->vkCreatePipelineLayout(h_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
    check_vk_result(err, "Create Pipeline Layout");

    pipelineLayoutInfo.pSetLayouts = &m_descSetLayout_cb;
    err = h_pfn->vkCreatePipelineLayout(h_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout_cb);
    check_vk_result(err, "Create Pipeline Layout");
}

void Renderer::createGraphicPipelines()
{
    VkPipelineInputAssemblyStateCreateInfo ia = {};
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

    // The viewport and scissor will be set dynamically via vkCmdSetViewport/Scissor.
    // This way the pipeline does not need to be touched when resizing the window.
    VkPipelineViewportStateCreateInfo vp = {};
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    vp.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rs = {};
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.polygonMode = VK_POLYGON_MODE_POINT; // {FILL, LINE, POINT}
    rs.cullMode = VK_CULL_MODE_NONE; // we want the back face as well
    rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rs.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo ms = {};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.sampleShadingEnable = VK_FALSE;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    std::unordered_map<BlendMode, VkPipelineDepthStencilStateCreateInfo> depthStencil;
    depthStencil[BlendMode::Opaque].sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil[BlendMode::Opaque].depthTestEnable = VK_TRUE;
    depthStencil[BlendMode::Opaque].depthWriteEnable = VK_TRUE;
    depthStencil[BlendMode::Opaque].depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    depthStencil[BlendMode::Transparent].sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil[BlendMode::Transparent].depthTestEnable = VK_FALSE;
    depthStencil[BlendMode::Transparent].depthWriteEnable = VK_FALSE;

    // ----------------------------------------------------------------------

    VkVertexInputAttributeDescription vertexAttrDescRGB = { 3, 1, VK_FORMAT_R8G8B8A8_UINT, 0 };
    VkVertexInputAttributeDescription vertexDescI = { 2, 1, VK_FORMAT_R8_UINT, 0 };

    VkPipelineShaderStageCreateInfo fragStageInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        m_shaderFrag.module(),
        "main",
        nullptr
    };

    VkPipelineColorBlendAttachmentState cb_opaque = {};
    // no blend, write out all of rgba
    cb_opaque.colorWriteMask = 0xF;

    VkPipelineColorBlendAttachmentState cb_transparent = {
        VK_TRUE,                             // blendEnable
        VK_BLEND_FACTOR_ONE,           // srcColorBlendFactor
        VK_BLEND_FACTOR_ONE,                 // dstColorBlendFactor
        VK_BLEND_OP_ADD,                     // colorBlendOp
        VK_BLEND_FACTOR_ONE, // srcAlphaBlendFactor
        VK_BLEND_FACTOR_ONE,                 // dstAlphaBlendFactor
        VK_BLEND_OP_ADD,                     // alphaBlendOp
        0xF                                  // colorWriteMask
    };

    std::unordered_map<BlendMode, VkPipelineColorBlendStateCreateInfo> colorBlend;
    colorBlend[BlendMode::Opaque].sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlend[BlendMode::Opaque].attachmentCount = 1;
    colorBlend[BlendMode::Opaque].pAttachments = &cb_opaque;
    colorBlend[BlendMode::Transparent].sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlend[BlendMode::Transparent].attachmentCount = 1;
    colorBlend[BlendMode::Transparent].pAttachments = &cb_transparent;

    VkDynamicState dynEnable[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dyn;
    memset(&dyn, 0, sizeof(dyn));
    dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn.dynamicStateCount = sizeof(dynEnable) / sizeof(VkDynamicState);
    dyn.pDynamicStates = dynEnable;

    for (int renderMode = 0; renderMode < RenderMode::RenderMode_MaxEnum; ++renderMode)
    {
        for (int format = 0; format < tls::PointFormat::TL_POINT_MAX_ENUM; ++format)
        {
            std::vector<VkVertexInputBindingDescription> vertexBindingDesc;
            std::vector<VkVertexInputAttributeDescription> vertexAttrDesc;

            vertexBindingDesc.push_back({
                0, // binding
                3 * sizeof(uint16_t),
                VK_VERTEX_INPUT_RATE_VERTEX });

            vertexBindingDesc.push_back({
                2,
                4 * sizeof(float),
                VK_VERTEX_INPUT_RATE_INSTANCE });

            // ---------------------------------
            // posXY
            vertexAttrDesc.push_back({
                0, // location
                0, // binding
                VK_FORMAT_R16G16_UINT,
                0 });
            // posZ
            vertexAttrDesc.push_back({
                1,
                0,
                VK_FORMAT_R16_UINT,
                2 * sizeof(uint16_t) });

            // origin
            vertexAttrDesc.push_back({
                5,
                2,
                VK_FORMAT_R32G32B32_SFLOAT,
                0 });
            // coord precision
            vertexAttrDesc.push_back({
                6,
                2,
                VK_FORMAT_R32_SFLOAT,
                3 * sizeof(float) });

            if (renderMode == RenderMode::Flat)
                goto gt_noColor;

            if (format == tls::PointFormat::TL_POINT_XYZ_I_RGB &&
                renderMode == RenderMode::IntensityRGB_Combined)
            {
                vertexAttrDesc.push_back(vertexAttrDescRGB);
                vertexBindingDesc.push_back({ 1, 3 * sizeof(uint8_t), VK_VERTEX_INPUT_RATE_VERTEX });

                vertexAttrDesc.push_back(vertexDescI);
                vertexBindingDesc.push_back({ 3, sizeof(uint8_t), VK_VERTEX_INPUT_RATE_VERTEX });
            }
            else if (format == tls::PointFormat::TL_POINT_XYZ_RGB ||
                (format == tls::PointFormat::TL_POINT_XYZ_I_RGB && (renderMode == RenderMode::RGB)))
            {
                vertexAttrDesc.push_back(vertexAttrDescRGB);
                vertexBindingDesc.push_back({ 1, 3 * sizeof(uint8_t), VK_VERTEX_INPUT_RATE_VERTEX });
            }
            else if (format == tls::PointFormat::TL_POINT_XYZ_I ||
                format == tls::PointFormat::TL_POINT_XYZ_I_RGB)
            {
                vertexAttrDesc.push_back(vertexDescI);
                vertexBindingDesc.push_back({ 1, sizeof(uint8_t), VK_VERTEX_INPUT_RATE_VERTEX });
            }

            // DÃ©dicace Ã  Alexis
        gt_noColor:

            VkPipelineVertexInputStateCreateInfo vertexInputInfo;
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputInfo.pNext = nullptr;
            vertexInputInfo.flags = 0;
            vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)vertexBindingDesc.size();
            vertexInputInfo.pVertexBindingDescriptions = vertexBindingDesc.data();
            vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)vertexAttrDesc.size();
            vertexInputInfo.pVertexAttributeDescriptions = vertexAttrDesc.data();

            std::string VS_key = getShaderKey((RenderMode)renderMode, (tls::PointFormat)format, false);
            std::string VScb_key = getShaderKey((RenderMode)renderMode, (tls::PointFormat)format, true);

            Shader* vertShader = nullptr;
            Shader* vertShader_cb = nullptr;

            if (m_shadersByFiles.find(VS_key) == m_shadersByFiles.end() ||
                m_shadersByFiles.find(VScb_key) == m_shadersByFiles.end())
            {
                VKLOG << "ERROR: Cannot initialize the pipeline, a shader is missing !" << Logger::endl;
                assert(0);
            }
            else
            {
                vertShader = &m_shadersByFiles.at(VS_key);
                vertShader_cb = &m_shadersByFiles.at(VScb_key);
            }

            VkPipelineShaderStageCreateInfo vertStageInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                nullptr,
                0,
                VK_SHADER_STAGE_VERTEX_BIT,
                vertShader->module(),
                "main",
                nullptr
            };

            VkPipelineShaderStageCreateInfo vertStageInfo_cb = {
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                nullptr,
                0,
                VK_SHADER_STAGE_VERTEX_BIT,
                vertShader_cb->module(),
                "main",
                nullptr
            };

            VkPipelineShaderStageCreateInfo geomClipStageInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                nullptr,
                0,
                VK_SHADER_STAGE_GEOMETRY_BIT,
                m_vertexShadersSrc.at((RenderMode)renderMode).at((tls::PointFormat)format).standardClipping ? m_shaderGeomClip.module() : m_shaderGeomColor.module(),
                "main",
                nullptr
            };

            VkPipelineShaderStageCreateInfo stages[2][3] = {
                { vertStageInfo, fragStageInfo },
                { vertStageInfo_cb, fragStageInfo, geomClipStageInfo },
            };

            // Add the geometry stage in the pipeline creation infos
            for (int clipMode = 0; clipMode < ClippingMode::ClipMaxEnum; clipMode++)
            {
                uint32_t stageCount = 3;
                VkPipelineLayout layout = m_pipelineLayout_cb;
                if (clipMode == ClippingMode::Deactivated)
                {
                    stageCount = 2;
                    layout = m_pipelineLayout;
                }

                // Graphics pipeline creation
                for (int blendMode = 0; blendMode < (int)BlendMode::MaxEnum; blendMode++)
                {
                    for (auto renderPass : h_renderPasses)
                    {
                        VkGraphicsPipelineCreateInfo info = {
                            VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                            nullptr,							// pNext
                            0,									// flags
                            stageCount,							// stageCount
                            stages[clipMode],					// pStages
                            &vertexInputInfo,					// pVertexInputState
                            &ia,                                // pInputAssembleState
                            nullptr,							// pTesselationState
                            &vp,								// pViewportState
                            &rs,                                // pRasterizationState
                            &ms,								// pMultisampleState
                            &depthStencil[BlendMode(blendMode)], // pDepthStencilState
                            &colorBlend[BlendMode(blendMode)],   // pColorBlendState
                            &dyn,								// pDynamicState
                            layout,                             // layout
                            renderPass.second,					// renderPass
                            0,									// subpass
                            VK_NULL_HANDLE,						// basePipelineHandle
                            0									// basePipelineIndex
                        };

                        VkPipeline pipe = VK_NULL_HANDLE;
                        VkResult err = h_pfn->vkCreateGraphicsPipelines(h_device, m_pipelineCache, 1, &info, nullptr, &pipe);
                        check_vk_result(err, "Create Graphic Pipeline (points)");
                        m_pipelines[createPipeKey(BlendMode(blendMode), RenderMode(renderMode), ClippingMode(clipMode), tls::PointFormat(format), renderPass.first)] = pipe;
                    }
                }
            }
        }
    }
}

void Renderer::bindVertexBuffers(VkCommandBuffer _cmdBuffer, RenderMode _renderMode, tls::PointFormat _format, const TlCellDrawInfo& _cellDrawInfo) const
{
    // Bind Buffer for vertex
    VkDeviceSize nullOffset = 0;
    h_pfn->vkCmdBindVertexBuffers(_cmdBuffer, 0, 1, &_cellDrawInfo.buffer, &nullOffset);

    switch (_format)
    {
    case tls::TL_POINT_XYZ_I:
    {
        VkDeviceSize iOffset = _cellDrawInfo.m_iOffset;
        h_pfn->vkCmdBindVertexBuffers(_cmdBuffer, 1, 1, &_cellDrawInfo.buffer, &iOffset);
        break;
    }
    case tls::TL_POINT_XYZ_RGB:
    {
        VkDeviceSize rgbOffset = _cellDrawInfo.m_rgbOffset;
        h_pfn->vkCmdBindVertexBuffers(_cmdBuffer, 1, 1, &_cellDrawInfo.buffer, &rgbOffset);
        break;
    }
    case tls::TL_POINT_XYZ_I_RGB:
    {
        if (_renderMode == RenderMode::IntensityRGB_Combined)
        {
            VkDeviceSize rgbOffset = _cellDrawInfo.m_rgbOffset;
            h_pfn->vkCmdBindVertexBuffers(_cmdBuffer, 1, 1, &_cellDrawInfo.buffer, &rgbOffset);
            VkDeviceSize iOffset = _cellDrawInfo.m_iOffset;
            h_pfn->vkCmdBindVertexBuffers(_cmdBuffer, 3, 1, &_cellDrawInfo.buffer, &iOffset);
        }
        else if (_renderMode == RenderMode::RGB)
        {
            VkDeviceSize rgbOffset = _cellDrawInfo.m_rgbOffset;
            h_pfn->vkCmdBindVertexBuffers(_cmdBuffer, 1, 1, &_cellDrawInfo.buffer, &rgbOffset);
        }
        else
        {
            VkDeviceSize iOffset = _cellDrawInfo.m_iOffset;
            h_pfn->vkCmdBindVertexBuffers(_cmdBuffer, 1, 1, &_cellDrawInfo.buffer, &iOffset);
        }
        break;
    }
    default:
        break;
    }
}

// Must be called inside a renderpass
void Renderer::setViewportAndScissor(int32_t _xPos, int32_t _yPos, uint32_t _width, uint32_t _height, VkCommandBuffer _cmdBuffer)
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

void Renderer::drawPoints(const TlScanDrawInfo &_drawInfo, const VkUniformOffset& viewProjUni, RenderMode _renderMode, VkCommandBuffer _cmdBuffer, BlendMode blendMode, VkFormat renderFormat) const
{
    // Bind the right pipeline
    PipelineKey pipelineDef = createPipeKey(blendMode, _renderMode, ClippingMode::Deactivated, _drawInfo.format, renderFormat);
#ifdef _DEBUG_
    if (m_pipelines.find(pipelineDef) == m_pipelines.end())
        assert(0 && "Pipeline is not initialized");
#endif

    h_pfn->vkCmdBindPipeline(_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelines.at(pipelineDef));

    // Bind buffer for instance
    VkDeviceSize offset = 0;
    h_pfn->vkCmdBindVertexBuffers(_cmdBuffer, 2, 1, &_drawInfo.instanceBuffer, &offset);

    // Bind the descriptor set for the scan
    //Note (AurÃ©lien) Quick fix for getting ramp working
    uint32_t offsets[] = { viewProjUni, _drawInfo.modelUni, };
    h_pfn->vkCmdBindDescriptorSets(_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descSet, 2, offsets);

    for (const TlCellDrawInfo& cellDrawInfo : _drawInfo.cellDrawInfo)
    {
        bindVertexBuffers(_cmdBuffer, _renderMode, _drawInfo.format, cellDrawInfo);
        
        // Draw
        h_pfn->vkCmdDraw(_cmdBuffer, cellDrawInfo.vertexCount, 1, 0, cellDrawInfo.cellIndex);
    }
}

void Renderer::drawPointsClipping(const TlScanDrawInfo &_drawInfo, const VkUniformOffset& viewProjUni, const VkUniformOffset& clippingUni, RenderMode _renderMode, VkCommandBuffer _cmdBuffer, BlendMode blendMode, VkFormat renderFormat) const
{
    // Bind the right pipeline
    PipelineKey pipelineDef = createPipeKey(blendMode, _renderMode, ClippingMode::Activated, _drawInfo.format, renderFormat);

    assert((m_pipelines.find(pipelineDef) != m_pipelines.end()) && "Pipeline is not initialized");

    h_pfn->vkCmdBindPipeline(_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelines.at(pipelineDef));

    // Bind buffer for instance
    VkDeviceSize offset = 0;
    h_pfn->vkCmdBindVertexBuffers(_cmdBuffer, 2, 1, &_drawInfo.instanceBuffer, &offset);

    // Bind the descriptor set for the scan
    uint32_t offsets[] = { viewProjUni, _drawInfo.modelUni, viewProjUni, clippingUni };
    h_pfn->vkCmdBindDescriptorSets(_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout_cb, 0, 1, &m_descSet_cb, 4, offsets);

    bool showWarningMsg = false;
    uint32_t maxConcurrentClipping = 0;
    for (const TlCellDrawInfo_multiCB& cellDraw : _drawInfo.cellDrawInfoCB)
    {
        TlCellDrawInfo castCellDraw{ cellDraw.buffer, cellDraw.cellIndex, cellDraw.vertexCount, cellDraw.m_iOffset, cellDraw.m_rgbOffset };
        bindVertexBuffers(_cmdBuffer, _renderMode, _drawInfo.format, castCellDraw);

        // Prepare the clipping indexes for the geometry shader
        ClippingGpuId clippingIndexes[MAX_CLIPPING_PER_CELL + 1];
        uint32_t cbCount = (uint32_t)cellDraw.clippingGpuIds.size();

        if (cbCount > MAX_CLIPPING_PER_CELL)
        {
            showWarningMsg = true;
            maxConcurrentClipping = std::max(cbCount, maxConcurrentClipping);
            cbCount = MAX_CLIPPING_PER_CELL;
        }
        clippingIndexes[0] = (uint16_t)cbCount;
        memcpy(clippingIndexes + 1, cellDraw.clippingGpuIds.data(), sizeof(ClippingGpuId) * cbCount);
        setClippingIndexes(clippingIndexes, _cmdBuffer);

        // Draw
        h_pfn->vkCmdDraw(_cmdBuffer, cellDraw.vertexCount, 1, 0, cellDraw.cellIndex);
    }

    // To generate less log
    if (showWarningMsg)
    {
        VKLOG << "WARNING: Too much clippings active on the same cell detected: " << maxConcurrentClipping << Logger::endl;
    }
}

void Renderer::setConstantPointSize(float ptSize, VkCommandBuffer _cmdBuffer)
{
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, POINT_STAGE_FLAGS, 0, 4, &ptSize);

    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout_cb, POINT_STAGE_FLAGS, 0, 4, &ptSize);
}

void Renderer::setConstantContrastBrightness(float contrast, float brightness, VkCommandBuffer _cmdBuffer)
{
    // NOTE(robin) - We transform the contrast and brightness in an other space to reduce computation in the vertex shader
    float contrast_vs = (259.f * (contrast + 255.f)) / (255.f * (259.f - contrast));
    float brightness_vs = brightness / 100.f - 0.5f;

    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, POINT_STAGE_FLAGS, 8, 4, &contrast_vs);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, POINT_STAGE_FLAGS, 12, 4, &brightness_vs);

    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout_cb, POINT_STAGE_FLAGS, 8, 4, &contrast_vs);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout_cb, POINT_STAGE_FLAGS, 12, 4, &brightness_vs);
}

void Renderer::setConstantSaturationLuminance(float saturation, float luminance, VkCommandBuffer _cmdBuffer)
{
    float fluminance = (luminance + 50.f) / 50.f;
    float fsaturation = (saturation + 50.f) / 50.f;

    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, POINT_STAGE_FLAGS, 16, 4, &fsaturation);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, POINT_STAGE_FLAGS, 20, 4, &fluminance);

    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout_cb, POINT_STAGE_FLAGS, 16, 4, &fsaturation);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout_cb, POINT_STAGE_FLAGS, 20, 4, &fluminance);
}

void Renderer::setConstantBlending(float blending, VkCommandBuffer _cmdBuffer)
{
    float fblending = blending / 200.f;

    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, POINT_STAGE_FLAGS, 24, 4, &fblending);

    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout_cb, POINT_STAGE_FLAGS, 24, 4, &fblending);
}

void Renderer::setConstantRampDistance(float min, float max, int steps, VkCommandBuffer _cmdBuffer)
{
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, POINT_STAGE_FLAGS, 28, 4, &min);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, POINT_STAGE_FLAGS, 32, 4, &max);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, POINT_STAGE_FLAGS, 36, 4, &steps);

    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout_cb, POINT_STAGE_FLAGS, 28, 4, &min);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout_cb, POINT_STAGE_FLAGS, 32, 4, &max);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout_cb, POINT_STAGE_FLAGS, 36, 4, &steps);
}

void Renderer::setConstantPtColor(const glm::vec3& color, VkCommandBuffer _cmdBuffer)
{
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, POINT_STAGE_FLAGS, 48, 12, &color[0]);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout_cb, POINT_STAGE_FLAGS, 48, 12, &color[0]);
}

void Renderer::setConstantBillboard(bool enabled, float feather, VkCommandBuffer _cmdBuffer)
{
    float enableFlag = enabled ? 1.0f : 0.0f;
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, POINT_STAGE_FLAGS, 40, 4, &enableFlag);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout, POINT_STAGE_FLAGS, 44, 4, &feather);

    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout_cb, POINT_STAGE_FLAGS, 40, 4, &enableFlag);
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout_cb, POINT_STAGE_FLAGS, 44, 4, &feather);
}

void Renderer::setClippingIndexes(const ClippingGpuId indexes[MAX_CLIPPING_PER_CELL + 1], VkCommandBuffer _cmdBuffer) const
{
    // NOTE - ClippingGpuId est maintenant codÃ© sur 2 octet. (uint16). On peut donc en mettre 32 dans 64 octets.
    h_pfn->vkCmdPushConstants(_cmdBuffer, m_pipelineLayout_cb, VK_SHADER_STAGE_GEOMETRY_BIT, 64, 64, indexes);
}

void Renderer::cleanup()
{
    //*** Common Resources ***
    if (m_pipelineCache) {
        h_pfn->vkDestroyPipelineCache(h_device, m_pipelineCache, nullptr);
        m_pipelineCache = VK_NULL_HANDLE;
    }

    if (m_descSetLayout) {
        h_pfn->vkDestroyDescriptorSetLayout(h_device, m_descSetLayout, nullptr);
        m_descSetLayout = VK_NULL_HANDLE;
    }

    if (m_descSetLayout_cb) {
        h_pfn->vkDestroyDescriptorSetLayout(h_device, m_descSetLayout_cb, nullptr);
        m_descSetLayout_cb = VK_NULL_HANDLE;
    }

    if (m_pipelineLayout) {
        h_pfn->vkDestroyPipelineLayout(h_device, m_pipelineLayout, nullptr);
        m_pipelineLayout = VK_NULL_HANDLE;
    }

    if (m_pipelineLayout_cb) {
        h_pfn->vkDestroyPipelineLayout(h_device, m_pipelineLayout_cb, nullptr);
        m_pipelineLayout_cb = VK_NULL_HANDLE;
    }

    //*** Shaders ***
    for (auto& iterator : m_shadersByFiles)
        iterator.second.reset();

    //*** Pipelines ***
    for (auto& iterator : m_pipelines)
    {
        if (iterator.second) {
            h_pfn->vkDestroyPipeline(h_device, iterator.second, nullptr);
            iterator.second = VK_NULL_HANDLE;
        }
    }
    m_pipelines.clear();
}