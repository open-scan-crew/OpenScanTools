#ifndef RENDERER_H
#define RENDERER_H

#include "vulkan/Shader.h"
#include "vulkan/vulkan.h"
#include "vulkan/VkUniform.h"
#include "pointCloudEngine/PCE_graphics.h"
#include "pointCloudEngine/RenderingTypes.h"
#include "pointCloudEngine/RenderingLimits.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <unordered_map>
#include <string>

class VulkanDeviceFunctions;

class Renderer
{
public:
	enum ShaderType { Vertex, Geometry, Fragment };
    enum ClippingMode { Deactivated, Activated, ClipMaxEnum };

	struct ShaderDefinition
	{
		ShaderDefinition(const ShaderType& type, const std::string& file, const std::string& entryPoint="main");
        std::string getStringKey() const { return (_file + "##" + _entryPoint); };
		ShaderType  _type;
		std::string _file;
		std::string _entryPoint;
	};
	
	struct RenderingDefinition
    {
        RenderingDefinition(const RenderMode& renderMode, const tls::PointFormat& pointFormat);
        RenderMode _renderMode;
        tls::PointFormat _format;
    };
	

public:
    Renderer();
    ~Renderer();

    void setViewportAndScissor(int32_t _xPos, int32_t _yPos, uint32_t _width, uint32_t height, VkCommandBuffer _cmdBuffer);

    void drawPoints(const TlScanDrawInfo &drawInfo, const VkUniformOffset& viewProjUni, RenderMode _renderMode, VkCommandBuffer _cmdBuffer, BlendMode blendMode, VkFormat renderFormat) const;
    void drawPointsClipping(const TlScanDrawInfo &drawInfo, const VkUniformOffset& viewProjUni, const VkUniformOffset& clippingBoxUni, RenderMode _renderMode, VkCommandBuffer _cmdBuffer, BlendMode blendMode, VkFormat renderFormat) const;

    void setConstantPointSize(float ptSize, VkCommandBuffer _cmdBuffer);
    void setConstantContrastBrightness(float contrast, float brightness, VkCommandBuffer _cmdBuffer);
        void setConstantSaturationLuminance(float saturation, float luminance, VkCommandBuffer _cmdBuffer);
    void setConstantBlending(float blending, VkCommandBuffer _cmdBuffer);
    void setConstantRampDistance(float min, float max, int steps, VkCommandBuffer _cmdBuffer);
        void setConstantPtColor(const glm::vec3& color, VkCommandBuffer _cmdBuffer);
    void setConstantBillboard(bool enabled, float feather, VkCommandBuffer _cmdBuffer);
    void setClippingIndexes(const ClippingGpuId indexes[MAX_CLIPPING_PER_CELL + 1], VkCommandBuffer _cmdBuffer) const;

    void cleanup();

private:
    void createShaders();
    void createDescriptorSetLayout();
    void createDescriptorSets();
    void createPipelines();

    void createPointPipelineLayout();
    void createGraphicPipelines();

    void bindVertexBuffers(VkCommandBuffer, RenderMode, tls::PointFormat, const TlCellDrawInfo&) const;

    std::string getShaderKey(RenderMode, tls::PointFormat, bool clip) const;

    // Resources NOT owned by the Renderer --> DO NOT DESTROY HERE
    VkDevice h_device = VK_NULL_HANDLE;
    VulkanDeviceFunctions* h_pfn = nullptr;
    VkDescriptorPool h_descPool = VK_NULL_HANDLE;
    std::unordered_map<VkFormat, VkRenderPass> h_renderPasses;

    // Resources owned by the Renderer
    //--- Common Resources ---
    VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;

    VkDescriptorSetLayout m_descSetLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descSetLayout_cb = VK_NULL_HANDLE;
    VkDescriptorSet m_descSet;
    VkDescriptorSet m_descSet_cb;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout_cb = VK_NULL_HANDLE;

    //--- Quantized Point Resources ---
	std::unordered_map<std::string, Shader> m_shadersByFiles;
    struct SrcVertexShader
    {
        const uint32_t* noClip;
        size_t noClipSize;
        const uint32_t* withClip;
        size_t withClipSize;
        bool standardClipping;
    };
    std::unordered_map<RenderMode, std::unordered_map<tls::PointFormat, SrcVertexShader>> m_vertexShadersSrc;
    Shader m_shaderFrag;
    Shader m_shaderGeomClip;
    Shader m_shaderGeomColor;

    typedef size_t PipelineKey;
    inline PipelineKey createPipeKey(BlendMode dm, RenderMode rm, ClippingMode cm, tls::PointFormat pf, VkFormat rdFormat) const
    {
        // bit mask sizes
        constexpr size_t DRAW_MODE_BMS = 8;
        constexpr size_t RENDER_MODE_BMS = 8;
        constexpr size_t CLIP_MODE_BMS = 8;
        constexpr size_t POINT_FORMAT_BMS = 8;
        constexpr size_t RENDER_FORMAT_BMS = 16;

        // bit mask offsets
        constexpr size_t DRAW_MODE_BMO = 0;
        constexpr size_t RENDER_MODE_BMO = DRAW_MODE_BMO + DRAW_MODE_BMS;
        constexpr size_t CLIP_MODE_BMO = RENDER_MODE_BMO + RENDER_MODE_BMS;
        constexpr size_t POINT_FORMAT_BMO = CLIP_MODE_BMO + CLIP_MODE_BMS;
        constexpr size_t RENDER_FORMAT_BMO = POINT_FORMAT_BMO + POINT_FORMAT_BMS;
        constexpr size_t ALL_BMO = RENDER_FORMAT_BMO + RENDER_FORMAT_BMS;

        assert((size_t)dm < (1 << DRAW_MODE_BMS) && "BlendMode too large for the key");
        assert((size_t)rm < (1 << RENDER_MODE_BMS) && "RenderMode too large for the key");
        assert((size_t)cm < (1 << CLIP_MODE_BMS) && "ClippingMode too large for the key");
        assert((size_t)pf < (1 << POINT_FORMAT_BMS) && "PointFormat too large for the key");
        assert((size_t)rdFormat < (1 << RENDER_FORMAT_BMS) && "VkFormat too large for the key");

        return ((size_t)dm + ((size_t)rm << RENDER_MODE_BMO) + ((size_t)cm << CLIP_MODE_BMO) + ((size_t)pf << POINT_FORMAT_BMO) + ((size_t)rdFormat << RENDER_FORMAT_BMO));
    };

    //std::unordered_map<BlendMode, std::unordered_map<RenderMode, std::unordered_map<ClippingMode, std::unordered_map<tls::PointFormat, VkPipeline>>>> m_pipelines;
    std::unordered_map<PipelineKey, VkPipeline> m_pipelines;
};

// NOTE(robin)  - Solution if we want to use a std::hash for the pipeline map
/*
struct PipeDef {
    Renderer::BlendMode drawMode;
    RenderMode renderMode;
    Renderer::ClippingMode clippingMode;
    tls::PointFormat pointFormat;
};

template<> struct std::hash<PipeDef>
{
    std::size_t operator()(PipeDef const& def) const noexcept
    {
        std::size_t h1 = std::hash<Renderer::BlendMode>{}(def.drawMode);
        std::size_t h2 = std::hash<RenderMode>{}(def.renderMode);
        std::size_t h3 = std::hash<Renderer::ClippingMode>{}(def.clippingMode);
        std::size_t h4 = std::hash<tls::PointFormat>{}(def.pointFormat);
        return (h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3)); // or use boost::hash_combine
    }
};
*/

#endif // _RENDERER_H_