#include "vulkan/vulkan.h"
#include "vulkan/VulkanFunctions.h"

namespace tls
{
    namespace vk
    {
        void destroyImageView(VulkanDeviceFunctions& pfnDev, VkDevice& device, VkImageView& _imageView);
        void destroyImage(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkImage& _image);
        void destroyBuffer(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkBuffer& _buffer);
        void destroyCommandPool(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkCommandPool& _cmdPool);
        void destroyDescriptorPool(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkDescriptorPool& _pool);
        void destroyDescriptorSetLayout(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkDescriptorSetLayout& _layout);

        void destroyPipeline(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkPipeline& _pipeline);
        void destroyPipelineCache(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkPipelineCache& _cache);
        void destroyPipelineLayout(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkPipelineLayout& _layout);

        void destroyRenderPass(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkRenderPass& _renderPass);
        void destroyFramebuffer(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkFramebuffer& _framebuffer);
        void destroySwapchain(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkSwapchainKHR& _swapchain);
        void destroyFence(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkFence& _fence);
        void destroySemaphore(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkSemaphore& _semaphore);
        void destroySampler(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkSampler& _sampler);

        void freeCommandBuffer(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkCommandPool& _cmdPool, VkCommandBuffer& _cmdBuffer);
        void freeMemory(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkDeviceMemory& _memory);
        void freeDescriptorSet(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkDescriptorPool& _pool, VkDescriptorSet& _set);
        void destroyMultiImageView(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkImageView*& _pImageViews, uint32_t& _count);

    }
}
