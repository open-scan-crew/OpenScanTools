#include "vulkan/VulkanHelperFunctions.h"

void tls::vk::destroyImageView(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkImageView& _imageView)
{
    if (_imageView) {
        _pfnDev.vkDestroyImageView(_device, _imageView, nullptr);
        _imageView = VK_NULL_HANDLE;
    }
}

void tls::vk::destroyImage(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkImage& _image)
{
    if (_image) {
        _pfnDev.vkDestroyImage(_device, _image, nullptr);
        _image = VK_NULL_HANDLE;
    }
}

void tls::vk::destroyBuffer(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkBuffer& _buffer)
{
    if (_buffer) {
        _pfnDev.vkDestroyBuffer(_device, _buffer, nullptr);
        _buffer = VK_NULL_HANDLE;
    }
}

void tls::vk::destroyCommandPool(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkCommandPool& _cmdPool)
{
    if (_cmdPool) {
        _pfnDev.vkDestroyCommandPool(_device, _cmdPool, nullptr);
        _cmdPool = VK_NULL_HANDLE;
    }
}

void tls::vk::destroyDescriptorSetLayout(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkDescriptorSetLayout& _layout)
{
    if (_layout) {
        _pfnDev.vkDestroyDescriptorSetLayout(_device, _layout, nullptr);
        _layout = VK_NULL_HANDLE;
    }
}

void tls::vk::destroyDescriptorPool(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkDescriptorPool& _pool)
{
    if (_pool) {
        _pfnDev.vkDestroyDescriptorPool(_device, _pool, nullptr);
        _pool = VK_NULL_HANDLE;
    }
}

void tls::vk::destroyPipeline(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkPipeline& _pipeline)
{
    if (_pipeline) {
        _pfnDev.vkDestroyPipeline(_device, _pipeline, nullptr);
        _pipeline = VK_NULL_HANDLE;
    }
}

void tls::vk::destroyPipelineCache(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkPipelineCache& _cache)
{
    if (_cache) {
        _pfnDev.vkDestroyPipelineCache(_device, _cache, nullptr);
        _cache = VK_NULL_HANDLE;
    }
}

void tls::vk::destroyPipelineLayout(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkPipelineLayout& _layout)
{
    if (_layout) {
        _pfnDev.vkDestroyPipelineLayout(_device, _layout, nullptr);
        _layout = VK_NULL_HANDLE;
    }
}

void tls::vk::destroyRenderPass(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkRenderPass& _renderPass)
{
    if (_renderPass) {
        _pfnDev.vkDestroyRenderPass(_device, _renderPass, nullptr);
        _renderPass = VK_NULL_HANDLE;
    }
}

void tls::vk::destroyFramebuffer(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkFramebuffer& _framebuffer)
{
    if (_framebuffer) {
        _pfnDev.vkDestroyFramebuffer(_device, _framebuffer, nullptr);
        _framebuffer = VK_NULL_HANDLE;
    }
}

void tls::vk::freeCommandBuffer(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkCommandPool& _cmdPool, VkCommandBuffer& _cmdBuffer)
{
    if (_cmdBuffer) {
        _pfnDev.vkFreeCommandBuffers(_device, _cmdPool, 1, &_cmdBuffer);
        _cmdBuffer = VK_NULL_HANDLE;
    }
}

void tls::vk::freeMemory(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkDeviceMemory& _memory)
{
    if (_memory) {
        _pfnDev.vkFreeMemory(_device, _memory, nullptr);
        _memory = VK_NULL_HANDLE;
    }
}

void tls::vk::destroySwapchain(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkSwapchainKHR& _swapchain)
{
    if (_swapchain) {
        _pfnDev.vkDestroySwapchainKHR(_device, _swapchain, nullptr);
        _swapchain = VK_NULL_HANDLE;
    }
}

void tls::vk::destroyFence(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkFence& _fence)
{
    if (_fence) {
        _pfnDev.vkDestroyFence(_device, _fence, nullptr);
        _fence = VK_NULL_HANDLE;
    }
}

void tls::vk::destroySemaphore(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkSemaphore& _semaphore)
{
    if (_semaphore) {
        _pfnDev.vkDestroySemaphore(_device, _semaphore, nullptr);
        _semaphore = VK_NULL_HANDLE;
    }
}


void tls::vk::destroySampler(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkSampler& _sampler)
{
    if (_sampler)
    {
        _pfnDev.vkDestroySampler(_device, _sampler, nullptr);
        _sampler = VK_NULL_HANDLE;
    }
}

void tls::vk::freeDescriptorSet(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkDescriptorPool& _pool, VkDescriptorSet& _set)
{
    if (_set)
    {
        _pfnDev.vkFreeDescriptorSets(_device, _pool, 1, &_set);
        _set = VK_NULL_HANDLE;
    }
}

void tls::vk::destroyMultiImageView(VulkanDeviceFunctions& _pfnDev, VkDevice& _device, VkImageView*& _pImageViews, uint32_t& _count)
{
    if (_pImageViews) {
        for (uint32_t i = 0; i < _count; i++)
            destroyImageView(_pfnDev, _device, _pImageViews[i]);
        delete[] _pImageViews;
        _pImageViews = nullptr;
    }
}