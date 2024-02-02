#ifndef SHADER_H_
#define SHADER_H_

#include "vulkan/vulkan.h"
#include "vulkan/VulkanFunctions.h"

#include <vector>
#include <string>

class VulkanDeviceFunctions;

class Shader
{
public:
    ~Shader();
    //void load(VkDevice dev, VulkanDeviceFunctions* pfn, const EShader& shader);
    void load(VkDevice dev, VulkanDeviceFunctions* pfn, const char* path, const char* entryPoint, const std::vector<std::string>& preProcessorDef = {});
    void createModule(VkDevice dev, VulkanDeviceFunctions* pfn, const uint32_t* binaryCode, size_t codeSize);

    VkShaderModule module() const;
    VkPipelineShaderStageCreateInfo getPipelineInfo(VkShaderStageFlagBits stage, const char* entrypoint);

    bool isValid() const { return module() != VK_NULL_HANDLE; }
    void reset();


private:
    VkShaderModule m_shaderModule = VK_NULL_HANDLE;
    VkDevice h_device = VK_NULL_HANDLE;
    VulkanDeviceFunctions* h_pfn = nullptr;
};

#endif