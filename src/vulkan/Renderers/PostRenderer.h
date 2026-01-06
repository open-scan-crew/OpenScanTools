#ifndef POST_RENDERER_H
#define POST_RENDERER_H

#include "vulkan/Shader.h"
#include "vulkan/vulkan.h"
#include "vulkan/VkUniform.h"
#include "pointCloudEngine/RenderingTypes.h"
#include "utils/Color32.hpp"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <vector>

class VulkanDeviceFunctions;
class VulkanManager;

class PostRenderer
{
public:
    PostRenderer();
    ~PostRenderer();

    void setViewportAndScissor(int32_t _xPos, int32_t _yPos, uint32_t _width, uint32_t height, VkCommandBuffer _cmdBuffer);

    void processPointFilling(VkCommandBuffer _cmdBuffer, VkDescriptorSet descSetSamplers, VkDescriptorSet descSetOutput, VkExtent2D extent);
    void processNormalShading(VkCommandBuffer _cmdBuffer, VkUniformOffset matrixUniOffset, VkDescriptorSet descSetColor, VkDescriptorSet descSetOutput, VkExtent2D extent);
    void processNormalColored(VkCommandBuffer _cmdBuffer, VkUniformOffset matrixUniOffset, VkDescriptorSet descSetColor, VkDescriptorSet descSetOutput, VkExtent2D extent);
    void processTransparencyHDR(VkCommandBuffer _cmdBuffer, VkDescriptorSet descSetColor, VkExtent2D extent);
    void processEdgeAwareBlur(VkCommandBuffer _cmdBuffer, const EdgeAwareBlur& blurSettings, VkDescriptorSet descSetColor, VkDescriptorSet descSetDepth, VkExtent2D extent);
    void processDepthLining(VkCommandBuffer _cmdBuffer, const DepthLining& liningSettings, VkDescriptorSet descSetColor, VkDescriptorSet descSetDepth, VkExtent2D extent);
    void processSSAO(VkCommandBuffer _cmdBuffer, VkDescriptorSet descSetAO, VkDescriptorSet descSetDepth, VkExtent2D extent) const;
    void processAOBlur(VkCommandBuffer _cmdBuffer, VkDescriptorSet descSetAO, VkDescriptorSet descSetDepth, VkExtent2D extent, bool horizontal) const;
    void processAOCompose(VkCommandBuffer _cmdBuffer, VkDescriptorSet descSetColor, VkDescriptorSet descSetAO, VkDescriptorSet descSetDepth, float aoStrength, VkExtent2D extent) const;

    // Compatibility helpers used by some build targets
    bool initEdgeAwareBlur(VulkanManager& vkm, uint32_t swapChainImageCount);
    void transitionAndDispatchEdgeAwareBlur(VkCommandBuffer _cmdBuffer, const EdgeAwareBlur& blurSettings, VkDescriptorSet descSetColor, VkDescriptorSet descSetDepth, VkExtent2D extent);
    void setConstantZRange(float nearZ, float farZ, VkCommandBuffer _cmdBuffer);
    void setConstantScreenSize(VkExtent2D screenSize, glm::vec2 pixelSize, VkCommandBuffer _cmdBuffer);
    void setConstantScreenOffset(glm::vec2 offset, VkCommandBuffer cmdBuffer);
    void setConstantProjMode(bool isPerspective, VkCommandBuffer _cmdBuffer);
    void setConstantTexelThreshold(int texelThreshold, VkCommandBuffer _cmdBuffer);
    void setConstantLighting(const PostRenderingNormals& lighting, VkCommandBuffer _cmdBuffer) const;

    void setConstantHDR(float transparency, bool substract, bool noFlash, bool flashAdvanced, float flashControl, VkExtent2D screenSize, Color32 background, VkCommandBuffer _cmdBuffer);

    void cleanup();

private:
    void createShaders();
    void createDescriptorSetLayout();
    void createDescriptorSets();
    void createPipelines();
    void createPipelineLayouts();
    void createFillingPipeline();
    void createNormalPipeline();
    void createEdgeAwarePipeline();
    void createDepthLiningPipeline();
    void createTransparencyHDRPipeline();
    void createAOPipeline();
    void createAOBlurPipeline();
    void createAOComposePipeline();

    void loadShaderSPV(Shader& shader, const std::vector<uint32_t>& code_spv);

private:
    // Resources NOT owned by the Renderer --> DO NOT DESTROY HERE
    VkDevice h_device = VK_NULL_HANDLE;
    VulkanDeviceFunctions* h_pfn = nullptr;
    VkDescriptorPool h_descPool = VK_NULL_HANDLE;

    // Resources owned by the Renderer
    VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;

    VkDescriptorSetLayout m_descSetLayoutMatrixCompute = VK_NULL_HANDLE;
    VkDescriptorSet m_descSetMatrixCompute;

    VkPipelineLayout m_fillingPipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_normalPipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_transparencyHDRPipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_aoPipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_aoComposePipelineLayout = VK_NULL_HANDLE;

    Shader m_fillingCompShader;
    Shader m_normalShadingCompShader;
    Shader m_normalColoredCompShader;
    Shader m_transparencyHDRCompShader;
    Shader m_edgeAwareBlurCompShader;
    Shader m_depthLiningCompShader;
    Shader m_ssaoCompShader;
    Shader m_aoBlurCompShader;
    Shader m_aoComposeCompShader;

    VkPipeline m_fillingPipeline;
    VkPipeline m_normalColoredPipeline;
    VkPipeline m_normalShadingPipeline;
    VkPipeline m_transparencyHDRPipeline;
    VkPipeline m_hdrSubsPipeline;
    VkPipeline m_edgeAwarePipeline;
    VkPipeline m_depthLiningPipeline;
    VkPipeline m_ssaoPipeline = VK_NULL_HANDLE;
    VkPipeline m_aoBlurPipeline = VK_NULL_HANDLE;
    VkPipeline m_aoComposePipeline = VK_NULL_HANDLE;

    VkPipelineLayout m_edgeAwarePipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_depthLiningPipelineLayout = VK_NULL_HANDLE;
};

#endif
