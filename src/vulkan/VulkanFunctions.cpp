#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"

#include "vulkan/VulkanFunctions.h"
#include "vulkan/VulkanPlatform.h"
#include "utils/Logger.h"


#define GET_INSTANCE_PROC_ADDR(inst, entrypoint)                                             \
{                                                                                            \
    vk##entrypoint = (PFN_vk##entrypoint)vkGetInstanceProcAddr(inst, "vk" #entrypoint);      \
    if (vk##entrypoint == NULL) {                                                            \
        Logger::log(LoggerMode::VKLog) << "vkGetInstanceProcAddr failed to find vk" #entrypoint << Logger::endl; \
    }                                                                                        \
}

#define GET_DEVICE_PROC_ADDR(dev, entrypoint)                                                \
{                                                                                            \
    vk##entrypoint = (PFN_vk##entrypoint)vkGetDeviceProcAddr(dev, "vk" #entrypoint);         \
    if (vk##entrypoint == NULL) {                                                            \
        Logger::log(LoggerMode::VKLog) << "vkGetDeviceProcAddr failed to find vk" #entrypoint << Logger::endl; \
    }                                                                                        \
}



VulkanFunctions::VulkanFunctions()
{
    // Initialize vkGetInstanceProcAddr from the right library
    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)m_vulkanLib.resolveFunction("vkGetInstanceProcAddr");

    if (vkGetInstanceProcAddr == NULL) {
        Logger::log(LoggerMode::VKLog) << "ERROR: Vulkan not supported !" << Logger::endl;
        exit(1);
    }

    GET_INSTANCE_PROC_ADDR(NULL, EnumerateInstanceLayerProperties)
    GET_INSTANCE_PROC_ADDR(NULL, EnumerateInstanceExtensionProperties)

    GET_INSTANCE_PROC_ADDR(NULL, EnumerateInstanceVersion)
    GET_INSTANCE_PROC_ADDR(NULL, CreateInstance)
}

void VulkanFunctions::loadInstanceFunctions(VkInstance instance)
{
    h_vkInstance = instance;

    GET_INSTANCE_PROC_ADDR(instance, EnumeratePhysicalDevices)
    GET_INSTANCE_PROC_ADDR(instance, EnumerateDeviceExtensionProperties)
    GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceProperties)
    GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceMemoryProperties)
    GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceQueueFamilyProperties)
    GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceFeatures)
	GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceFeatures2)
    GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceFormatProperties)
    GET_INSTANCE_PROC_ADDR(instance, CreateDevice)
    GET_INSTANCE_PROC_ADDR(instance, DestroyDevice)
    GET_INSTANCE_PROC_ADDR(instance, DestroyInstance)

    GET_INSTANCE_PROC_ADDR(instance, DestroySurfaceKHR)
    GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceSupportKHR)
    GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceCapabilitiesKHR)
    GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceFormatsKHR)
    GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfacePresentModesKHR)

    GET_INSTANCE_PROC_ADDR(instance, CreateDebugUtilsMessengerEXT)
    GET_INSTANCE_PROC_ADDR(instance, DestroyDebugUtilsMessengerEXT)
    GET_INSTANCE_PROC_ADDR(instance, SubmitDebugUtilsMessageEXT)
}

void VulkanFunctions::unloadInstanceFunctions()
{
    h_vkInstance = VK_NULL_HANDLE;
}

PFN_vkGetDeviceProcAddr VulkanFunctions::getDeviceProcAddr(VkDevice device)
{
    return ((PFN_vkGetDeviceProcAddr)vkGetInstanceProcAddr(h_vkInstance, "vkGetDeviceProcAddr"));
}

VkSurfaceKHR VulkanFunctions::createSurfaceForWindow(uint64_t winId)
{
#if defined(_WIN32)
    PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
    GET_INSTANCE_PROC_ADDR(h_vkInstance, CreateWin32SurfaceKHR)

    HINSTANCE hinst = (HINSTANCE)::GetModuleHandle(NULL);

    VkWin32SurfaceCreateInfoKHR surfaceInfo = {
        VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        NULL,
        0,
        hinst,
        (HWND)winId
    };

    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkResult res = vkCreateWin32SurfaceKHR(h_vkInstance, &surfaceInfo, nullptr, &surface);
    if (res != VK_SUCCESS)
    {
        Logger::log(LoggerMode::VKLog) << "ERROR: Failed to create a Vulkan Surface from window id !" << Logger::endl;
    }

    return surface;
#endif
}

uint32_t VulkanFunctions::version()
{
    if (vkEnumerateInstanceVersion == NULL)
    {
        return VK_MAKE_VERSION(1, 0, 0);
    }
    else
    {
        uint32_t version = 0;
        vkEnumerateInstanceVersion(&version);
        return version;
    }
}

VulkanDeviceFunctions::VulkanDeviceFunctions(VkInstance instance, VkDevice device, PFN_vkGetInstanceProcAddr pfnGetInstanceProcAddr)
    : vkGetInstanceProcAddr(pfnGetInstanceProcAddr)
    , h_vkInstance(instance)
    , h_device(device)
{
    GET_INSTANCE_PROC_ADDR(h_vkInstance, GetDeviceProcAddr);
    GET_DEVICE_PROC_ADDR(h_device, GetDeviceQueue)
    GET_DEVICE_PROC_ADDR(h_device, AllocateCommandBuffers)

    GET_DEVICE_PROC_ADDR(h_device, DeviceWaitIdle)
    GET_DEVICE_PROC_ADDR(h_device, QueueSubmit)
    GET_DEVICE_PROC_ADDR(h_device, QueueWaitIdle)
    GET_DEVICE_PROC_ADDR(h_device, CreateFence)
    GET_DEVICE_PROC_ADDR(h_device, WaitForFences)
    GET_DEVICE_PROC_ADDR(h_device, ResetFences)
    GET_DEVICE_PROC_ADDR(h_device, DestroyFence)
    GET_DEVICE_PROC_ADDR(h_device, CreateSemaphore)
    GET_DEVICE_PROC_ADDR(h_device, DestroySemaphore)
    GET_DEVICE_PROC_ADDR(h_device, CreateEvent)
    GET_DEVICE_PROC_ADDR(h_device, DestroyEvent)
    GET_DEVICE_PROC_ADDR(h_device, GetEventStatus)
    GET_DEVICE_PROC_ADDR(h_device, SetEvent)
    GET_DEVICE_PROC_ADDR(h_device, ResetEvent)
    GET_DEVICE_PROC_ADDR(h_device, CmdSetEvent)
    GET_DEVICE_PROC_ADDR(h_device, CmdResetEvent)
    GET_DEVICE_PROC_ADDR(h_device, CreateCommandPool)
    GET_DEVICE_PROC_ADDR(h_device, DestroyCommandPool)
    GET_DEVICE_PROC_ADDR(h_device, BeginCommandBuffer)
    GET_DEVICE_PROC_ADDR(h_device, EndCommandBuffer)
    GET_DEVICE_PROC_ADDR(h_device, ResetCommandBuffer)
    GET_DEVICE_PROC_ADDR(h_device, FreeCommandBuffers)
    GET_DEVICE_PROC_ADDR(h_device, CreateDescriptorPool)
    GET_DEVICE_PROC_ADDR(h_device, DestroyDescriptorPool)
    GET_DEVICE_PROC_ADDR(h_device, CreateDescriptorSetLayout)
    GET_DEVICE_PROC_ADDR(h_device, DestroyDescriptorSetLayout)
    GET_DEVICE_PROC_ADDR(h_device, AllocateDescriptorSets)
    GET_DEVICE_PROC_ADDR(h_device, FreeDescriptorSets)
    GET_DEVICE_PROC_ADDR(h_device, UpdateDescriptorSets)
    GET_DEVICE_PROC_ADDR(h_device, CreateRenderPass)
    GET_DEVICE_PROC_ADDR(h_device, DestroyRenderPass)
    GET_DEVICE_PROC_ADDR(h_device, CreateFramebuffer)
    GET_DEVICE_PROC_ADDR(h_device, DestroyFramebuffer)

    GET_DEVICE_PROC_ADDR(h_device, CreatePipelineCache)
    GET_DEVICE_PROC_ADDR(h_device, DestroyPipelineCache)
    GET_DEVICE_PROC_ADDR(h_device, CreatePipelineLayout)
    GET_DEVICE_PROC_ADDR(h_device, DestroyPipelineLayout)
    GET_DEVICE_PROC_ADDR(h_device, CreateGraphicsPipelines)
    GET_DEVICE_PROC_ADDR(h_device, CreateComputePipelines)
    GET_DEVICE_PROC_ADDR(h_device, DestroyPipeline)
    GET_DEVICE_PROC_ADDR(h_device, CreateShaderModule)
    GET_DEVICE_PROC_ADDR(h_device, DestroyShaderModule)

    GET_DEVICE_PROC_ADDR(h_device, AllocateMemory)
    GET_DEVICE_PROC_ADDR(h_device, FreeMemory)
    GET_DEVICE_PROC_ADDR(h_device, MapMemory)
    GET_DEVICE_PROC_ADDR(h_device, UnmapMemory)
    GET_DEVICE_PROC_ADDR(h_device, FlushMappedMemoryRanges)
    GET_DEVICE_PROC_ADDR(h_device, InvalidateMappedMemoryRanges)
    GET_DEVICE_PROC_ADDR(h_device, CreateBuffer)
    GET_DEVICE_PROC_ADDR(h_device, DestroyBuffer)
    GET_DEVICE_PROC_ADDR(h_device, GetBufferMemoryRequirements)
    GET_DEVICE_PROC_ADDR(h_device, GetImageMemoryRequirements)
    GET_DEVICE_PROC_ADDR(h_device, GetImageSubresourceLayout)
    GET_DEVICE_PROC_ADDR(h_device, BindBufferMemory)
    GET_DEVICE_PROC_ADDR(h_device, CreateImage)
    GET_DEVICE_PROC_ADDR(h_device, DestroyImage)
    GET_DEVICE_PROC_ADDR(h_device, BindImageMemory)
    GET_DEVICE_PROC_ADDR(h_device, CreateImageView)
    GET_DEVICE_PROC_ADDR(h_device, DestroyImageView)
    GET_DEVICE_PROC_ADDR(h_device, CreateSampler)
    GET_DEVICE_PROC_ADDR(h_device, DestroySampler)

    GET_DEVICE_PROC_ADDR(h_device, CmdBeginRenderPass)
    GET_DEVICE_PROC_ADDR(h_device, CmdEndRenderPass)
    GET_DEVICE_PROC_ADDR(h_device, CmdNextSubpass)
    GET_DEVICE_PROC_ADDR(h_device, CmdBlitImage)
    GET_DEVICE_PROC_ADDR(h_device, CmdCopyBuffer)
    GET_DEVICE_PROC_ADDR(h_device, CmdCopyImage)
    GET_DEVICE_PROC_ADDR(h_device, CmdCopyBufferToImage)
    GET_DEVICE_PROC_ADDR(h_device, CmdCopyImageToBuffer)
    GET_DEVICE_PROC_ADDR(h_device, CmdPipelineBarrier)
    GET_DEVICE_PROC_ADDR(h_device, CmdBindPipeline)
    GET_DEVICE_PROC_ADDR(h_device, CmdBindVertexBuffers)
    GET_DEVICE_PROC_ADDR(h_device, CmdBindIndexBuffer)
    GET_DEVICE_PROC_ADDR(h_device, CmdBindDescriptorSets)
    GET_DEVICE_PROC_ADDR(h_device, CmdSetViewport)
    GET_DEVICE_PROC_ADDR(h_device, CmdSetScissor)
    GET_DEVICE_PROC_ADDR(h_device, CmdPushConstants)
    GET_DEVICE_PROC_ADDR(h_device, CmdClearColorImage)
    GET_DEVICE_PROC_ADDR(h_device, CmdDraw)
    GET_DEVICE_PROC_ADDR(h_device, CmdDrawIndexed)
    GET_DEVICE_PROC_ADDR(h_device, CmdDispatch)

    GET_DEVICE_PROC_ADDR(h_device, CreateSwapchainKHR)
    GET_DEVICE_PROC_ADDR(h_device, DestroySwapchainKHR)
    GET_DEVICE_PROC_ADDR(h_device, GetSwapchainImagesKHR)
    GET_DEVICE_PROC_ADDR(h_device, AcquireNextImageKHR)
    GET_DEVICE_PROC_ADDR(h_device, QueuePresentKHR)
}

PFN_vkVoidFunction VulkanDeviceFunctions::external_loader(const char* function_name, void* loader_ptr)
{
    VulkanDeviceFunctions* pfn = reinterpret_cast<VulkanDeviceFunctions*>(loader_ptr);
    if (pfn->h_device == VK_NULL_HANDLE)
        return nullptr;

    // Explicite lookup table to avoid layer validation warnings
    std::vector<const char*> instance_func = {
        "vkDestroySurfaceKHR",
        "vkEnumeratePhysicalDevices",
        "vkGetPhysicalDeviceProperties",
        "vkGetPhysicalDeviceMemoryProperties",
        "vkGetPhysicalDeviceQueueFamilyProperties",
        "vkGetPhysicalDeviceSurfaceCapabilitiesKHR",
        "vkGetPhysicalDeviceSurfaceFormatsKHR",
        "vkGetPhysicalDeviceSurfacePresentModesKHR"
    };

    bool is_instance_level = false;
    for (const char* func_name : instance_func)
    {
        is_instance_level |= (strcmp(func_name, function_name) == 0);
    }

    if (is_instance_level)
    {
        return pfn->vkGetInstanceProcAddr(pfn->h_vkInstance, function_name);
    }
    else
    {
        return pfn->vkGetDeviceProcAddr(pfn->h_device, function_name);
    }
}
