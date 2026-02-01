#include "vulkan/VulkanManager.h"
#include "utils/Logger.h"

#include "pointCloudEngine/TlStreamer.h"
#include "vulkan/TlFramebuffer_T.h"
#include "vulkan/VulkanHelperFunctions.h"
#include "models/OpenScanToolsModelEssentials.h"

#include "magic_enum/magic_enum.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <chrono>
#include <thread>
#include <map>
#include <set>
#include <string>

#include <glm/gtc/packing.hpp>

#define TL_ENGINE_VERSION VK_MAKE_VERSION(0, 1, 0)

#define VKM_INFO Logger::log(LoggerMode::VKLog) << "INFO - "
#define VKM_WARNING Logger::log(LoggerMode::VKLog) << "WARNING - "
#define VKM_ERROR Logger::log(LoggerMode::VKLog) << "ERROR - "

#define LOG_FLAG(log, blank, flags, flag_bit) \
        if ((flags & flag_bit) != 0)\
            log << "X";\
        else\
            log << " ";\
        log << blank;

namespace
{
    bool readEnvFlag(const char* name)
    {
        const char* value = std::getenv(name);
        if (!value)
            return false;
        return strcmp(value, "1") == 0 || strcmp(value, "true") == 0 || strcmp(value, "TRUE") == 0;
    }

    double readEnvDouble(const char* name, double fallback)
    {
        const char* value = std::getenv(name);
        if (!value || *value == '\0')
            return fallback;
        char* end = nullptr;
        double parsed = std::strtod(value, &end);
        if (end == value)
            return fallback;
        return parsed;
    }
}

void deleteCharPP(uint32_t _count, char* const* _cstringList)
{
    for (uint32_t i = 0; i < _count; i++) {
        delete[] _cstringList[i];
    }
    delete[] _cstringList;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    Logger::log(VK_LOG) << "Validation Layer: " << pCallbackData->pMessage << Logger::endl;

    return VK_FALSE;
}

std::string vkVersionToStr(uint32_t version)
{
    std::string str = std::to_string(VK_VERSION_MAJOR(version));
    str += "." + std::to_string(VK_VERSION_MINOR(version));
    str += "." + std::to_string(VK_VERSION_PATCH(version));
    return str;
}

std::string nvidiaVersionToStr(uint32_t version)
{
    // TODO(robin) Put in a GPU_SPEC.h file
    std::string str = std::to_string((uint32_t)(version) >> 22);
    str += "." + std::to_string(((uint32_t)(version) >> 14) & 0x00ff);
    str += "." + std::to_string((uint32_t)(version) & 0x3fff);
    return str;
}

VulkanManager::~VulkanManager()
{
    deleteCharPP(m_extCount, m_extensions);

    // Destroy all the vulkan objects
    cleanupAll();
    checkAllocations();
}

bool VulkanManager::initVkInstance(bool enableValidationLayers, uint32_t extCount, const char** extensions)
{
    // We can only init one time
    if (m_vkInstance == VK_NULL_HANDLE)
    {
        if (enableValidationLayers)
        {
            VKM_INFO << "Enabling Standard Validation Layer." << Logger::endl;
            m_layerCount = 1;
            m_layers = new const char* { "VK_LAYER_KHRONOS_validation" };
        }

        VKM_INFO << "Vulkan found with version: " << vkVersionToStr(m_pfn.version()) << Logger::endl;

        // Deep Copy the extensions required
        if (enableValidationLayers)
            // Prepare space for additional extension
            m_extCount = extCount + 1;
        else
            m_extCount = extCount;

        m_extensions = new char* [m_extCount];
        for (uint32_t i = 0; i < extCount; i++)
        {
            m_extensions[i] = new char[strlen(extensions[i]) + 1];
            strcpy(m_extensions[i], extensions[i]);
        }

        // Add the extension for printing the validation messages
        if (enableValidationLayers)
        {
            m_extensions[m_extCount - 1] = new char[strlen(VK_EXT_DEBUG_UTILS_EXTENSION_NAME) + 1];
            strcpy(m_extensions[m_extCount - 1], VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        // Request extension for the swapChain
        m_devExtCount = 1;
        m_devExtensions = new const char* { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        if (!createVulkanInstance())
            return false;
        if (!setupDebugCallback())
            return false;
    }
    return true;
}

bool VulkanManager::initPhysicalDevice(std::string preferedDevice, std::unordered_map<uint32_t, std::string>& compliantDevices)
{
    if(!selectPhysicalDevice(preferedDevice, compliantDevices))
        return false;

    if (!checkAvailableFeatures(m_physDev, true))
        return false;

    // Print relevant limits
    SubLogger& log = Logger::log(VK_LOG);
    m_pfn.vkGetPhysicalDeviceProperties(m_physDev, &m_physDevProperties);
    VkPhysicalDeviceLimits const& limits = m_physDevProperties.limits;
    log << "***** Some Physical Device Limits *****";
    log << "\n - maxMemoryAllocationCount: " << limits.maxMemoryAllocationCount;
    log << "\n - maxPushConstantsSize: " << limits.maxPushConstantsSize;
    log << "\n - minUniformBufferOffsetAlignment: " << limits.minUniformBufferOffsetAlignment;
    log << "\n - minStorageBufferOffsetAlignment: " << limits.minStorageBufferOffsetAlignment;
    log << "\n - maxImageArrayLayers: " << limits.maxImageArrayLayers;
    log << "\n - maxPerStageDescriptorUniformBuffers: " << limits.maxPerStageDescriptorUniformBuffers;
    log << "\n - maxDescriptorSetUniformBuffers: " << limits.maxDescriptorSetUniformBuffers;
    log << "\n - maxDescriptorSetUniformBuffersDynamic: " << limits.maxDescriptorSetUniformBuffersDynamic;
    log << "\n - maxUniformBufferRange: " << limits.maxUniformBufferRange;
    log << "\n" << Logger::endl;

    // *** Blit support ***
    // From the Vulkan specs, VK_FORMAT_FEATURE_BLIT_SRC_BIT and VK_FORMAT_FEATURE_BLIT_DST_BIT are always supported for the formats below :
    //   - VK_FORMAT_R8G8B8A8_UNORM
    //   - VK_FORMAT_B8G8R8A8_UNORM
    //   - VK_FORMAT_R16G16B16A16_SFLOAT
    // 
    // For other formats, check the specs or use the function 'testFormatFeatures()'

    return true;
}

bool VulkanManager::initResources()
{
    logFormatFeatures(VK_FORMAT_D32_SFLOAT, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT | VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    logFormatFeatures(VK_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT);

    if (!createLogicalDevice() || !initVma())
        return false;

    findDepthFormat();
    m_imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
    //m_imageFormat = VK_FORMAT_B8G8R8A8_UINT; // cannot blend color
    //m_imageFormat = VK_FORMAT_R16G16B16A16_USCALED;  // not always supported

    if (   !createPCRenderPass(VK_FORMAT_B8G8R8A8_UNORM) 
        || !createPCRenderPass(VK_FORMAT_R16G16B16A16_SFLOAT)
        || !createPCRenderPass(VK_FORMAT_R16G16B16A16_UNORM)
        || !createObjectRenderPasses()
        || !createCommandPools()
        || !createDescriptorPool()
        || !createDescriptorSetLayouts()
        || !createFences()
        || !createStagingMemory()
        || !createUniformBuffer())
        return false;

    return true;
}

void VulkanManager::stopAllRendering()
{
    // TODO - Add an atomic to block all new command to the queues

    if (m_device)
        m_pfnDev->vkDeviceWaitIdle(m_device);
}

void VulkanManager::initStreaming()
{
    bool disableStreaming = readEnvFlag("OST_DISABLE_STREAMING");
    if (m_singleQueueFallback && readEnvFlag("OST_DISABLE_STREAMING_SINGLE_QUEUE"))
        disableStreaming = true;
    if (m_singleQueueFallback && !readEnvFlag("OST_FORCE_STREAMING_SINGLE_QUEUE"))
        disableStreaming = true;

    if (disableStreaming)
    {
        VKM_WARNING << "Streaming disabled (single-queue or environment override)." << Logger::endl;
        startTransferThread();
        return;
    }

    if (m_streamer == nullptr)
        m_streamer = new TlStreamer(m_device, m_pfnDev, getQueue(m_streamingQID), m_streamingQID.family, 32 * 1024 * 1024);
    else
        VKM_ERROR << "Try to start the streaming module more than one time." << Logger::endl;

    m_streamingThread = std::thread(&TlStreamer::start, m_streamer);

    startTransferThread();
}

void VulkanManager::stopStreaming()
{
    // Stop the independant streamer
    m_streamer->stop();
    m_streamingThread.join();

    delete m_streamer;
    m_streamer = nullptr;

    // Stop the transfer queue
    m_stopTransferThread = true;
    m_transfer_cv.notify_all();
    m_transferThread.join();
}

void VulkanManager::startTransferThread()
{
    m_transferThread = std::thread(
        [this]
    {
        for (;;)
        {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(m_transferMutex);
                m_transfer_cv.wait(lock,
                    [this] { return m_stopTransferThread || !m_transferTasks.empty(); });
                // NOTE - Avant de stopper le thread il faut finir tout les transferts car d'autres threads attendent le std::future.
                if (m_stopTransferThread && m_transferTasks.empty())
                    return;
                task = std::move(m_transferTasks.front());
                m_transferTasks.pop();
            }

            task();
        }
    });
}

bool VulkanManager::initFramebuffer(TlFramebuffer& _framebuffer, uint64_t _winId, int _width, int _height, bool preciseColor)
{
    if (m_vkInstance == VK_NULL_HANDLE) {
        VKM_ERROR << "No Vulkan instance, cannot init resources.\n" << Logger::endl;
        return false;
    }

    VkSurfaceKHR surface = m_pfn.createSurfaceForWindow(_winId);

    _framebuffer = new TlFramebuffer_T();

    _framebuffer->surface = surface;
    _framebuffer->requestedExtent = { (uint32_t)_width, (uint32_t)_height };

    // Call once
    _framebuffer->surfaceFormat = selectSwapchainSurfaceFormat(surface, m_imageFormat);
    _framebuffer->presentMode = selectSwapchainPresentMode(surface);
    _framebuffer->imageCount = selectImageCount(surface);

    m_maxSafeFrame = std::max(m_maxSafeFrame, _framebuffer->imageCount);

    createDrawBuffers(_framebuffer);

    // Recreate on resize
    if (!createSwapchain(_framebuffer))
        return false;
    _framebuffer->pcFormat = preciseColor ? VK_FORMAT_R16G16B16A16_SFLOAT : VK_FORMAT_B8G8R8A8_UNORM;
    createPCRenderPass(_framebuffer->pcFormat);
    createDepthBuffers(_framebuffer);
    createAttachments(_framebuffer);
    createCopyBuffers(_framebuffer);
    allocateDescriptorSet(_framebuffer);
    createPcFramebuffer(_framebuffer);
    createFinalFramebuffers(_framebuffer);
    createCommandBuffers(_framebuffer);
    createSemaphores(_framebuffer);

    return true;
}

void VulkanManager::destroyFramebuffer(TlFramebuffer& framebuffer)
{
    cleanupSizeDependantResources(framebuffer);
    cleanupPermanentResources(framebuffer);

    // Destroy the surface in last
    m_pfn.vkDestroySurfaceKHR(m_vkInstance, framebuffer->surface, nullptr);
    delete framebuffer;
    framebuffer = TL_NULL_HANDLE;
}

bool VulkanManager::createVirtualViewport(uint32_t _width, uint32_t _height, int multisample, TlFramebuffer& _virtualViewport)
{
    _virtualViewport = new TlFramebuffer_T();
    _virtualViewport->requestedExtent = { _width, _height };
    _virtualViewport->extent = { _width, _height };
    // TODO - Essayer de conserver une image finale en 16bits VK_FORMAT_R16G16B16A16_UNORM
    _virtualViewport->surfaceFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    _virtualViewport->pcFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
    //VkFormat::VK_FORMAT_R16G16B16A16_UNORM;
    //VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT;
    _virtualViewport->imageCount = 1;
    _virtualViewport->currentImage = 0;

    createDrawBuffers(_virtualViewport);

    createPCRenderPass(_virtualViewport->pcFormat);
    createVirtualRenderImages(_virtualViewport);
    createDepthBuffers(_virtualViewport);
    createAttachments(_virtualViewport);
    createCopyBuffers(_virtualViewport);
    allocateDescriptorSet(_virtualViewport);
    createPcFramebuffer(_virtualViewport);
    createFinalFramebuffers(_virtualViewport);
    createCommandBuffers(_virtualViewport);
    createSemaphores(_virtualViewport);

    return true;
}

ImageTransferEvent VulkanManager::transferFramebufferImage(TlFramebuffer _framebuffer, VkCommandBuffer _cmdBuffer, bool preciseColor) const
{
    // Source for the copy is the last rendered swapchain image
    VkImage srcImage = preciseColor ? _framebuffer->pcColorImage : _framebuffer->pImages[_framebuffer->currentImage];
    VkImageLayout srcLayout = preciseColor ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkFormat transferFormat = preciseColor ? _framebuffer->pcFormat : VK_FORMAT_B8G8R8A8_UNORM;

    // Create the image and his memory
    ImageTransferEvent ite = {};

    // Create the linear tiled destination image to copy to and to read the memory from
    createImage(_framebuffer->extent, 1u, transferFormat, VK_IMAGE_TILING_LINEAR, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, ite.image, ite.memory);
    ite.width = _framebuffer->extent.width;
    ite.height = _framebuffer->extent.height;
    ite.format = transferFormat;

    VkImageMemoryBarrier imgBarriers[2] = {
        // Transition source image from present/general to transfer source layout
        {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            nullptr,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_ACCESS_TRANSFER_READ_BIT,
            srcLayout,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            srcImage,
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
        },
        // Transition destination image to transfer destination layout
        {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            nullptr,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            ite.image,
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
        }
    };

    m_pfnDev->vkCmdPipelineBarrier(
        _cmdBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        2, imgBarriers);


    // If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB)
    if (checkBlitSupport(_framebuffer->surfaceFormat.format, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_LINEAR))
    {
        // Define the region to blit (we will blit the whole swapchain image)
        VkOffset3D blitSize;
        blitSize.x = _framebuffer->extent.width;
        blitSize.y = _framebuffer->extent.height;
        blitSize.z = 1;
        VkImageBlit imageBlitRegion{};
        imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.srcSubresource.layerCount = 1;
        imageBlitRegion.srcOffsets[1] = blitSize;
        imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.dstSubresource.layerCount = 1;
        imageBlitRegion.dstOffsets[1] = blitSize;

        // Issue the blit command
        m_pfnDev->vkCmdBlitImage(
            _cmdBuffer,
            srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            ite.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &imageBlitRegion,
            VK_FILTER_NEAREST);
    }
    else
    {
        // Otherwise use image copy (requires us to manually flip components)
        // TODO - notify the user that the output image needs to be flipped
        VkImageCopy imageCopyRegion{};
        imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.srcSubresource.layerCount = 1;
        imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.dstSubresource.layerCount = 1;
        imageCopyRegion.extent.width = _framebuffer->extent.width;
        imageCopyRegion.extent.height = _framebuffer->extent.height;
        imageCopyRegion.extent.depth = 1;

        // Issue the copy command
        m_pfnDev->vkCmdCopyImage(
            _cmdBuffer,
            srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            ite.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &imageCopyRegion);
    }

    // Transition back the swap chain image after the blit is done
    imgBarriers[0].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    imgBarriers[0].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    imgBarriers[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    imgBarriers[0].newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Transition destination image to general layout, which is the required layout for mapping the image memory later on
    imgBarriers[1].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imgBarriers[1].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    imgBarriers[1].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imgBarriers[1].newLayout = VK_IMAGE_LAYOUT_GENERAL;

    m_pfnDev->vkCmdPipelineBarrier(
        _cmdBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        2, imgBarriers);

    VkEventCreateInfo eventCreateInfo = {};
    eventCreateInfo.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    m_pfnDev->vkCreateEvent(m_device, &eventCreateInfo, nullptr, &ite.transferEvent);
    m_pfnDev->vkCmdSetEvent(_cmdBuffer, ite.transferEvent, VK_PIPELINE_STAGE_TRANSFER_BIT);

    return ite;
}

// TODO - add the pixel encoding/size
// TODO - This function is best handled by the 'ImageWriter' dedicated class
bool VulkanManager::doImageTransfer(ImageTransferEvent ite, uint32_t dstW, uint32_t dstH, char* dstBuffer, size_t dstSize, uint32_t dstOffsetW, uint32_t dstOffsetH, uint32_t border, bool preciseColor) const
{
    assert(dstBuffer);
    if (dstOffsetW > dstW || dstOffsetH > dstH)
        return false;
    // Wait for the event to be signaled
    int count = 0;
    // Try a few times without a long sleep to avoid stalling when the GPU finishes quickly
    while (m_pfnDev->vkGetEventStatus(m_device, ite.transferEvent) != VK_EVENT_SET && count < 1000)
    {
        if (count < 10)
            std::this_thread::yield();
        else
            std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(2.0f));
        ++count;
    }
    // NOTE - This is not a big problem if we continue with a VkEvent not signaled.
    //        This does not break any Vulkan rules.
    //        The image downloaded from the device memory can just be incomplete.

    // Get layout of the image (including row pitch)
    VkImageSubresource subResource{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
    VkSubresourceLayout subResourceLayout;
    m_pfnDev->vkGetImageSubresourceLayout(m_device, ite.image, &subResource, &subResourceLayout);

    // Map image memory so we can start copying from it
    const char* srcBuffer;
    m_pfnDev->vkMapMemory(m_device, ite.memory, 0, VK_WHOLE_SIZE, 0, (void**)&srcBuffer);
    srcBuffer += subResourceLayout.offset;
    srcBuffer += subResourceLayout.rowPitch * border;

    // On Ã©crit ligne par ligne
    uint32_t innerW = ite.width - 2 * border;
    uint32_t innerH = ite.height - 2 * border;
    uint32_t clampedW = std::min(innerW, dstW - dstOffsetW);
    uint32_t clampedH = std::min(innerH, dstH - dstOffsetH);
    const bool convertToUint16 = preciseColor && ite.format == VK_FORMAT_R16G16B16A16_SFLOAT;
    const size_t bytesPerPixel = convertToUint16 ? 8ull : 4ull;

    for (uint32_t h = 0; h < clampedH; h++)
    {        
        size_t srcOffset = bytesPerPixel * border;
        size_t dstOffset = bytesPerPixel * (dstOffsetW + (h + (size_t)dstOffsetH) * dstW);

        if (convertToUint16)
        {
            const uint16_t* srcRow = reinterpret_cast<const uint16_t*>(srcBuffer + srcOffset);
            uint16_t* dstRow = reinterpret_cast<uint16_t*>(dstBuffer + dstOffset);
            for (uint32_t w = 0; w < clampedW; ++w)
            {
                const uint16_t* srcPixel = srcRow + 4ull * w;
                // Components are stored as float16 -> convert to uint16_t in [0, 65535]
                float r = glm::unpackHalf1x16(srcPixel[0]);
                float g = glm::unpackHalf1x16(srcPixel[1]);
                float b = glm::unpackHalf1x16(srcPixel[2]);
                float a = glm::unpackHalf1x16(srcPixel[3]);
                dstRow[4ull * w + 0] = static_cast<uint16_t>(std::round(std::clamp(r, 0.f, 1.f) * 65535.f));
                dstRow[4ull * w + 1] = static_cast<uint16_t>(std::round(std::clamp(g, 0.f, 1.f) * 65535.f));
                dstRow[4ull * w + 2] = static_cast<uint16_t>(std::round(std::clamp(b, 0.f, 1.f) * 65535.f));
                dstRow[4ull * w + 3] = static_cast<uint16_t>(std::round(std::clamp(a, 0.f, 1.f) * 65535.f));
            }
        }
        else
        {
            memcpy_s(dstBuffer + dstOffset, dstSize, srcBuffer + srcOffset, bytesPerPixel * (size_t)clampedW);
        }
        srcBuffer += subResourceLayout.rowPitch;
    }

    // Clean up resources
    m_pfnDev->vkUnmapMemory(m_device, ite.memory);
    m_pfnDev->vkFreeMemory(m_device, ite.memory, nullptr);
    m_pfnDev->vkDestroyImage(m_device, ite.image, nullptr);
    m_pfnDev->vkDestroyEvent(m_device, ite.transferEvent, nullptr);

    return true;
}

void VulkanManager::resizeFramebuffer(TlFramebuffer _fb, int _width, int _height)
{
    if (_fb == TL_NULL_HANDLE)
        return;
    if (_width < 0 || _height < 0)
    {
        _fb->requestedExtent = { 0u, 0u };
    }
    else if (_fb->extent.width != _width || _fb->extent.height != _height)
    {
        _fb->requestedExtent = { static_cast<uint32_t>(_width), static_cast<uint32_t>(_height) };
        _fb->mustRecreateSwapchain.store(true);
        VKM_INFO << "Resize the framebuffer to {" << _width << " x " << _height << "}\n" << Logger::endl;
    }
}

VkInstance VulkanManager::getVkInstance() const
{
    return m_vkInstance;
}

VkRenderPass VulkanManager::getPCRenderPass(VkFormat imageFormat) const
{
    if (m_renderPass_pc.find(imageFormat) != m_renderPass_pc.end())
        return m_renderPass_pc.at(imageFormat);
    else
        return VK_NULL_HANDLE;
}

VkRenderPass VulkanManager::getObjectRenderPass() const
{
    return m_renderPass_obj;
}

uint32_t VulkanManager::getGraphicsQFI() const
{
    return m_graphicsQID.family;
}

VkQueue VulkanManager::getGraphicsQueue() const
{
    return getQueue(m_graphicsQID);
}

VkCommandBuffer VulkanManager::getGraphicsCmdBuffer(TlFramebuffer _fb)
{
    return _fb->graphicsCmdBuffers[_fb->currentFrame];
}

/*
VkCommandBuffer VulkanManager::getThreadCmdBuffer(TlFramebuffer _fb)
{
    std::thread::id threadId = std::this_thread::get_id();
    if (_fb->cmdBuffers[_fb->currentFrame].find(threadId) == _fb->cmdBuffers[_fb->currentFrame].end())
    {
        // Create the command pool if it does not exist on this thread
        if (_fb->cmdPools.find(threadId) == _fb->cmdPools.end())
        {
            VkCommandPoolCreateInfo cmdPoolInfo = {};
            VkCommandPool cmdPool = VK_NULL_HANDLE;

            cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            cmdPoolInfo.queueFamilyIndex = m_graphicsQID.family;

            VkResult err = m_pfnDev->vkCreateCommandPool(m_device, &cmdPoolInfo, nullptr, &cmdPool);
            check_vk_result(err, "Create Graphics Command Pool");
            _fb->cmdPools.insert({ threadId, cmdPool });
        }
        // Create a command buffer and initialize it

    }
    else
    {
        return (_fb->cmdBuffers[_fb->currentFrame][threadId]);
    }
    return VK_NULL_HANDLE;
}*/

VkPhysicalDevice VulkanManager::getPhysicalDevice() const
{
    return m_physDev;
}

VkDevice VulkanManager::getDevice() const
{
    return m_device;
}

VulkanDeviceFunctions* VulkanManager::getDeviceFunctions() const
{
    return m_pfnDev;
}

VkDescriptorPool VulkanManager::getDescriptorPool() const
{
    return m_descPool;
}

VkDescriptorSetLayout VulkanManager::getDSLayout_inputDepth()
{
    return getInstance().m_descSetLayout_inputDepth;
}

VkDescriptorSetLayout VulkanManager::getDSLayout_fillingSamplers()
{
    return getInstance().m_descSetLayout_filling;
}

VkDescriptorSetLayout VulkanManager::getDSLayout_finalOutput()
{
    return getInstance().m_descSetLayout_finalOutput;
}

VkDescriptorSetLayout VulkanManager::getDSLayout_inputTransparentLayer()
{
    return getInstance().m_descSetLayout_inputTransparentLayer;
}

uint32_t VulkanManager::getImageCount(TlFramebuffer _fb)
{
    return (_fb->imageCount);
}

VkBuffer VulkanManager::getUniformBuffer() const
{
    return m_uniformBuf;
}

checkFct VulkanManager::getCheckFunction() const
{
    return check_vk_result;
}

VkPhysicalDeviceLimits VulkanManager::getPhysicalDeviceLimits()
{
    VkPhysicalDeviceProperties properties;
    m_pfn.vkGetPhysicalDeviceProperties(m_physDev, &properties);
    return (properties.limits);
}


float VulkanManager::sampleDepth(TlFramebuffer _fb)
{
    if (_fb == TL_NULL_HANDLE)
        return 1.f;
    if (!_fb->pMappedCopyDepth)
        return 1.f;
    if (_fb->dsFormat == VK_FORMAT_D32_SFLOAT)
        return ((float*)_fb->pMappedCopyDepth)[0];
    else if (_fb->dsFormat == VK_FORMAT_X8_D24_UNORM_PACK32)
    {
        uint32_t unorm_depth = ((uint32_t*)_fb->pMappedCopyDepth)[0];
        return ((float)unorm_depth / 16777215u);
    }
    else
        return 1.f;
}

uint32_t VulkanManager::sampleIndex(TlFramebuffer _fb, uint32_t posX, uint32_t posY)
{
    if (posX >= _fb->extent.width || posY >= _fb->extent.height || !_fb->pMappedCopyIndex)
        return INVALID_PICKING_ID;

    return ((uint32_t*)_fb->pMappedCopyIndex)[posY * _fb->extent.width + posX];
}

//From https://stackoverflow.com/questions/5056645/sorting-stdmap-using-value
template<typename A, typename B>
static std::pair<B, A> flip_pair(const std::pair<A, B>& p)
{
    return std::pair<B, A>(p.second, p.first);
};

template<typename A, typename B>
static std::multimap<B, A> flip_map(const std::unordered_map<A, B>& src)
{
    std::multimap<B, A> dst;
    std::transform(src.begin(), src.end(), std::inserter(dst, dst.begin()), flip_pair<A, B>);
    return dst;
};
//////////////////

std::vector<uint32_t> VulkanManager::sampleIndexList(TlFramebuffer _fb, uint32_t posX, uint32_t posY, uint32_t range)
{
    if (posX >= _fb->extent.width || posY >= _fb->extent.height || !_fb->pMappedCopyIndex) 
        return {};

    uint32_t startX = std::max(0, (int)posX - (int)range);
    uint32_t startY = std::max(0, (int)posY - (int)range);
    uint32_t endX = std::min(_fb->extent.width, posX + range + 1);
    uint32_t endY = std::min(_fb->extent.height, posY + range + 1);

    std::unordered_map<uint32_t, uint32_t> counter;
    for (uint32_t x = startX; x < endX; x++)
    {
        for (uint32_t y = startY; y < endY; y++)
        {
            uint32_t value(((uint32_t*)_fb->pMappedCopyIndex)[y * _fb->extent.width + x]);
            if (value != INVALID_PICKING_ID)
                counter[value] += (range + 1) - (uint32_t)std::max(abs((float)posX - x), abs((float)posY - y));
        }
    }
    if (counter.empty())
        return {};

    std::vector<uint32_t> result;
    std::multimap<uint32_t, uint32_t> dst = flip_map(counter);
    for (const auto& iterator : dst)
        result.push_back(iterator.second);
    return result;
}

//-----------------------------------------------------------------------------
//                Frame drawing functions
//-----------------------------------------------------------------------------

void VulkanManager::startNextFrame()
{
    if (m_missedDeviceAllocations > 0)
        VKM_WARNING << m_missedDeviceAllocations << " device allocations failed during frame " << m_currentFrameIndex << Logger::endl;
    if (m_missedHostAllocations > 0)
        VKM_WARNING << m_missedHostAllocations << " host allocations failed during frame " << m_currentFrameIndex << Logger::endl;
    m_missedDeviceAllocations = 0;
    m_missedHostAllocations = 0;

    m_currentFrameIndex.fetch_add(1);
    m_noMoreFreeMemoryForFrame_device.store(false);
    m_noMoreFreeMemoryForFrame_host.store(false);
    vmaSetCurrentFrameIndex(m_allocator, m_currentFrameIndex.load());
}

bool VulkanManager::acquireNextImage(TlFramebuffer _fb)
{
    assert(_fb);
    if (checkSwapchain(_fb) == false)
        return false;

    _fb->currentFrame = (_fb->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    VkResult err = m_pfnDev->vkAcquireNextImageKHR(m_device, _fb->swapchain, UINT64_MAX, _fb->imageAvailableSemaphore[_fb->currentFrame], VK_NULL_HANDLE, &_fb->currentImage);

    if (err == VK_ERROR_OUT_OF_DATE_KHR) {
        _fb->mustRecreateSwapchain.store(true);
        VKM_INFO << "Out of date swapchain (begin render pass)\n" << Logger::endl;
        return false;
    }
    else if (err != VK_SUCCESS && err != VK_SUBOPTIMAL_KHR) {
        VKM_ERROR << "Failed to acquire swapchain image !\n" << Logger::endl;
        //assert(0);
        return false;
    }

    return true;
}

void VulkanManager::beginCommandBuffer(VkCommandBuffer _cmdBuffer)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkResult err = m_pfnDev->vkBeginCommandBuffer(_cmdBuffer, &beginInfo);
    check_vk_result(err, "Begin command buffer");
}

void VulkanManager::resetCommandBuffer(VkCommandBuffer _cmdBuffer)
{
    m_pfnDev->vkResetCommandBuffer(_cmdBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
}

void VulkanManager::endCommandBuffer(VkCommandBuffer _cmdBuffer)
{
    VkResult err = m_pfnDev->vkEndCommandBuffer(_cmdBuffer);
    check_vk_result(err, "End Command buffer");
}

void VulkanManager::beginScanRenderPass(TlFramebuffer _fb, VkClearColorValue _clearColor)
{
    if (_fb->initColorLayout)
    {
        VkImageMemoryBarrier imageBarrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            nullptr,
            0,
            0,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            _fb->pcColorImage,
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
        };

        m_pfnDev->vkCmdPipelineBarrier(_fb->graphicsCmdBuffers[_fb->currentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
        _fb->initColorLayout = false;
    }

    // The render pass does not automatically clear the point cloud attachments.
    const int clearCount = 2;
    VkClearValue clearValues[clearCount];
    clearValues[0].color = _clearColor;
    clearValues[1].depthStencil = { 1.f, 0u };

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass_pc.at(_fb->pcFormat);
    renderPassInfo.framebuffer = _fb->pcFramebuffer;
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = _fb->extent;
    renderPassInfo.clearValueCount = clearCount;
    renderPassInfo.pClearValues = clearValues;

    m_pfnDev->vkCmdBeginRenderPass(_fb->graphicsCmdBuffers[_fb->currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanManager::endScanRenderPass(TlFramebuffer _fb)
{
    m_pfnDev->vkCmdEndRenderPass(_fb->graphicsCmdBuffers[_fb->currentFrame]);
}

void VulkanManager::beginPostTreatmentFilling(TlFramebuffer _fb)
{
    // Transition image layout for sampling
    VkImageMemoryBarrier barriers[] = {
        {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            nullptr,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            _fb->pcColorImage,
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
        },
        {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            nullptr,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            _fb->pcDepthImage,
            { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 }
        }
    };

    VkBufferMemoryBarrier buffer_barrier =
    {
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        nullptr,
        VK_ACCESS_SHADER_READ_BIT,
        VK_ACCESS_SHADER_WRITE_BIT,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        _fb->correctedDepthBuffer,
        0,
        _fb->correctedDepthSize
    };

    m_pfnDev->vkCmdPipelineBarrier(
        _fb->graphicsCmdBuffers[_fb->currentFrame],
        VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0,
        0, nullptr,
        sizeof(buffer_barrier) / sizeof(VkBufferMemoryBarrier), &buffer_barrier,
        sizeof(barriers) / sizeof(VkImageMemoryBarrier), barriers);
}

void VulkanManager::beginPostTreatmentNormal(TlFramebuffer _fb)
{
    // Transition image layout for sampling
    VkImageMemoryBarrier barriers[] = {
        {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            nullptr,
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            _fb->pcColorImage,
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
        }
    };

    VkBufferMemoryBarrier buffer_barrier =
    {
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        nullptr,
        VK_ACCESS_SHADER_WRITE_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        _fb->correctedDepthBuffer,
        0,
        _fb->correctedDepthSize
    };

    m_pfnDev->vkCmdPipelineBarrier(
        _fb->graphicsCmdBuffers[_fb->currentFrame],
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0,
        0, nullptr,
        sizeof(buffer_barrier) / sizeof(VkBufferMemoryBarrier), &buffer_barrier,
        sizeof(barriers) / sizeof(VkImageMemoryBarrier), barriers);
}

void VulkanManager::beginPostTreatmentAmbientOcclusion(TlFramebuffer _fb)
{
    beginPostTreatmentNormal(_fb);
}

void VulkanManager::beginPostTreatmentEdgeAwareBlur(TlFramebuffer _fb)
{
    beginPostTreatmentNormal(_fb);
}

void VulkanManager::beginPostTreatmentDepthLining(TlFramebuffer _fb)
{
    beginPostTreatmentNormal(_fb);
}

void VulkanManager::beginPostTreatmentTransparency(TlFramebuffer _fb)
{
    // Transition image layout for sampling
    VkImageMemoryBarrier barriers[] = {
        {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            nullptr,
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            _fb->pcColorImage,
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
        }
    };

    m_pfnDev->vkCmdPipelineBarrier(
        _fb->graphicsCmdBuffers[_fb->currentFrame],
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        sizeof(barriers) / sizeof(VkImageMemoryBarrier), barriers);
}

void VulkanManager::beginObjectRenderPass(TlFramebuffer _fb)
{
    VkCommandBuffer currentCmdBuffer = _fb->graphicsCmdBuffers[_fb->currentFrame];

    VkImageMemoryBarrier imageBarriers_alpha[] = {
        {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            0,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            _fb->initLayout > 0 ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            _fb->pImages[_fb->currentImage],
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
        },
        {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            nullptr,
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            VK_ACCESS_TRANSFER_READ_BIT,
            _fb->initColorLayout ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            _fb->pcColorImage,
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
        }
    };
    if (_fb->initLayout > 0) _fb->initLayout--;
    _fb->initColorLayout = false;

    VkBufferMemoryBarrier bufferBarrier =
    {
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        nullptr,
        VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
        VK_ACCESS_TRANSFER_READ_BIT,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        _fb->correctedDepthBuffer,
        0,
        _fb->correctedDepthSize
    };

    m_pfnDev->vkCmdPipelineBarrier(
        currentCmdBuffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        sizeof(bufferBarrier) / sizeof(VkBufferMemoryBarrier), &bufferBarrier,
        sizeof(imageBarriers_alpha) / sizeof (VkImageMemoryBarrier), imageBarriers_alpha);

    // Copy the point cloud depth image to the main depth image
    pipelineBarrier(currentCmdBuffer, _fb->objectDepthImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);

    if (_fb->pcFormat != _fb->surfaceFormat.format && 
        checkBlitSupport(_fb->pcFormat, _fb->surfaceFormat.format, VK_IMAGE_TILING_OPTIMAL))
    {
        // Define the region to blit (we will blit the whole swapchain image)
        VkOffset3D blitSize;
        blitSize.x = _fb->extent.width;
        blitSize.y = _fb->extent.height;
        blitSize.z = 1;
        VkImageBlit imageBlitRegion{};
        imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.srcSubresource.layerCount = 1;
        imageBlitRegion.srcOffsets[1] = blitSize;
        imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.dstSubresource.layerCount = 1;
        imageBlitRegion.dstOffsets[1] = blitSize;

        // Issue the blit command
        m_pfnDev->vkCmdBlitImage(
            currentCmdBuffer,
            _fb->pcColorImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            _fb->pImages[_fb->currentImage], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &imageBlitRegion,
            VK_FILTER_NEAREST);
    }
    else
    {
        VkImageCopy imageCopyRegion{};
        imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.srcSubresource.layerCount = 1;
        imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.dstSubresource.layerCount = 1;
        imageCopyRegion.extent.width = _fb->extent.width;
        imageCopyRegion.extent.height = _fb->extent.height;
        imageCopyRegion.extent.depth = 1;

        // Transfer the color and depth values
        m_pfnDev->vkCmdCopyImage(
            currentCmdBuffer,
            _fb->pcColorImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            _fb->pImages[_fb->currentImage], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &imageCopyRegion);
    }

    copyBufferToImage(currentCmdBuffer, _fb->correctedDepthBuffer, _fb->objectDepthImage, VK_IMAGE_ASPECT_DEPTH_BIT, 0, { _fb->extent.width, _fb->extent.height, 1 });

    // Switch the next render target in the right layout
    VkImageMemoryBarrier imageBarriers_beta[] = {
    {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        nullptr,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        _fb->pImages[_fb->currentImage],
        { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
    },
    {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        nullptr,
        VK_ACCESS_TRANSFER_READ_BIT,
        0,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        _fb->pcColorImage,
        { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
    }
    };

    m_pfnDev->vkCmdPipelineBarrier(
        currentCmdBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        0, nullptr,
        0, nullptr,
        sizeof(imageBarriers_beta) / sizeof(VkImageMemoryBarrier), imageBarriers_beta);

    VkImageMemoryBarrier barrier_depth = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        _fb->objectDepthImage,
        { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 }
    };

    m_pfnDev->vkCmdPipelineBarrier(
        currentCmdBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        0,
        0, nullptr,
        0, nullptr,
        sizeof(barrier_depth) / sizeof(VkImageMemoryBarrier), &barrier_depth);

    // Begin object render pass
    constexpr int clearCount = 5;
    VkClearValue clearValues[clearCount];
    clearValues[0].color = { 0.f, 0.f, 0.f, 0.f }; // unused by render pass
    clearValues[1].depthStencil = { 1.f, 0u };
    clearValues[2].color = { .uint32 = { INVALID_PICKING_ID, 0u, 0u, 0u } };
    clearValues[3].color = { 0.f, 0.f, 0.f, 0.f };
    clearValues[4].depthStencil = { 1.f, 0u };

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass_obj;
    renderPassInfo.framebuffer = _fb->finalFramebuffers[_fb->currentImage];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = _fb->extent;
    renderPassInfo.clearValueCount = clearCount;
    renderPassInfo.pClearValues = clearValues;

    m_pfnDev->vkCmdBeginRenderPass(currentCmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {
        0.f, 0.f,
        float(_fb->extent.width), float(_fb->extent.height),
        0, 1
    };
    m_pfnDev->vkCmdSetViewport(currentCmdBuffer, 0, 1, &viewport);

    VkRect2D scissor = {
        { 0, 0 },
        { _fb->extent.width, _fb->extent.height }
    };
    m_pfnDev->vkCmdSetScissor(currentCmdBuffer, 0, 1, &scissor);

}

void VulkanManager::endObjectRenderPass(TlFramebuffer _fb)
{
    m_pfnDev->vkCmdEndRenderPass(_fb->graphicsCmdBuffers[_fb->currentFrame]);

    // **** Copy ****
    VkBufferImageCopy copyRegion = {};
    copyRegion.bufferOffset = 0;
    copyRegion.bufferRowLength = 0;
    copyRegion.bufferImageHeight = 0;
    copyRegion.imageSubresource = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1 };
    copyRegion.imageOffset = { 0, 0, 0 };
    copyRegion.imageExtent = { _fb->extent.width, _fb->extent.height, 1 };
    //m_pfnDev->vkCmdCopyImageToBuffer(_fb->graphicsCmdBuffers[_fb->currentFrame], _fb->objectDepthImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, _fb->copyBufDepth, 1, &copyRegion);

    copyRegion.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    m_pfnDev->vkCmdCopyImageToBuffer(_fb->graphicsCmdBuffers[_fb->currentFrame], _fb->idAttImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, _fb->copyBufIndex, 1, &copyRegion);
}

void VulkanManager::applyPicking(TlFramebuffer _fb, glm::ivec2 _pickingPos, VkCommandBuffer* singleCommandBuffer)
{
    if (_pickingPos.x < 0 || _pickingPos.y < 0 ||
        _pickingPos.x >= (int)_fb->extent.width || _pickingPos.y >= (int)_fb->extent.height)
        return;
    // Copy only the picking region
    VkBufferImageCopy copyRegion = {};
    copyRegion.bufferOffset = 0;
    copyRegion.bufferRowLength = 0;
    copyRegion.bufferImageHeight = 0;
    copyRegion.imageSubresource = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1 };
    copyRegion.imageOffset = { _pickingPos.x, _pickingPos.y, 0 };
    copyRegion.imageExtent = { 1, 1, 1 };
    if(singleCommandBuffer)
        m_pfnDev->vkCmdCopyImageToBuffer(*singleCommandBuffer, _fb->objectDepthImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, _fb->copyBufDepth, 1, &copyRegion);
    else
    m_pfnDev->vkCmdCopyImageToBuffer(_fb->graphicsCmdBuffers[_fb->currentFrame], _fb->objectDepthImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, _fb->copyBufDepth, 1, &copyRegion);
}

void VulkanManager::submitMultipleFramebuffer(std::vector<TlFramebuffer> fbs)
{
    VkResult err;

    if (fbs.empty())
        return;

    const uint32_t fenceIndex = m_currentFrameIndex.load() % MAX_FRAMES_IN_FLIGHT;
    VkFence renderFence = m_renderFences[fenceIndex];
    m_pfnDev->vkResetFences(m_device, 1, &renderFence);

    std::vector<VkSubmitInfo> submitInfos;
    std::vector<VkPipelineStageFlags> waitStageMasks(
        fbs.size(),
        VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    submitInfos.reserve(fbs.size());
    for (size_t i = 0; i < fbs.size(); ++i)
    {
        TlFramebuffer fb = fbs[i];
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &fb->imageAvailableSemaphore[fb->currentFrame];
        submitInfo.pWaitDstStageMask = &waitStageMasks[i];
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &fb->graphicsCmdBuffers[fb->currentFrame];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &fb->renderFinishedSemaphore;

        submitInfos.push_back(submitInfo);
    }
    err = m_pfnDev->vkQueueSubmit(getQueue(m_graphicsQID), (uint32_t)submitInfos.size(), submitInfos.data(), renderFence);
    check_vk_result(err, "Submit Graphic Queue");

    for (TlFramebuffer fb : fbs)
    {
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &fb->renderFinishedSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &fb->swapchain;
        presentInfo.pImageIndices = &fb->currentImage;
        // FIXME - Utiliser la bonne queue si la presentation se fait sur une queue diffÃ©rente.
        err = m_pfnDev->vkQueuePresentKHR(getQueue(m_graphicsQID), &presentInfo);
        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
            VKM_INFO << "Out of date swapchain (end render pass)\n" << Logger::endl;
            fb->mustRecreateSwapchain.store(true);
        }
        else if (err == VK_SUBOPTIMAL_KHR) {
            VKM_INFO << "Suboptimal swapchain (end render pass)\n" << Logger::endl;
            fb->mustRecreateSwapchain.store(true);
        }
        else if (err != VK_SUCCESS) {
            VKM_ERROR << "Failed to present swapchain image !\n" << Logger::endl;
            exit(1);
        }
    }
}

void VulkanManager::submitVirtualFramebuffer(TlFramebuffer fb)
{
    VkResult err;
    const uint32_t fenceIndex = m_currentFrameIndex.load() % MAX_FRAMES_IN_FLIGHT;
    VkFence renderFence = m_renderFences[fenceIndex];
    m_pfnDev->vkResetFences(m_device, 1, &renderFence);

    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &fb->graphicsCmdBuffers[fb->currentFrame];
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;

    err = m_pfnDev->vkQueueSubmit(getQueue(m_graphicsQID), 1, &submitInfo, renderFence);
    check_vk_result(err, "Submit Graphic Queue");
}

void VulkanManager::waitForRenderFence()
{
    const uint32_t fenceIndex = m_currentFrameIndex.load() % MAX_FRAMES_IN_FLIGHT;
    VkFence renderFence = m_renderFences[fenceIndex];
    m_pfnDev->vkWaitForFences(m_device, 1, &renderFence, VK_TRUE, UINT64_MAX);
}

void VulkanManager::waitIdle()
{
    m_pfnDev->vkDeviceWaitIdle(m_device);
}

void VulkanManager::waitForStreamingIdle()
{
    m_streamer->waitIdle(m_currentFrameIndex.load());
}

uint32_t VulkanManager::getCurrentFrameIndex() const
{
    return m_currentFrameIndex.load();
}

uint32_t VulkanManager::getSafeFrameIndex()
{
    return (m_currentFrameIndex.load() - m_maxSafeFrame);
}


//-----------------------------------------------------------------------------
//                           Transfer Functions
//-----------------------------------------------------------------------------

bool VulkanManager::loadInSimpleBuffer(SimpleBuffer& smpBuf, VkDeviceSize dataSize, const void* pData, VkDeviceSize& bufOffset, VkDeviceSize byteAlign)
{
    if (smpBuf.isLocalMem)
    {
        try {
            auto start = std::chrono::steady_clock::now();
            std::future<bool> rb = enqueueTransfer(&VulkanManager::loadInSimpleBuffer_local, this, smpBuf, dataSize, pData, bufOffset, byteAlign);
            bool result = rb.get();
            if (m_singleQueueFallback)
            {
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
                VKM_INFO << "Transfer loadInSimpleBuffer waited " << duration.count() << " ms (single-queue)." << Logger::endl;
            }
            return result;
        }
        catch (std::exception e)
        {
            // the thread that execute the queue may be stopped.
            return false;
        }
    }
    else
        return loadInSimpleBuffer_host(smpBuf, dataSize, pData, bufOffset, byteAlign);
}

bool VulkanManager::downloadSimpleBuffer(const SimpleBuffer& smpBuf, void* pData, VkDeviceSize dataSize)
{
    try {
        auto start = std::chrono::steady_clock::now();
        std::future<bool> rb = enqueueTransfer(&VulkanManager::downloadSimpleBuffer_async, this, smpBuf, pData, dataSize);
        bool result = rb.get();
        if (m_singleQueueFallback)
        {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
            VKM_INFO << "Transfer downloadSimpleBuffer waited " << duration.count() << " ms (single-queue)." << Logger::endl;
        }
        return result;
    }
    catch (std::exception e)
    {
        // the thread that execute the queue may be stopped.
        return false;
    }
}

bool VulkanManager::loadInSimpleBuffer_local(SimpleBuffer& smpBuf, VkDeviceSize dataSize, const void* pData, VkDeviceSize& bufOffset, VkDeviceSize byteAlign)
{
    // We cannot make a transfer on another thread than the transfer thread
    assert(m_transferThread.get_id() == std::this_thread::get_id());

    // Checks that the staging buffer is corretly initialize
    assert(m_stagingMem != VK_NULL_HANDLE && m_stagingBuf != VK_NULL_HANDLE);

    bufOffset = aligned(bufOffset, byteAlign);
    if (smpBuf.size < dataSize + bufOffset)
    {
        VKM_ERROR << "buffer too small for transfer.\n" << Logger::endl;
        return false;
    }

    VkResult err;

    // Map the staging buffer
    if (m_pStaging == nullptr)
    {
        err = m_pfnDev->vkMapMemory(m_device, m_stagingMem, 0, m_stagingSize, 0, &m_pStaging);
        check_vk_result(err, "Map Memory (staging)");
    }

    uint32_t subTransferCount = (uint32_t)((dataSize - 1) / m_stagingSize + 1);
    //VKM_INFO << "Loading to local memory in " << subTransferCount << " packets" << Logger::endl;
    for (uint32_t t = 0; t < subTransferCount; ++t)
    {
        VkDeviceSize subOffset = m_stagingSize * t;
        VkDeviceSize rSize = std::min(dataSize - subOffset, m_stagingSize);
        memcpy((char*)m_pStaging, (char*)pData + subOffset, rSize);

        //+++++++++++++++++++++++++++++++
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        m_pfnDev->vkBeginCommandBuffer(m_transferCmdBuf, &beginInfo);
        //+++++++++++++++++++++++++++++++

        // Copy the buffers
        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = bufOffset + subOffset;
        copyRegion.size = rSize;
        m_pfnDev->vkCmdCopyBuffer(m_transferCmdBuf, m_stagingBuf, smpBuf.buffer, 1, &copyRegion);

        //------------+++++++++++++
        m_pfnDev->vkEndCommandBuffer(m_transferCmdBuf);

        submitTransferAndWait(m_transferCmdBuf);
        //++++++++++++++++++++++++++++

        if (m_singleQueueFallback)
        {
            std::this_thread::yield();
        }
    }
    return (true);
}

bool VulkanManager::loadInSimpleBuffer_host(SimpleBuffer& smpBuf, VkDeviceSize dataSize, const void* pData, VkDeviceSize& bufOffset, VkDeviceSize byteAlign)
{
    bufOffset = aligned(bufOffset, byteAlign);
    if (smpBuf.size < dataSize + bufOffset)
    {
        VKM_ERROR << "buffer too small for transfer.\n" << Logger::endl;
        return false;
    }

    VmaAllocationInfo allocInfo;
    vmaGetAllocationInfo(m_allocator, smpBuf.alloc, &allocInfo);

    if (allocInfo.pMappedData == nullptr)
    {
        VKM_ERROR << "The simple buffer memory is not mapped." << Logger::endl;
        return false;
    }
    // copy the data to the staging buffer
    memcpy((char*)allocInfo.pMappedData + bufOffset, (char*)pData, dataSize);
    return (true);
}

bool VulkanManager::loadTextureArray_async(const void* _data, uint32_t _width, uint32_t _height, VkImage& _image, uint32_t _layer, uint32_t _layerCount)
{
    // Copy the data in the staging buffer
    if (m_pStaging == nullptr)
    {
        VkResult err = m_pfnDev->vkMapMemory(m_device, m_stagingMem, 0, m_stagingSize, 0, &m_pStaging);
        check_vk_result(err, "Map Memory (staging)");
    }

    VkCommandBuffer cmdBuf = beginTransferCommand();

    // Transition the image to a sampler layout
    VkImageMemoryBarrier barrierIn = {};
    barrierIn.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrierIn.srcAccessMask = 0;
    barrierIn.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrierIn.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrierIn.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrierIn.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierIn.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierIn.image = _image;
    barrierIn.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, _layerCount };

    m_pfnDev->vkCmdPipelineBarrier(
        cmdBuf,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrierIn);

    VkDeviceSize rowSize = 4 * (VkDeviceSize)_width;
    uint32_t rowPacket = std::min(_height, (uint32_t)(m_stagingSize / rowSize));
    if (rowPacket == 0)
        return false;

    for (uint32_t row = 0; row < _height; row += rowPacket)
    {
        uint32_t rest = std::min(rowPacket, _height - row);
        memcpy((char*)m_pStaging, (char*)_data + row * rowSize, rest * rowSize);
        VkBufferImageCopy copyRegion = {
            0,            // bufferOffset
            _width,            // bufferRowLength
            rest,            // bufferImageHeight
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, _layer, 1 },  // imageSubresource
            { 0, (int32_t)(row), 0 },  // imageOffset
            { _width, rest, 1},      // imageExtent
        };

        m_pfnDev->vkCmdCopyBufferToImage(cmdBuf, m_stagingBuf, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

        if (rowPacket < _height)
        {
            // End and wait the current transfer
            endTransferCommand(cmdBuf);
            // Start a new transfer
            cmdBuf = beginTransferCommand();
        }
    }

    // Transition the image to a sampler layout
    VkImageMemoryBarrier barrierOut = {};
    barrierOut.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrierOut.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrierOut.dstAccessMask = 0;
    barrierOut.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrierOut.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrierOut.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierOut.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierOut.image = _image;
    barrierOut.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, _layerCount };

    m_pfnDev->vkCmdPipelineBarrier(
        cmdBuf,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrierOut);

    endTransferCommand(cmdBuf);

    return (true);
}

bool VulkanManager::downloadSimpleBuffer_async(const SimpleBuffer& smpBuf, void* pData, VkDeviceSize dataSize)
{
    // We cannot make a transfer on another thread than the transfer thread
    assert(m_transferThread.get_id() == std::this_thread::get_id());

    // Checks that the staging buffer is corretly initialize
    assert(m_stagingMem != VK_NULL_HANDLE && m_stagingBuf != VK_NULL_HANDLE);

    if (dataSize < smpBuf.size)
    {
        VKM_ERROR << "buffer too small for transfer.\n" << Logger::endl;
        return false;
    }

    if (smpBuf.isLocalMem)
    {
        VkResult err;
        // Map the staging buffer
        if (m_pStaging == nullptr)
        {
            err = m_pfnDev->vkMapMemory(m_device, m_stagingMem, 0, m_stagingSize, 0, &m_pStaging);
            check_vk_result(err, "Map Memory (staging)");
        }

        uint32_t subTransferCount = (uint32_t)((dataSize - 1) / m_stagingSize + 1);
        //VKM_INFO << "Loading to local memory in " << subTransferCount << " packets" << Logger::endl;
        for (uint32_t t = 0; t < subTransferCount; ++t)
        {
            VkDeviceSize subOffset = m_stagingSize * t;
            VkDeviceSize rSize = std::min(dataSize - subOffset, m_stagingSize);

            //+++++++++++++++++++++++++++++++
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            m_pfnDev->vkBeginCommandBuffer(m_transferCmdBuf, &beginInfo);
            //+++++++++++++++++++++++++++++++

            // Copy the buffers
            VkBufferCopy copyRegion = {};
            copyRegion.srcOffset = subOffset;
            copyRegion.dstOffset = 0;
            copyRegion.size = rSize;
            m_pfnDev->vkCmdCopyBuffer(m_transferCmdBuf, smpBuf.buffer, m_stagingBuf, 1, &copyRegion);

            //------------+++++++++++++
            m_pfnDev->vkEndCommandBuffer(m_transferCmdBuf);

            submitTransferAndWait(m_transferCmdBuf);
            //++++++++++++++++++++++++++++

            VkMappedMemoryRange range = {};
            range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range.memory = m_stagingMem;
            range.offset = 0;
            range.size = VK_WHOLE_SIZE;
            m_pfnDev->vkFlushMappedMemoryRanges(m_device, 1, &range);
            memcpy((char*)pData + subOffset, (char*)m_pStaging, rSize);

            if (m_singleQueueFallback)
            {
                std::this_thread::yield();
            }
        }
    }
    else
    {
        VmaAllocationInfo allocInfo;
        vmaGetAllocationInfo(m_allocator, smpBuf.alloc, &allocInfo);

        if (allocInfo.pMappedData == nullptr)
        {
            VKM_ERROR << "The simple buffer memory is not mapped." << Logger::endl;
            return false;
        }
        // copy the data to the staging buffer
        memcpy((char*)pData, (char*)allocInfo.pMappedData, dataSize);
    }
    return (true);
}


void* VulkanManager::getMappedPointer(SmartBuffer& sbuf)
{
    if (sbuf.alloc == nullptr)
        return 0;

    VmaAllocationInfo allocInfo;
    vmaGetAllocationInfo(m_allocator, sbuf.alloc, &allocInfo);

#ifdef _DEBUG_
    if (allocInfo.pMappedData == nullptr)
        VKM_ERROR << "The buffer memory is not mapped." << Logger::endl;
#endif

    return (allocInfo.pMappedData);
}

bool VulkanManager::submitTransferAndWait(VkCommandBuffer cmdBuffer)
{
    VkFence fence = VK_NULL_HANDLE;
    if (m_singleQueueFallback)
    {
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkResult err = m_pfnDev->vkCreateFence(m_device, &fenceInfo, nullptr, &fence);
        if (err != VK_SUCCESS)
            return false;
    }

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;

    VkResult err = m_pfnDev->vkQueueSubmit(getQueue(m_transferQID), 1, &submitInfo, fence);
    if (err != VK_SUCCESS)
    {
        if (fence != VK_NULL_HANDLE)
            m_pfnDev->vkDestroyFence(m_device, fence, nullptr);
        return false;
    }

    if (m_singleQueueFallback)
    {
        m_pfnDev->vkWaitForFences(m_device, 1, &fence, VK_TRUE, UINT64_MAX);
        m_pfnDev->vkDestroyFence(m_device, fence, nullptr);
    }
    else
    {
        m_pfnDev->vkQueueWaitIdle(getQueue(m_transferQID));
    }

    return true;
}

bool VulkanManager::allocUniform(uint32_t dataSize, uint32_t imageCount, VkMultiUniform& uniform)
{
    return m_uniformAllocator.alloc(dataSize, imageCount, uniform);
}

// inline ??
bool VulkanManager::allocUniform(uint32_t dataSize, TlFramebuffer framebuffer, VkMultiUniform& uniform)
{
    return m_uniformAllocator.alloc(dataSize, framebuffer->imageCount, uniform);
}

uint32_t VulkanManager::getUniformSizeAligned(uint32_t dataSize)
{
    return (uint32_t)aligned(dataSize, m_physDevProperties.limits.minUniformBufferOffsetAlignment);
}

// inline ??
void VulkanManager::freeUniform(VkMultiUniform& uniform)
{
    m_uniformAllocator.free(uniform);
}

void VulkanManager::loadUniformData(uint32_t dataSize, const void* pData, uint32_t localOffset, uint32_t swapIndex, const VkMultiUniform& uniform) const
{
    // Prerequisites:
    // - dataSize must not be equal to zero
    // - the data size to upload must not be superior to the uniform size
    // - frameIndex must be inferior to the offsets count
    assert(dataSize);
    assert(uniform.size >= (dataSize + localOffset));
    assert(uniform.frameCount > swapIndex);
    uint32_t frameOffset = uniform.offset + uniform.size * swapIndex;
    void* p;
    VkResult err = m_pfnDev->vkMapMemory(m_device, m_uniformMem, frameOffset, uniform.size, 0, &p);
    check_vk_result(err, "Map Uniform Memory");

    memcpy((char*)p + localOffset, pData, dataSize);

    m_pfnDev->vkUnmapMemory(m_device, m_uniformMem);
}

void VulkanManager::loadConstantUniform(uint32_t dataSize, const void* pData, VkMultiUniform& uniform)
{
    // Prerequisites:
    // - dataSize must not be equal to zero
    // - the data size to upload must not be superior to the uniform size
    assert(dataSize);
    assert(uniform.size >= (dataSize));
    void* p;
    VkResult err = m_pfnDev->vkMapMemory(m_device, m_uniformMem, uniform.offset, (uint64_t)uniform.size * uniform.frameCount, 0, &p);
    check_vk_result(err, "Map Uniform Memory");

    for (uint64_t i = 0; i < uniform.frameCount; ++i)
        memcpy((char*)p + i * uniform.size, pData, dataSize);

    m_pfnDev->vkUnmapMemory(m_device, m_uniformMem);
}

VkResult VulkanManager::allocateMemory(VkDeviceMemory& memory, VkMemoryRequirements requirements, VkMemoryPropertyFlags properties, const char* logName) const
{
    // Find an adequate memory type
    uint32_t memTypeId = findMemoryType(m_physDev, properties);

    // NOTE - VkMemoryRequirements indicate a pre-selection of the memType acceptable.
    /*
    VkPhysicalDeviceMemoryProperties memProperties;
    m_pfn.vkGetPhysicalDeviceMemoryProperties(m_physDev, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((requirements.memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            memTypeId = i;
            break;
        }
    }*/

    if (memTypeId == UINT32_MAX)
        check_vk_result(VK_ERROR_OUT_OF_DEVICE_MEMORY, "Memory Type not found");

    VkMemoryAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        requirements.size,
        memTypeId
    };

    // TODO - try to allocate on all memType acceptable
    return m_pfnDev->vkAllocateMemory(m_device, &allocInfo, nullptr, &memory);
}

// Warning - heavy load
void VulkanManager::printAllocationStats(std::string& log)
{
    log.clear();

    VmaDetailedStatistics stats;
    vmaCalculatePoolStatistics(m_allocator, m_localPool, &stats);

    log += "Block count:           " + std::to_string(stats.statistics.blockCount) + "\n";
    log += "Allocation count:      " + std::to_string(stats.statistics.allocationCount) + "\n";
    log += "Block size:            " + std::to_string(stats.statistics.blockBytes) + " bytes\n";
    log += "Allocations size:      " + std::to_string(stats.statistics.allocationBytes) + " bytes\n";
    log += "Unused Size:           " + std::to_string(stats.statistics.blockBytes - stats.statistics.allocationBytes) + "\n";
    log += "Unused range count:    " + std::to_string(stats.unusedRangeCount) + "\n";
    log += "Unused range min size: " + std::to_string(stats.unusedRangeSizeMin) + "\n";
    log += "Unused range max size: " + std::to_string(stats.unusedRangeSizeMax) + "\n";
}

//-------------------------------------------------------------------
//              NEW - smart buffer system - NEW
//-------------------------------------------------------------------

VkResult VulkanManager::tryAllocBuffer(VkBufferCreateInfo& bufferCreateInfo, VmaAllocationCreateInfo& allocationCreateInfo, SimpleBuffer* sbuf)
{
    bool tryAlloc = true;
    VkResult err = VK_ERROR_UNKNOWN;
    while (err < 0 && tryAlloc)
    {
        err = vmaCreateBuffer(m_allocator, &bufferCreateInfo, &allocationCreateInfo, &(sbuf->buffer), &(sbuf->alloc), nullptr);
        // No hard check result, if not enough memory available try to free more
        if (err < 0)
        {
            if (sbuf->isLocalMem)
                tryAlloc = freeDeviceMemory(bufferCreateInfo.size);
            else
                tryAlloc = freeHostMemory(bufferCreateInfo.size);
        }
    }

    if (err < 0)
    {
        if (sbuf->isLocalMem)
            m_missedDeviceAllocations++;
        else
            m_missedHostAllocations++;
        return err;
    }
    sbuf->size = bufferCreateInfo.size;
    return err;
}

VkResult VulkanManager::allocSmartBuffer(VkDeviceSize dataSize, SmartBuffer& sbuf, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags)
{
    // New allocation must be made on an empty buffer
    assert(sbuf.buffer == VK_NULL_HANDLE || sbuf.alloc == VK_NULL_HANDLE);

    VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = dataSize;
    bufferInfo.usage = usageFlags;

    bool isLocal = (propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0;

    VmaAllocationCreateInfo allocCI = {};
    allocCI.flags = 0;
    // allocCI.usage is ignored if we specify the pool
    sbuf.isLocalMem = isLocal;
    if (isLocal)
    {
        bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        allocCI.pool = m_localPool;
    }
    else
    {
        allocCI.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        allocCI.pool = m_hostPool;
    }

    VkResult err = VK_ERROR_UNKNOWN;
    if ((err = tryAllocBuffer(bufferInfo, allocCI, &sbuf)) != VK_SUCCESS)
        return err;

    // Actualize the buffer size after allocation
    VmaAllocationInfo allocInfo;
    vmaGetAllocationInfo(m_allocator, sbuf.alloc, &allocInfo);
    sbuf.size = allocInfo.size;

    {
        std::lock_guard<std::mutex> lock(m_mutexBufferAllocated);
        m_smartBufferAllocated.insert(&sbuf);
    }
    if (isLocal)
        m_pointsDevicePoolUsed += sbuf.size;
    else
        m_pointsHostPoolUsed += sbuf.size;

    return err;
}

VkResult VulkanManager::allocSimpleBuffer(VkDeviceSize dataSize, SimpleBuffer& sbuf, VkBufferUsageFlags usageFlags)
{
    assert(dataSize && "Cannot allocate a buffer with null size");
    if (sbuf.buffer != VK_NULL_HANDLE || sbuf.alloc != VK_NULL_HANDLE)
    {
        VKM_ERROR << "New allocation on an already allocated buffer\n" << Logger::endl;
        assert(0);
        return VkResult::VK_ERROR_DEVICE_LOST;
    }

    VkResult err = VK_ERROR_UNKNOWN;
    VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = dataSize;
    bufferInfo.usage = usageFlags;

    VmaAllocationCreateInfo allocCI = {};
    if ((usageFlags & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) == 0)
    {
        allocCI.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        allocCI.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        allocCI.pool = m_hostPool;
        sbuf.isLocalMem = false;
        if ((err = tryAllocBuffer(bufferInfo, allocCI, &sbuf)) != VK_SUCCESS)
            return err;
    }
    else 
    {
        // NOTE(robin) - TRANSFER_SRC est uniquement utile pour le calcul de distance sur mesh CPU.
        // NOTE(robin) - On peut ajouter STORAGE_BUFFER pour l'utiliser dans le compute shader
        bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        allocCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        allocCI.pool = m_localPool;
        sbuf.isLocalMem = true;
        if((err = tryAllocBuffer(bufferInfo, allocCI, &sbuf)) != VK_SUCCESS)
            return err;
    }

    // Actualize the buffer size after allocation
    VmaAllocationInfo allocInfo;
    vmaGetAllocationInfo(m_allocator, sbuf.alloc, &allocInfo);
    sbuf.size = allocInfo.size;

    {
        std::lock_guard<std::mutex> lock(m_mutexBufferAllocated);
        m_simpleBufferAllocated.insert(&sbuf);
    }

    if (sbuf.isLocalMem)
        m_objectsDevicePoolUsed += sbuf.size;
    else
        m_objectsHostPoolUsed += sbuf.size;

    return err;
}

bool VulkanManager::touchSmartBuffer(SmartBuffer& sbuf)
{
    uint32_t localCurrFI = m_currentFrameIndex.load();
    uint32_t localLastUseFI = sbuf.lastUseFrameIndex.load();

    while (true)
    {
        if (localLastUseFI == TL_FRAME_INDEX_LOST)
            return false;
        else if (sbuf.lastUseFrameIndex.compare_exchange_weak(localLastUseFI, localCurrFI))
            break;
    }

    return (sbuf.state.load() == TlDataState::LOADED);
}

bool VulkanManager::lockSmartBuffer(SmartBuffer& sbuf)
{
    uint32_t processCount = sbuf.ongoingProcesses.load();

    while (true)
    {
        if (processCount == TL_PROCESS_LOST)
            return false;
        else if (sbuf.ongoingProcesses.compare_exchange_weak(processCount, processCount + 1))
            return true;
    }
}

bool VulkanManager::unlockSmartBuffer(SmartBuffer& sbuf)
{
    uint32_t processCount = sbuf.ongoingProcesses.load();

    while (true)
    {
        if (processCount == 0)
            return false;
        else if (sbuf.ongoingProcesses.compare_exchange_weak(processCount, processCount - 1))
            return true;
    }
}

bool VulkanManager::safeFreeAllocation(SmartBuffer& sbuf)
{
    uint32_t localCurrFI = m_currentFrameIndex.load();
    uint32_t localLastUseFI = sbuf.lastUseFrameIndex.load();
    uint32_t processCount = sbuf.ongoingProcesses.load();

    while (true)
    {
        bool frameSafe = false;
        bool processSafe = false;
        // Check if the last buffer use is older than the oldest frame draw
        if (localLastUseFI >= localCurrFI - m_maxSafeFrame)
            return false;

        if (processCount > 0)
            return false;

        // Try to make it lost
        if (sbuf.lastUseFrameIndex.compare_exchange_weak(localLastUseFI, TL_FRAME_INDEX_LOST))
            frameSafe = true;

        if (sbuf.ongoingProcesses.compare_exchange_weak(processCount, TL_PROCESS_LOST))
            processSafe = true;



        if (frameSafe && processSafe)
        {
            freeAllocation(sbuf);
            return true;
        }
        else if (frameSafe)
        {
            // Put back the previous value
            sbuf.lastUseFrameIndex.store(localLastUseFI);
        }
        else if (processSafe)
        {
            // Put back the previous value
            sbuf.ongoingProcesses.store(processCount);
        }
    }
}

void VulkanManager::freeAllocation(SmartBuffer& sbuf)
{
    sbuf.state.store(TlDataState::NOT_LOADED);
    //assert(sbuf.alloc != nullptr && sbuf.buffer != VK_NULL_HANDLE);
    if (sbuf.alloc == VK_NULL_HANDLE)
        return;

    vmaDestroyBuffer(m_allocator, sbuf.buffer, sbuf.alloc);
    {
        std::lock_guard<std::mutex> lock(m_mutexBufferAllocated);
        m_smartBufferAllocated.erase(&sbuf);
    }
    if (sbuf.isLocalMem)
        m_pointsDevicePoolUsed -= sbuf.size;
    else
        m_pointsHostPoolUsed -= sbuf.size;

    sbuf.lastUseFrameIndex.store(0);
    sbuf.ongoingProcesses.store(0);
    sbuf.alloc = VK_NULL_HANDLE;
    sbuf.buffer = VK_NULL_HANDLE;
    sbuf.size = 0;
}

void VulkanManager::freeAllocation(SimpleBuffer& sbuf)
{
    if (sbuf.alloc == VK_NULL_HANDLE)
        return;

    assert(sbuf.buffer != VK_NULL_HANDLE && sbuf.size != 0);
    vmaDestroyBuffer(m_allocator, sbuf.buffer, sbuf.alloc);
    {
        std::lock_guard<std::mutex> lock(m_mutexBufferAllocated);
        m_simpleBufferAllocated.erase(&sbuf);
    }
    if (sbuf.isLocalMem)
        m_objectsDevicePoolUsed -= sbuf.size;
    else
        m_objectsHostPoolUsed -= sbuf.size;


    sbuf.alloc = VK_NULL_HANDLE;
    sbuf.buffer = VK_NULL_HANDLE;
    sbuf.size = 0;
}

bool VulkanManager::freeDeviceMemory(VkDeviceSize minMemoryNeeded)
{
    if (m_noMoreFreeMemoryForFrame_device.load())
        return false;

    VkDeviceSize usageNeeded = minMemoryNeeded < m_localPoolMaxSize / 20 || minMemoryNeeded > m_localPoolMaxSize ? m_localPoolMaxSize / 20 : minMemoryNeeded;
    VkDeviceSize usageStart = m_pointsDevicePoolUsed + m_objectsDevicePoolUsed;
    VkDeviceSize usageTarget = usageStart > usageNeeded ? usageStart - usageNeeded : 0;

    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    std::multimap<uint32_t, SmartBuffer*> frameSortedBuffers;

    uint32_t localCurrFI = m_currentFrameIndex.load();
    {
        std::lock_guard<std::mutex> lock(m_mutexBufferAllocated);
        // Sort the buffers available for release
        for (auto sbuf : m_smartBufferAllocated)
        {
            if (!sbuf->isLocalMem)
                continue;

            if (sbuf->ongoingProcesses.load() > 0)
                continue;

            if (sbuf->lastUseFrameIndex.load() >= localCurrFI - m_maxSafeFrame)
                continue;

            if (sbuf->state.load() != TlDataState::LOADED)
                continue;

            frameSortedBuffers.insert({ sbuf->lastUseFrameIndex.load(), sbuf });
        }
    }

    // Try to free the buffers until the new used space target is reached
    for (auto frame_sbuf : frameSortedBuffers)
    {
        // The memory used is actualized after each freeAllocation()
        if (m_pointsDevicePoolUsed + m_objectsDevicePoolUsed <= usageTarget)
            break;

        safeFreeAllocation(*(frame_sbuf.second));
    }

    if (m_pointsDevicePoolUsed + m_objectsDevicePoolUsed > usageTarget)
        m_noMoreFreeMemoryForFrame_device.store(true);

    // Stats
    float time = std::chrono::duration<float, std::ratio<1>>(std::chrono::steady_clock::now() - start).count();
    VkDeviceSize usageFreed = usageStart - (m_pointsDevicePoolUsed + m_objectsDevicePoolUsed);
    VkDeviceSize usageFreedMb = usageFreed / 1048576;
    float speed = usageFreedMb / time;
    VKM_INFO << "Device Memory freed " << usageFreedMb << " Mb in " << time << " s (" << speed << " Mb/s)" << Logger::endl;

    return (minMemoryNeeded <= usageFreed);
}

bool VulkanManager::freeHostMemory(VkDeviceSize memoryNeeded)
{
    if (m_noMoreFreeMemoryForFrame_host.load())
        return false;

    VkDeviceSize usageNeeded = memoryNeeded < m_hostPoolMaxSize / 20 || memoryNeeded > m_hostPoolMaxSize ? m_hostPoolMaxSize / 20 : memoryNeeded;
    VkDeviceSize usageStart = m_pointsHostPoolUsed + m_objectsHostPoolUsed;
    VkDeviceSize usageTarget = usageStart > usageNeeded ? usageStart - usageNeeded : 0;

    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    std::multimap<uint32_t, SmartBuffer*> frameSortedBuffers;

    uint32_t localCurrFI = m_currentFrameIndex.load();
    {
        std::lock_guard<std::mutex> lock(m_mutexBufferAllocated);
        // Sort the buffers available for release
        for (auto sbuf : m_smartBufferAllocated)
        {
            if (sbuf->isLocalMem)
                continue;

            if (sbuf->ongoingProcesses.load() > 0)
                continue;

            if (sbuf->lastUseFrameIndex.load() >= localCurrFI - m_maxSafeFrame)
                continue;

            if (sbuf->state.load() != TlDataState::LOADED)
                continue;

            frameSortedBuffers.insert({ sbuf->lastUseFrameIndex.load(), sbuf });
        }
    }

    for (auto frame_sbuf : frameSortedBuffers)
    {
        // The mem used is actualized by each freeAllocation()
        if (m_pointsHostPoolUsed + m_objectsHostPoolUsed <= usageTarget)
            break;

        safeFreeAllocation(*(frame_sbuf.second));
    }

    if (m_pointsHostPoolUsed + m_objectsHostPoolUsed > usageTarget)
        m_noMoreFreeMemoryForFrame_host.store(true);

    // Stats
    float time = std::chrono::duration<float, std::ratio<1>>(std::chrono::steady_clock::now() - start).count();
    VkDeviceSize usageFreed = usageStart - (m_pointsHostPoolUsed + m_objectsHostPoolUsed);
    VkDeviceSize usageFreedMb = usageFreed / 1048576;
    float speed = usageFreedMb / time;
    if (usageFreedMb > 0)
        VKM_INFO << "Host Memory freed " << usageFreedMb << " Mb in " << time << " s (" << speed << " Mb/s)" << Logger::endl;
    else
        VKM_INFO << "Host Memory freed " << usageFreed << "bytes in " << time << Logger::endl;

    return (memoryNeeded <= usageFreed);
}

//-----------------------------------------------------------------------------
//                               Texture
//-----------------------------------------------------------------------------

bool VulkanManager::createTextureArray(uint32_t _width, uint32_t _height, uint32_t _layerCount, VkImage& _image, VkImageView& _imageView, VkSampler& _sampler, VkDeviceMemory& _memory)
{
    // Check that the physical device can store enough layers
    VkPhysicalDeviceLimits limits = getPhysicalDeviceLimits();
    if (_layerCount > limits.maxImageArrayLayers)
    {
        Logger::log(LoggerMode::VKLog) << "Cannot create a texture sample with " << _layerCount << " layers, max supported is " << limits.maxImageArrayLayers << Logger::endl;
        return false;
    }

    createImage({ _width, _height }, _layerCount, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _image, _memory);

    VkImageViewCreateInfo viewInfo = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,                      // pNext
        0,                            // flags
        _image,                       // image
        VK_IMAGE_VIEW_TYPE_2D_ARRAY,  // viewType
        VK_FORMAT_B8G8R8A8_UNORM,     // format
        {   // components
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY
        },
        { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, _layerCount}  // SubresourceRange
    };

    VkResult err = m_pfnDev->vkCreateImageView(m_device, &viewInfo, nullptr, &_imageView);
    check_vk_result(err, "Create Image View");

    _sampler = createTextureSampler();

    return (true);
}

bool VulkanManager::uploadTextureArray(const void* _data, uint32_t _width, uint32_t _height, VkImage& _image, uint32_t _layer, uint32_t _layerCount)
{
    try {
        std::future<bool> rb = enqueueTransfer(&VulkanManager::loadTextureArray_async, this, _data, _width, _height, _image, _layer, _layerCount);
        return rb.get();
    }
    catch (std::exception e)
    {
        // the thread that execute the queue may be stopped.
        return false;
    }
}


//-----------------------------------------------------------------------------
//                               Benchmark
//-----------------------------------------------------------------------------

void VulkanManager::startRecordingStats()
{
    if (m_streamer)
    {
        m_streamer->startRecordingFrame();
    }
}

void VulkanManager::stopRecordingStats()
{
    if (m_streamer)
    {
        m_streamer->stopRecordingFrame();
    }
}


//-----------------------------------------------------------------------------
//                      Main Initialization Functions
//-----------------------------------------------------------------------------

bool VulkanManager::createVulkanInstance()
{
    checkValidationLayerSupport();
    checkInstanceExtensionSupport();

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "OpenScanTools";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.pEngineName = "Find a name";
    appInfo.engineVersion = TL_ENGINE_VERSION;
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = m_layerCount;
    createInfo.ppEnabledLayerNames = m_layers;
    createInfo.enabledExtensionCount = m_extCount;
    createInfo.ppEnabledExtensionNames = m_extensions;

    VkResult err = m_pfn.vkCreateInstance(&createInfo, nullptr, &m_vkInstance);
    check_vk_result(err, "Create Vulkan Instance");

    m_pfn.loadInstanceFunctions(m_vkInstance);

    return (err == VkResult::VK_SUCCESS);
}

bool VulkanManager::setupDebugCallback()
{
    if (m_layerCount == 0) return true;

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;

    VkResult err = m_pfn.vkCreateDebugUtilsMessengerEXT(m_vkInstance, &createInfo, nullptr, &m_debugCallback);
    check_vk_result(err, "Create Debug Utils Messenger");
    return err == VkResult::VK_SUCCESS;
}

bool VulkanManager::selectPhysicalDevice(std::string preferedDevice, std::unordered_map<uint32_t, std::string>& compliantDevices)
{
    // The VkInstance must be initialized first
    if (m_vkInstance == VK_NULL_HANDLE)
        return false;

    bool preferedDeviceFound = false;

    // Enumeration of the physical devices
    uint32_t devCount = 0;
    m_pfn.vkEnumeratePhysicalDevices(m_vkInstance, &devCount, nullptr);
    if (devCount == 0) {
        VKM_ERROR << "No physical device with Vulkan support found.\n";
        return false;
    }

    VkPhysicalDevice* physDevs = new VkPhysicalDevice[devCount];
    m_pfn.vkEnumeratePhysicalDevices(m_vkInstance, &devCount, physDevs);

    VkPhysicalDeviceProperties* physDevProps = new VkPhysicalDeviceProperties[devCount];
    Logger::log(VK_LOG) << "Info - " << devCount << " physical devices found with Vulkan support." << Logger::endl;
    for (uint32_t i = 0; i < devCount; i++) {
        m_pfn.vkGetPhysicalDeviceProperties(physDevs[i], &physDevProps[i]);
        Logger::log(VK_LOG) << "Physical device [" << i << "]: " << physDevProps[i].deviceName << " | driver: " << nvidiaVersionToStr(physDevProps[i].driverVersion) << " | Vulkan API: " << vkVersionToStr(physDevProps[i].apiVersion) << Logger::endl;
        if (isDeviceSuitable(physDevs[i]))
        {
            compliantDevices.insert({ i, physDevProps[i].deviceName });
            if (preferedDevice == physDevProps[i].deviceName)
            {
                preferedDeviceFound = true;
                m_physDev = physDevs[i];
            }
        }
    }

    // cleanup
    delete[] physDevs;
    delete[] physDevProps;

    // selectFeatures and various log is done in initPhysicalDevice()

    return preferedDeviceFound;
}

/*
Initialize the logical device and the queues associated with it
/**/
bool VulkanManager::createLogicalDevice()
{
    if (m_device) {
        VKM_WARNING << "Cannot recreate a logical device before destroying the first one.\n"
            << Logger::endl;
        return true;
    }

    checkQueueFamilies(m_physDev, true, false);

    // --- Queue creation --- NEW STRATEGY !!!
    // + Create all the queue available
    // + Decide later which one is suitable for presentation of a surface
    VkDeviceQueueCreateInfo* pQueueInfo = new VkDeviceQueueCreateInfo[m_queueFamilyCount];

    // All the queues have the same priority
    std::vector<std::vector<float>> priorities(m_queueFamilyCount, std::vector<float>());

    for (uint32_t i = 0; i < m_queueFamilyCount; ++i)
    {
        priorities[i] = std::vector<float>(m_queueFamilyProps[i].queueCount, 1.f);

        pQueueInfo[i] = {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            NULL,            // pNext
            0,               // flags
            i,               // queueFamilyIndex
            m_queueFamilyProps[i].queueCount,  // queueCount
            priorities[i].data()                // qQueuePriorities
        };
    }

    // Creation of the device
    VkDeviceCreateInfo devInfo;
    memset(&devInfo, 0, sizeof(devInfo));
    devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devInfo.queueCreateInfoCount = m_queueFamilyCount;
    devInfo.pQueueCreateInfos = pQueueInfo;
    devInfo.enabledExtensionCount = m_devExtCount;
    devInfo.ppEnabledExtensionNames = m_devExtensions;
    devInfo.pEnabledFeatures = &m_features;
    VkResult err = m_pfn.vkCreateDevice(m_physDev, &devInfo, nullptr, &m_device);
    check_vk_result(err, "Create Logical Device");

    delete[] pQueueInfo;

    // Initialize the Device functions
    m_pfnDev = new VulkanDeviceFunctions(m_vkInstance, m_device, m_pfn.vkGetInstanceProcAddr);

    m_ppQueue = new VkQueue *[m_queueFamilyCount];
    for (uint32_t i = 0; i < m_queueFamilyCount; ++i)
    {
        m_ppQueue[i] = new VkQueue[m_queueFamilyProps[i].queueCount];
        for (uint32_t j = 0; j < m_queueFamilyProps[i].queueCount; ++j)
        {
            m_pfnDev->vkGetDeviceQueue(m_device, i, j, (m_ppQueue[i] + j));
        }
    }

    return err == VkResult::VK_SUCCESS;
}

bool VulkanManager::initVma()
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = m_physDev;
    allocatorInfo.device = m_device;
    allocatorInfo.instance = m_vkInstance;

    VmaVulkanFunctions VmaFunctions =
    {
        m_pfn.vkGetInstanceProcAddr,
        m_pfn.getDeviceProcAddr(m_device),
        m_pfn.vkGetPhysicalDeviceProperties,
        m_pfn.vkGetPhysicalDeviceMemoryProperties,
        m_pfnDev->vkAllocateMemory,
        m_pfnDev->vkFreeMemory,
        m_pfnDev->vkMapMemory,
        m_pfnDev->vkUnmapMemory,
        m_pfnDev->vkFlushMappedMemoryRanges,
        m_pfnDev->vkInvalidateMappedMemoryRanges,
        m_pfnDev->vkBindBufferMemory,
        m_pfnDev->vkBindImageMemory,
        m_pfnDev->vkGetBufferMemoryRequirements,
        m_pfnDev->vkGetImageMemoryRequirements,
        m_pfnDev->vkCreateBuffer,
        m_pfnDev->vkDestroyBuffer,
        m_pfnDev->vkCreateImage,
        m_pfnDev->vkDestroyImage,
        m_pfnDev->vkCmdCopyBuffer
    };

    allocatorInfo.pVulkanFunctions = &VmaFunctions;

    vmaCreateAllocator(&allocatorInfo, &m_allocator);

    checkMemoryType(m_physDev, true, false);
    VkPhysicalDeviceMemoryProperties memProp;
    m_pfn.vkGetPhysicalDeviceMemoryProperties(m_physDev, &memProp);

    VkDeviceSize memSizeDevice = memProp.memoryHeaps[m_deviceHeapIndex].size;
    VkDeviceSize memSizeHost = memProp.memoryHeaps[m_hostHeapIndex].size;
    VkDeviceSize blockSize = 256ull * 1024 * 1024;
    double defaultFraction = m_singleQueueFallback ? 0.30 : 0.90;
    double deviceFraction = readEnvDouble("OST_VMA_DEVICE_POOL_FRACTION", defaultFraction);
    double hostFraction = readEnvDouble("OST_VMA_HOST_POOL_FRACTION", defaultFraction);
    double poolFraction = readEnvDouble("OST_VMA_POOL_FRACTION", -1.0);
    if (poolFraction > 0.0)
    {
        deviceFraction = poolFraction;
        hostFraction = poolFraction;
    }
    deviceFraction = std::min(std::max(deviceFraction, 0.01), 0.95);
    hostFraction = std::min(std::max(hostFraction, 0.01), 0.95);

    size_t blockCountDevice = static_cast<size_t>((memSizeDevice * deviceFraction) / blockSize);
    size_t blockCountHost = static_cast<size_t>((memSizeHost * hostFraction) / blockSize);
    blockCountDevice = std::max<size_t>(1, blockCountDevice);
    blockCountHost = std::max<size_t>(1, blockCountHost);

    VKM_INFO << "Local Memory Selection: type index " << m_memTypeIndex_local << " with " << memSizeDevice << " bytes available." << Logger::endl;
    VKM_INFO << "Host Memory Selection: type index " << m_memTypeIndex_host << " with " << memSizeHost << " bytes available." << Logger::endl;
    VKM_INFO << "Maximum Device memory used (< " << deviceFraction * 100.0 << "%): " << blockCountDevice * blockSize << Logger::endl;
    VKM_INFO << "Maximum Host memory used (< " << hostFraction * 100.0 << "%): " << blockCountHost * blockSize << Logger::endl;

    VmaPoolCreateInfo poolCI = {};
    poolCI.memoryTypeIndex = m_memTypeIndex_local;
    poolCI.blockSize = blockSize;
    poolCI.maxBlockCount = blockCountDevice;
    poolCI.flags = 0;

    VkResult err = vmaCreatePool(m_allocator, &poolCI, &m_localPool);
    check_vk_result(err, "Create Device Memory Pool");
    m_localPoolMaxSize = blockSize * blockCountDevice;

    poolCI.memoryTypeIndex = m_memTypeIndex_host;
    poolCI.blockSize = blockSize;
    poolCI.maxBlockCount = blockCountHost;
    poolCI.flags = 0;

    err = vmaCreatePool(m_allocator, &poolCI, &m_hostPool);
    check_vk_result(err, "Create Host Memory Pool");
    m_hostPoolMaxSize = blockSize * blockCountHost;

    return err == VkResult::VK_SUCCESS;
}

void VulkanManager::findDepthFormat()
{
    // The Vulkan Spec require that at least one of VK_FORMAT_X8_D24_UNORM_PACK32 and VK_FORMAT_D32_SFLOAT must be supported
    VkFormat preferedFormat[] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_X8_D24_UNORM_PACK32
    };

    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    for (uint32_t i = 0; i < sizeof(preferedFormat) / sizeof(preferedFormat[0]); i++) {
        VkFormatProperties props;
        m_pfn.vkGetPhysicalDeviceFormatProperties(m_physDev, preferedFormat[i], &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            m_depthFormat = preferedFormat[i];
            return;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            m_depthFormat = preferedFormat[i];
            return;
        }
    }

    check_vk_result(VK_ERROR_FORMAT_NOT_SUPPORTED, "Find Depth Format");
}

// Render pass for the point cloud drawing. But we copy the last point cloud drawing in the color and depth attachment at the beginning of the render pass
// 2 attachments
// 1 subpasses
bool VulkanManager::createPCRenderPass(VkFormat imageFormat)
{
    // Check if the render pass is already created
    if (m_renderPass_pc.find(imageFormat) != m_renderPass_pc.end())
        return true;

    VkAttachmentDescription pcColorAttachment = {
        0,
        imageFormat,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_GENERAL
    };

    VkAttachmentDescription pcDepthAttachment = {
        0,
        m_depthFormat,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
    };

    VkAttachmentReference pcColorAttachmentRef = {
        0,                                         // attachment
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL   // layout
    };

    VkAttachmentReference pcDepthAttachmentRef = {
        1,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpasses[] = {
        {
            0,                                // flags
            VK_PIPELINE_BIND_POINT_GRAPHICS,  // pipelineBindPoint
            0,                                // inputAttachmentCount
            nullptr,                          // pInputAttachments
            1,                                // colorAttachmentCount
            &pcColorAttachmentRef,            // pColorAttachments
            nullptr,                          // pResolveAttachments
            &pcDepthAttachmentRef,            // pDepthStencilAtachment
            0,                                // preserveAttachmentCount
            nullptr                           // pPreserveAttachments
        }
    };

    VkSubpassDependency dependency[] = {
        {
            VK_SUBPASS_EXTERNAL,  // srcSubpass
            0,                    // dstSubpass
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // srcStageMask
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // dstStageMask
            0,   // srcAccessMask
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, // dstAccessMask
            0
        }
    };

    VkAttachmentDescription attachments[] = { pcColorAttachment, pcDepthAttachment };
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 2;  // color + depth
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = subpasses;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = dependency;

    VkRenderPass rp;
    VkResult err = m_pfnDev->vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &rp);
    check_vk_result(err, "Create Point Cloud Render Pass");
    m_renderPass_pc.insert({ imageFormat, rp });
    return err == VkResult::VK_SUCCESS;
}

// Render pass for objects drawing
bool VulkanManager::createObjectRenderPasses()
{
    // Prerequisite
    if (m_depthFormat == VK_FORMAT_UNDEFINED || m_imageFormat == VK_FORMAT_UNDEFINED) {
        check_vk_result(VK_ERROR_FORMAT_NOT_SUPPORTED, "Depth & Color format undefined");
    }

    VkAttachmentDescription mainColorDesc = {
        0,                                        // flags
        m_imageFormat,                            // format
        VK_SAMPLE_COUNT_1_BIT,                    // samples
        VK_ATTACHMENT_LOAD_OP_LOAD,               // loadOp
        VK_ATTACHMENT_STORE_OP_STORE,             // storeOp
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,          // stencilLoadOp
        VK_ATTACHMENT_STORE_OP_DONT_CARE,         // stencilStoreOp
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // initialLayout
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR           // finalLayout
    };

    VkAttachmentDescription mainDepthDesc = {
        0,
        m_depthFormat,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_LOAD,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
    };

    VkAttachmentDescription indexDesc = {
        0,
        VK_FORMAT_R32_UINT,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
    };

    VkAttachmentDescription transpColorDesc = {
        0,
        //m_imageFormat,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkAttachmentDescription gizmoDepthDesc = {
        0,
        m_depthFormat,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_DONT_CARE, // not used outside the subpass
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED, // can be DEPTH_STENCIL
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };


    VkAttachmentReference mainColorRef = {
        0,                                         // attachment
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL   // layout
    };

    VkAttachmentReference mainDepthRef = {
        1,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkAttachmentReference indexRef = {
        2,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkAttachmentReference transpWriteRef = {
        3,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkAttachmentReference transpReadRef = {
        3,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkAttachmentReference gizmoDepthRef = {
        4,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkAttachmentReference colorIndexRefs[] = { mainColorRef, indexRef };
    VkAttachmentReference transpIndexRefs[] = { transpWriteRef, indexRef };

    VkSubpassDescription subpasses[] = {
        // Subpass for the opaque objects (meshes, lines)
        {
            0,                               // flags
            VK_PIPELINE_BIND_POINT_GRAPHICS, // pipelineBindPoint
            0,                               // inputAttachmentCount
            nullptr,                         // pInputAttachments
            2,                               // colorAttachmentCount
            colorIndexRefs,                  // pColorAttachments
            nullptr,                         // pResolveAttachments
            &mainDepthRef,                   // pDepthStencilAtachment
            0,                               // preserveAttachmentCount
            nullptr                          // pPreserveAttachments
        },
        // Subpass for the transparent objects (meshes, boxes, cylinder)
        {
            0,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            0,
            nullptr,
            2,
            transpIndexRefs,
            nullptr,
            &mainDepthRef,
            0,
            nullptr
        },
        // OPTIONAL - Add a subpass to decompose the transparent rendering in 2 steps :
        //  (1) Render the front of the transparent objects and store the front depth
        //  (2) Render the middle of the transparent objects and test against the front depth

        // Subpass for blending the first subpass and the second subpass (opaque + transparent)
        {
            0,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            1,
            &transpReadRef,
            1,
            &mainColorRef,
            nullptr,
            &mainDepthRef, // no read/write here, but the depth map is mandatory
            0,
            nullptr
        },
        // Subpass for markers
        {
            0,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            0,
            nullptr,
            2,
            colorIndexRefs,
            nullptr,
            &mainDepthRef, // no read/write
            0,
            nullptr
        },
        // Subpass for ImGui
        {
            0,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            0,
            nullptr,
            1,
            &mainColorRef,
            nullptr,
            &mainDepthRef, // no read/write
            0,
            nullptr
        },
        // Subpass for gizmos (manipulators & coordinate system)
        {
            0,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            0,
            nullptr,
            2,
            colorIndexRefs,
            nullptr,
            &gizmoDepthRef,
            0,
            nullptr
        }
    };

    VkSubpassDependency dependency[] = {
        {
            0,
            1,
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, // can it be "early tests" cause there is no late tests in our shaders
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
            0
        },
        /*
        0, 2,
        COLOR_ATTACHMENT_WRITE, COLOR_ATTACHMENT_READ & WRITE
        */
        {
            1,
            2,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
            0
        },
        {
            2,
            3,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            0
        },
        {
            3,
            4,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            0
        },
        {
            4,
            5,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            0
        }
    };

    VkAttachmentDescription attachments[] = { mainColorDesc, mainDepthDesc, indexDesc, transpColorDesc, gizmoDepthDesc };
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = sizeof(attachments) / sizeof(VkAttachmentDescription);
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = sizeof(subpasses) / sizeof(VkSubpassDescription);
    renderPassInfo.pSubpasses = subpasses;
    renderPassInfo.dependencyCount = sizeof(dependency) / sizeof(VkSubpassDependency);
    renderPassInfo.pDependencies = dependency;

    VkResult err = m_pfnDev->vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass_obj);
    check_vk_result(err, "Create Object Render Pass");
    return err == VkResult::VK_SUCCESS;

    // Transparent Render Pass
    // Changes from the opaque render pass:
    //   - The color attachment is clear on load
    //   - 
}

bool VulkanManager::createCommandPools()
{
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    // Graphics
    // TODO - test the impact of the flag VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmdPoolInfo.queueFamilyIndex = m_graphicsQID.family;

    VkResult err = m_pfnDev->vkCreateCommandPool(m_device, &cmdPoolInfo, nullptr, &m_graphicsCmdPool);
    check_vk_result(err, "Create Graphics Command Pool");

    // Transfers
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmdPoolInfo.queueFamilyIndex = m_transferQID.family;

    err = m_pfnDev->vkCreateCommandPool(m_device, &cmdPoolInfo, nullptr, &m_transferCmdPool);
    check_vk_result(err, "Create Transfer Command Pool");

    //Compute
    //cmdPoolInfo.queueFamilyIndex = m_computeQID.family;
    cmdPoolInfo.queueFamilyIndex = m_graphicsQID.family;  // FIXME - specific test
    err = m_pfnDev->vkCreateCommandPool(m_device, &cmdPoolInfo, nullptr, &m_computeCmdPool);
    check_vk_result(err, "Create Compute Command Pool");

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_transferCmdPool;
    allocInfo.commandBufferCount = 1;

    m_pfnDev->vkAllocateCommandBuffers(m_device, &allocInfo, &m_transferCmdBuf);
    return err == VkResult::VK_SUCCESS;
}

bool VulkanManager::createDescriptorPool()
{
    // NOTE - Ne pas oublier dâ€™augmenter les pools lorsque lâ€™on voudra plus que 2 viewports
    VkDescriptorPoolSize poolSizes[] =
    {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6 }, // for ImGui, how much ?
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 24 },  // for PointCloud Engine
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 4},
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10 }, // 5 by viewport
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 8 }
    };

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 40; // sampler & uniform
    poolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);
    poolInfo.pPoolSizes = poolSizes;
    VkResult err = m_pfnDev->vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descPool);
    check_vk_result(err, "Create Descriptor Pool");
    return err == VkResult::VK_SUCCESS;
}

bool VulkanManager::createDescriptorSetLayouts()
{
    VkResult err;
    VkDescriptorSetLayoutCreateInfo descLayoutInfo;
    descLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descLayoutInfo.pNext = nullptr;
    descLayoutInfo.flags = 0;
    // ********** Layout for TlFramebuffer ***********

    VkDescriptorSetLayoutBinding layoutBindings1 = {
        3, // binding
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        1, // descriptor count
        VK_SHADER_STAGE_FRAGMENT_BIT,
        nullptr
    };

    descLayoutInfo.bindingCount = 1;
    descLayoutInfo.pBindings = &layoutBindings1;
    err = m_pfnDev->vkCreateDescriptorSetLayout(m_device, &descLayoutInfo, nullptr, &m_descSetLayout_inputDepth);
    check_vk_result(err, "Create Descriptor Set Layout");

    // ********** Layout for TlFramebufferV2 ***********

    VkDescriptorSetLayoutBinding layoutBindings2[] = {
        {
            0,
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            1, // descriptor count
            VK_SHADER_STAGE_COMPUTE_BIT,
            nullptr
        },
        {
            1,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            1, // descriptor count
            VK_SHADER_STAGE_COMPUTE_BIT,
            nullptr
        }
    };

    descLayoutInfo.bindingCount = sizeof(layoutBindings2) / sizeof(VkDescriptorSetLayoutBinding);
    descLayoutInfo.pBindings = layoutBindings2;
    err = m_pfnDev->vkCreateDescriptorSetLayout(m_device, &descLayoutInfo, nullptr, &m_descSetLayout_filling);
    check_vk_result(err, "Create Descriptor Set Layout");

    VkDescriptorSetLayoutBinding layoutBindings3[] = {
        {
            2,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            1, // descriptor count
            VK_SHADER_STAGE_COMPUTE_BIT,
            nullptr
        }
    };

    descLayoutInfo.bindingCount = sizeof(layoutBindings3) / sizeof(VkDescriptorSetLayoutBinding);
    descLayoutInfo.pBindings = layoutBindings3;
    err = m_pfnDev->vkCreateDescriptorSetLayout(m_device, &descLayoutInfo, nullptr, &m_descSetLayout_finalOutput);
    check_vk_result(err, "Create Descriptor Set Layout");

    // ********* Layout for Input attachment of transparent rendering ***********

    VkDescriptorSetLayoutBinding layoutBindingsTransp[] =
    {
        {
            0, // binding
            VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            1, // descriptor count
            VK_SHADER_STAGE_FRAGMENT_BIT,
            nullptr
        }
    };

    descLayoutInfo.bindingCount = sizeof(layoutBindingsTransp) / sizeof(VkDescriptorSetLayoutBinding);
    descLayoutInfo.pBindings = layoutBindingsTransp;
    err = m_pfnDev->vkCreateDescriptorSetLayout(m_device, &descLayoutInfo, nullptr, &m_descSetLayout_inputTransparentLayer);
    check_vk_result(err, "Create Descriptor Set Layout");

    // FIXME(robin) - Il y a plusieurs code erreur dans cette fonction
    return err == VkResult::VK_SUCCESS;
}

bool VulkanManager::createFences()
{
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkResult err = VK_SUCCESS;
    for (size_t i = 0; i < m_renderFences.size(); ++i)
    {
        err = m_pfnDev->vkCreateFence(m_device, &fenceInfo, nullptr, &m_renderFences[i]);
        check_vk_result(err, "Create Fence");
    }
    return err == VkResult::VK_SUCCESS;
}

bool VulkanManager::createStagingMemory()
{
    // The staging and source buffer
    // - host visible memory
    // - transfer source usage
    // - fixed size
    // - 1 instance with permanent allocation
    VkResult err = createBuffer(m_stagingBuf, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, m_stagingSize);
    check_vk_result(err, "Create Staging Buffer");

    VkMemoryRequirements stagingMemReq;
    m_pfnDev->vkGetBufferMemoryRequirements(m_device, m_stagingBuf, &stagingMemReq);

    err = allocateMemory(m_stagingMem, stagingMemReq, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "Staging memory");
    check_vk_result(err, "Allocate Staging Memory");

    err = m_pfnDev->vkBindBufferMemory(m_device, m_stagingBuf, m_stagingMem, 0);
    check_vk_result(err, "Bind Buffer and Staging Memory");

    err = m_pfnDev->vkMapMemory(m_device, m_stagingMem, 0, m_stagingSize, 0, &m_pStaging);
    check_vk_result(err, "Map staging memory");

    return err == VkResult::VK_SUCCESS;
}

bool VulkanManager::createUniformBuffer()
{
    const uint32_t uniformSize = 1024 * 1024;
    VkResult err = createBuffer(m_uniformBuf, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uniformSize);
    check_vk_result(err, "Buffer creation for Uniforms");

    VkMemoryRequirements uniMemReq;
    m_pfnDev->vkGetBufferMemoryRequirements(m_device, m_uniformBuf, &uniMemReq);

    err = allocateMemory(m_uniformMem, uniMemReq, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "Uniform buffer");
    check_vk_result(err, "Uniform Memory Allocation");

    //+++ Buffer Binding +++//
    err = m_pfnDev->vkBindBufferMemory(m_device, m_uniformBuf, m_uniformMem, 0);
    check_vk_result(err, "Bind Buffer Memory (Uniform buffer)");

    m_uniformAllocator.init(uniformSize, (uint32_t)m_physDevProperties.limits.minUniformBufferOffsetAlignment, 64);

    return err == VkResult::VK_SUCCESS;
}

VkSampler VulkanManager::createTextureSampler()
{
    VkSamplerCreateInfo samplerInfo = {
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        nullptr,                                 // pNext
        0,                                       // flags
        VK_FILTER_LINEAR,                        // magFilter
        VK_FILTER_LINEAR,                        // minFilter
        VK_SAMPLER_MIPMAP_MODE_NEAREST,          // mipmapMode
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, // addressModeU
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, // addressModeV
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, // addressModeW
        0.f,                                     // mipLodBias
        VK_FALSE,                                // anisotropyEnable
        1.f,                                     // maxAnisotropy
        VK_FALSE,                                // compareEnable
        VK_COMPARE_OP_ALWAYS,                    // compareOp
        0.f,                                     // minLod
        0.f,                                     // maxLod
        VK_BORDER_COLOR_INT_TRANSPARENT_BLACK,   // borderColor
        VK_FALSE                                 // unormalizedCoordinates
    };

    VkSampler sampler;

    VkResult err = m_pfnDev->vkCreateSampler(m_device, &samplerInfo, nullptr, &sampler);
    check_vk_result(err, "Sampler creation");

    return sampler;
}

// ----------------------------------------------------------------------------
//                      Framebuffer Resource Creation
// ----------------------------------------------------------------------------

void VulkanManager::createDrawBuffers(TlFramebuffer _fb)
{
    _fb->drawMarkerBuffers.resize(_fb->imageCount);
    _fb->drawMeasureBuffers.resize(_fb->imageCount);
}

bool VulkanManager::createSwapchain(TlFramebuffer _fb)
{
    _fb->extent = selectSwapchainExtent(_fb->surface, _fb->requestedExtent);
    if (_fb->extent.width == 0 || _fb->extent.height == 0)
    {
        VKM_WARNING << "Cannot create a swapchain with one dimension equal to zero." << Logger::endl;
        return false;
    }

    _fb->presentQFI = findPresentQueueFamily(_fb->surface);

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _fb->surface;
    createInfo.minImageCount = _fb->imageCount;
    createInfo.imageFormat = _fb->surfaceFormat.format;
    createInfo.imageColorSpace = _fb->surfaceFormat.colorSpace;
    createInfo.imageExtent = _fb->extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (m_graphicsQID.family != _fb->presentQFI) {
        uint32_t queueFamilyIndexes[] = { m_graphicsQID.family, _fb->presentQFI };
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndexes;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        // queueFamilyIndexCount is optional
        // pQueueFamilyIndices is optional
    }
    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = _fb->presentMode;
    createInfo.clipped = VK_TRUE;

    VkResult err = m_pfnDev->vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &(_fb->swapchain));
    check_vk_result(err, "Create Swapchain KHR");

    // Image count queried during the creation can be higher in the implementation
    m_pfnDev->vkGetSwapchainImagesKHR(m_device, _fb->swapchain, &(_fb->imageCount), nullptr);

    _fb->pImages = new VkImage[_fb->imageCount];
    m_pfnDev->vkGetSwapchainImagesKHR(m_device, _fb->swapchain, &(_fb->imageCount), _fb->pImages);

    // Image Views
    _fb->pImageViews = new VkImageView[_fb->imageCount];
    for (uint32_t i = 0; i < _fb->imageCount; i++) {
        _fb->pImageViews[i] = createImageView(_fb->pImages[i], _fb->surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    _fb->initLayout = _fb->imageCount;

    return true;
}

void VulkanManager::createVirtualRenderImages(TlFramebuffer _fb)
{
    assert(_fb->imageCount == 1);
    // Pas de swapchain extent

    // Pas de present queue
    _fb->pImages = new VkImage[_fb->imageCount];
    _fb->pImageViews = new VkImageView[_fb->imageCount];

    createImage(_fb->extent, 1, _fb->surfaceFormat.format, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _fb->pImages[0], _fb->virtualImageMemory);

    _fb->pImageViews[0] = createImageView(_fb->pImages[0], _fb->surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);

    _fb->initLayout = _fb->imageCount; // 1 normaly
}

void VulkanManager::createDepthBuffers(TlFramebuffer _fb)
{
    _fb->dsFormat = m_depthFormat;
    VKM_INFO << "Depth buffer format: " << _fb->dsFormat << Logger::endl;

    // Depth Render Target
    createImage(_fb->extent, 1, _fb->dsFormat, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _fb->objectDepthImage, _fb->objectDepthMemory);

    _fb->objectDepthImageView = createImageView(_fb->objectDepthImage, _fb->dsFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VulkanManager::createAttachments(TlFramebuffer _fb)
{
    // intermediary point cloud color render target
    createImage(_fb->extent, 1, _fb->pcFormat, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _fb->pcColorImage, _fb->pcColorMemory);
    _fb->initColorLayout = true;

    _fb->pcColorImageView = createImageView(_fb->pcColorImage, _fb->pcFormat, VK_IMAGE_ASPECT_COLOR_BIT);

    // object id attachment
    createImage(_fb->extent, 1, VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _fb->idAttImage, _fb->idAttMemory);

    _fb->idAttImageView = createImageView(_fb->idAttImage, VK_FORMAT_R32_UINT, VK_IMAGE_ASPECT_COLOR_BIT);

    // render target for transparent objects
    createImage(_fb->extent, 1, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _fb->transparentObjectImage, _fb->transparentObjectMemory);

    _fb->transparentObjectImageView = createImageView(_fb->transparentObjectImage, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);

    // Depth map for gizmos
    createImage(_fb->extent, 1, _fb->dsFormat, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _fb->gizmoDepthImage, _fb->gizmoDepthMemory);

    _fb->gizmoDepthImageView = createImageView(_fb->gizmoDepthImage, _fb->dsFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    // intermediary point cloud depth render target
    createImage(_fb->extent, 1, _fb->dsFormat, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _fb->pcDepthImage, _fb->pcDepthMemory);

    _fb->pcDepthImageView = createImageView(_fb->pcDepthImage, _fb->dsFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    // Depth Render Target
    uint32_t dataSizeDepth = _fb->extent.width * _fb->extent.height * 4;
    VkResult err = createBuffer(_fb->correctedDepthBuffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, dataSizeDepth);
    check_vk_result(err, "Buffer creation for corrected depth");

    VkMemoryRequirements memRequirements;
    m_pfnDev->vkGetBufferMemoryRequirements(m_device, _fb->correctedDepthBuffer, &memRequirements);
    allocateMemory(_fb->correctedDepthBufferMemory, memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Corrected depth");
    _fb->correctedDepthSize = dataSizeDepth;

    err = m_pfnDev->vkBindBufferMemory(m_device, _fb->correctedDepthBuffer, _fb->correctedDepthBufferMemory, 0);
    check_vk_result(err, "Bind Buffer Memory (Copy Depth)");

    // Sampler
    VkSamplerCreateInfo samplerInfo = {
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        nullptr,                                 // pNext
        0,                                       // flags
        VK_FILTER_NEAREST,                       // magFilter
        VK_FILTER_NEAREST,                       // minFilter
        VK_SAMPLER_MIPMAP_MODE_NEAREST,          // mipmapMode
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, // addressModeU
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, // addressModeV
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, // addressModeW
        0.f,                                     // mipLodBias
        VK_FALSE,                                // anisotropyEnable
        1.f,                                     // maxAnisotropy
        VK_FALSE,                                // compareEnable
        VK_COMPARE_OP_ALWAYS,                    // compareOp
        0.f,                                     // minLod
        0.f,                                     // maxLod
        VK_BORDER_COLOR_INT_OPAQUE_WHITE,        // borderColor
        VK_TRUE                                  // unormalizedCoordinates
    };

    err = m_pfnDev->vkCreateSampler(m_device, &samplerInfo, nullptr, &_fb->rawSampler);
    check_vk_result(err, "Sampler creation");
}

// *** Creation of the buffer that will store in host memory a copy of the depth ***
void VulkanManager::createCopyBuffers(TlFramebuffer _fb)
{
    // NOTE - we use either VK_FORMAT_D32_SFLOAT or VK_FORMAT_X8_D24_UNORM_PACK32
    //        that are both on 4 bytes
    uint32_t dataSizeDepth = _fb->extent.width * _fb->extent.height * 4;
    uint32_t dataSizeIndex = _fb->extent.width * _fb->extent.height * 4;

    // Buffer to copy the depth in.
    VkResult err = createBuffer(_fb->copyBufDepth, VK_BUFFER_USAGE_TRANSFER_DST_BIT, dataSizeDepth);
    check_vk_result(err, "Buffer creation for depth copy");
    // Buffer to copy the index image in
    err = createBuffer(_fb->copyBufIndex, VK_BUFFER_USAGE_TRANSFER_DST_BIT, dataSizeIndex);
    check_vk_result(err, "Buffer creation for index copy");

    // One memory block for the two buffers
    VkMemoryRequirements memReq1, memReq2;
    m_pfnDev->vkGetBufferMemoryRequirements(m_device, _fb->copyBufDepth, &memReq1);
    m_pfnDev->vkGetBufferMemoryRequirements(m_device, _fb->copyBufIndex, &memReq2);
    _fb->copyMemSize = aligned(memReq1.size, memReq2.alignment) + memReq2.size;

    VkMemoryRequirements memReqTotal = memReq1;
    memReqTotal.size = _fb->copyMemSize;

    err = allocateMemory(_fb->copyMemory, memReqTotal, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "CPU Copy Depth & Index");

    check_vk_result(err, "Host visible-coherent (for attch copy) Memory Allocation");

    //+++ Buffer Binding +++//
    // Uniform buffer
    err = m_pfnDev->vkBindBufferMemory(m_device, _fb->copyBufDepth, _fb->copyMemory, 0);
    check_vk_result(err, "Bind Buffer Memory (Copy Depth)");

    err = m_pfnDev->vkBindBufferMemory(m_device, _fb->copyBufIndex, _fb->copyMemory, aligned(memReq1.size, memReq2.alignment));
    check_vk_result(err, "Bind Buffer Memory (Copy Index)");

    err = m_pfnDev->vkMapMemory(m_device, _fb->copyMemory, 0, _fb->copyMemSize, 0, &_fb->pMappedCopyDepth);
    check_vk_result(err, "Map Memory (copy depth)");
    _fb->pMappedCopyIndex = (char*)_fb->pMappedCopyDepth + aligned(memReq1.size, memReq2.alignment);
}

void VulkanManager::allocateDescriptorSet(TlFramebuffer _fb)
{
    {
        VkDescriptorSetAllocateInfo descSetAllocInfo = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            nullptr,
            m_descPool,
            1,
            &m_descSetLayout_inputDepth
        };
        VkResult err = m_pfnDev->vkAllocateDescriptorSets(m_device, &descSetAllocInfo, &_fb->descSetInputDepth);
        check_vk_result(err, "Allocate Descriptor Sets");

        VkDescriptorBufferInfo depthInfo = {
            _fb->correctedDepthBuffer,
            0,
            _fb->correctedDepthSize
        };

        VkWriteDescriptorSet writeDesc = {};
        writeDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDesc.dstSet = _fb->descSetInputDepth;
        writeDesc.dstBinding = 3;
        writeDesc.descriptorCount = 1;
        writeDesc.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writeDesc.pBufferInfo = &depthInfo;

        m_pfnDev->vkUpdateDescriptorSets(m_device, 1, &writeDesc, 0, nullptr);
    }

    //----------------------------------//
    {
        VkDescriptorSetAllocateInfo descSetAllocInfo = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            nullptr,
            m_descPool,
            1,
            &m_descSetLayout_filling
        };
        VkResult err = m_pfnDev->vkAllocateDescriptorSets(m_device, &descSetAllocInfo, &_fb->descSetSamplers);
        check_vk_result(err, "Allocate Descriptor Sets");

        VkDescriptorImageInfo imageInfos[] = {
            {
                VK_NULL_HANDLE,
                _fb->pcColorImageView,
                VK_IMAGE_LAYOUT_GENERAL
            },
            {
                _fb->rawSampler,
                _fb->pcDepthImageView,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
            }
        };

        constexpr uint32_t count = sizeof(imageInfos) / sizeof(VkDescriptorImageInfo);
        VkWriteDescriptorSet writeDesc[count];
        memset(writeDesc, 0, sizeof(writeDesc));

        writeDesc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDesc[0].dstSet = _fb->descSetSamplers;
        writeDesc[0].dstBinding = 0;
        writeDesc[0].descriptorCount = 1;
        writeDesc[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        writeDesc[0].pImageInfo = &imageInfos[0];

        writeDesc[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDesc[1].dstSet = _fb->descSetSamplers;
        writeDesc[1].dstBinding = 1;
        writeDesc[1].descriptorCount = 1;
        writeDesc[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDesc[1].pImageInfo = &imageInfos[1];

        m_pfnDev->vkUpdateDescriptorSets(m_device, count, writeDesc, 0, nullptr);
    }
    //----------------------------------//
    {
        VkDescriptorSetAllocateInfo allocInfo = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            nullptr,
            m_descPool,
            1,
            &m_descSetLayout_finalOutput
        };
        VkResult err = m_pfnDev->vkAllocateDescriptorSets(m_device, &allocInfo, &_fb->descSetCorrectedDepth);
        check_vk_result(err, "Allocate Descriptor Sets");

        VkDescriptorBufferInfo bufferInfo = {
            _fb->correctedDepthBuffer,
            0,
            _fb->correctedDepthSize
        };

        VkWriteDescriptorSet writeDescFinal = {};
        writeDescFinal.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescFinal.dstSet = _fb->descSetCorrectedDepth;
        writeDescFinal.dstBinding = 2;
        writeDescFinal.descriptorCount = 1;
        writeDescFinal.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writeDescFinal.pBufferInfo = &bufferInfo;

        m_pfnDev->vkUpdateDescriptorSets(m_device, 1, &writeDescFinal, 0, nullptr);
    }
    //----------------------------------//
    {
        VkDescriptorSetAllocateInfo allocInfo = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            nullptr,
            m_descPool,
            1,
            & m_descSetLayout_inputTransparentLayer
        };
        VkResult err = m_pfnDev->vkAllocateDescriptorSets(m_device, &allocInfo, &_fb->descSetInputTransparentLayer);
        check_vk_result(err, "AllocateDescriptor Sets");

        VkDescriptorImageInfo imageInfo = {
            VK_NULL_HANDLE,
            _fb->transparentObjectImageView,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        VkWriteDescriptorSet writeDesc = {};
        writeDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDesc.dstSet = _fb->descSetInputTransparentLayer;
        writeDesc.dstBinding = 0;
        writeDesc.descriptorCount = 1;
        writeDesc.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        writeDesc.pImageInfo = &imageInfo;

        m_pfnDev->vkUpdateDescriptorSets(m_device, 1, &writeDesc, 0, nullptr);
    }
}

void VulkanManager::createPcFramebuffer(TlFramebuffer _fb)
{
    assert(m_renderPass_pc.find(_fb->pcFormat) != m_renderPass_pc.end());

    VkImageView attachments[] = { _fb->pcColorImageView, _fb->pcDepthImageView };

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_renderPass_pc.at(_fb->pcFormat);
    framebufferInfo.attachmentCount = sizeof(attachments) / sizeof(VkImageView);
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = _fb->extent.width;
    framebufferInfo.height = _fb->extent.height;
    framebufferInfo.layers = 1;

    VkResult err = m_pfnDev->vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &_fb->pcFramebuffer);
    check_vk_result(err, "Create Framebuffer");
}

void VulkanManager::createFinalFramebuffers(TlFramebuffer _fb)
{
    VkResult err;
    _fb->finalFramebuffers = new VkFramebuffer[_fb->imageCount];

    for (size_t i = 0; i < _fb->imageCount; i++)
    {
        VkImageView attachments_obj[] = { _fb->pImageViews[i], _fb->objectDepthImageView, _fb->idAttImageView, _fb->transparentObjectImageView, _fb->gizmoDepthImageView };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass_obj;
        framebufferInfo.attachmentCount = sizeof(attachments_obj) / sizeof(VkImageView);
        framebufferInfo.pAttachments = attachments_obj;
        framebufferInfo.width = _fb->extent.width;
        framebufferInfo.height = _fb->extent.height;
        framebufferInfo.layers = 1;

        err = m_pfnDev->vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &_fb->finalFramebuffers[i]);
        check_vk_result(err, "Create Framebuffer");
    }
}

void VulkanManager::createCommandBuffers(TlFramebuffer _fb)
{
    if (m_graphicsCmdPool == VK_NULL_HANDLE)
        return;

    _fb->graphicsCmdBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool = m_graphicsCmdPool;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    VkResult err = m_pfnDev->vkAllocateCommandBuffers(m_device, &info, _fb->graphicsCmdBuffers.data());
    check_vk_result(err, "Allocate Graphics Command Buffers");
}

void VulkanManager::createSemaphores(TlFramebuffer _fb)
{
    _fb->imageAvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
    VkResult err;
    // Image Semaphore
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        err = m_pfnDev->vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &_fb->imageAvailableSemaphore[i]);
        check_vk_result(err, "Create Semaphore");
    }

    err = m_pfnDev->vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &_fb->renderFinishedSemaphore);
    check_vk_result(err, "Create Semaphore");
}

//-----------------------------------------------------------------------------
//             Swapchain configuration functions
//-----------------------------------------------------------------------------

VkSurfaceFormatKHR VulkanManager::selectSwapchainSurfaceFormat(VkSurfaceKHR _surface, VkFormat _preferedFormat)
{
    uint32_t formatCount;
    m_pfn.vkGetPhysicalDeviceSurfaceFormatsKHR(m_physDev, _surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats;
    formats.resize(formatCount);
    m_pfn.vkGetPhysicalDeviceSurfaceFormatsKHR(m_physDev, _surface, &formatCount, formats.data());

    VkSurfaceFormatKHR surfaceFormat = formats[0];
    // If the surface has no preferred format, we choose it freely
    if (formatCount == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
        surfaceFormat = { _preferedFormat, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    }
    else
    {
        // Else we search our prefered format in the proposed ones
        for (uint32_t i = 0; i < formatCount; i++) {
            if (formats[i].format == _preferedFormat && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                surfaceFormat = formats[i];
                break;
            }
        }
    }

    return (surfaceFormat);
}

/*
We can choose between the modes :
VK_PRESENT_MODE_IMMEDIATE_KHR
VK_PRESENT_MODE_MAILBOX_KHR
VK_PRESENT_MODE_FIFO_KHR            -> always available
VK_PRESENT_MODE_FIFO_RELAXED_KHR
...
*/
VkPresentModeKHR VulkanManager::selectSwapchainPresentMode(VkSurfaceKHR _surface)
{
    /*
    uint32_t presentCount;
    m_pfn.vkGetPhysicalDeviceSurfacePresentModesKHR(m_physDev, _surface, &presentCount, nullptr);
    VkPresentModeKHR* pPresentModes = new VkPresentModeKHR[presentCount];
    m_pfn.vkGetPhysicalDeviceSurfacePresentModesKHR(m_physDev, _surface, &presentCount, pPresentModes);

    delete[] pPresentModes;
    */

    // Select preferably the mode FIFO to synchronize the framerate with the presentation rate.
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanManager::selectSwapchainExtent(VkSurfaceKHR _surface, VkExtent2D _requestedExtent)
{
    VkSurfaceCapabilitiesKHR capabilities;
    m_pfn.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physDev, _surface, &capabilities);

    VkExtent2D chosenExtent = _requestedExtent;

    // If the extent is not given by Vulkan we can set it ourself
    if (capabilities.currentExtent.width == 0xFFFFFFFF) {
        // Clamp within min and max image extent
        chosenExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, chosenExtent.width));
        chosenExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, chosenExtent.height));

        VKM_INFO << "Extent not provided, auto select {" << chosenExtent.width << " x " << chosenExtent.height << "}" << Logger::endl;
    }
    else {
        chosenExtent = capabilities.currentExtent;
        VKM_INFO << "Swapchain Extent provided by Vulkan {" << chosenExtent.width << " x " << chosenExtent.height << "}" << Logger::endl;
    }

    return chosenExtent;
}

uint32_t VulkanManager::findPresentQueueFamily(VkSurfaceKHR _surface)
{
    uint32_t presentQFI = UINT32_MAX;

    // Test each family for presentation
    VkBool32* supportsPresent = new VkBool32[m_queueFamilyCount];
    for (uint32_t i = 0; i < m_queueFamilyCount; i++) {
        m_pfn.vkGetPhysicalDeviceSurfaceSupportKHR(m_physDev, i, _surface, &supportsPresent[i]);
    }

    // Look for a graphic queue that support present.
    for (uint32_t i = 0; i < m_queueFamilyCount; i++) {
        if (supportsPresent[i] == VK_TRUE)
        {
            presentQFI = i;
            if (m_queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                // sharing mode exclusive
                break;
            }

            // sharing mode concurrent
        }
    }

    if (presentQFI == UINT32_MAX) {
        VKM_ERROR << "Cannot find a presentation queue for the Surface." << Logger::endl;
        exit(1);
    }

    delete[] supportsPresent;
    return presentQFI;
}


uint32_t VulkanManager::selectImageCount(VkSurfaceKHR _surface)
{
    VkSurfaceCapabilitiesKHR capabilities;
    m_pfn.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physDev, _surface, &capabilities);

    // NOTE(robin) AMD device can return minImageCount = 1
    uint32_t imageCount = std::max(2u, capabilities.minImageCount);
    // NOTE(robin) maxImageCount can be 0, which means that there is no limit
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    return imageCount;
}

void VulkanManager::copyBufferToImage(VkCommandBuffer _cmdBuf, VkBuffer _buffer, VkImage _image, VkImageAspectFlags _aspectFlags, uint32_t _layer, VkExtent3D _extent)
{
    VkBufferImageCopy copyRegion = {
        0,            // bufferOffset
        0,            // bufferRowLength
        0,            // bufferImageHeight
        { _aspectFlags, 0, _layer, 1 },  // imageSubresource
        { 0, 0, 0 },  // imageOffset
        _extent,      // imageExtent
    };

    m_pfnDev->vkCmdCopyBufferToImage(_cmdBuf, _buffer, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
}

void VulkanManager::copyImageToBuffer(VkCommandBuffer _cmdBuf, VkImage _image, VkImageAspectFlags _aspectFlags, uint32_t _layer, VkExtent3D _extent, VkBuffer _buffer)
{
    VkBufferImageCopy copyRegion = {
        0,            // bufferOffset
        0,            // bufferRowLength
        0,            // bufferImageHeight
        { _aspectFlags, 0, _layer, 1 },  // imageSubresource
        { 0, 0, 0 },  // imageOffset
        _extent,      // imageExtent
    };

    m_pfnDev->vkCmdCopyImageToBuffer(_cmdBuf, _image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, _buffer, 1, &copyRegion);
}

void VulkanManager::pipelineBarrier(VkCommandBuffer _cmdBuf, VkImage _image, VkImageLayout _oldLayout, VkImageLayout _newLayout, VkImageAspectFlags _aspectFlags)
{
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.oldLayout = _oldLayout;
    barrier.newLayout = _newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = _image;
    barrier.subresourceRange = { _aspectFlags, 0, 1, 0, 1 };

    m_pfnDev->vkCmdPipelineBarrier(
        _cmdBuf,
        VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);
}

VkCommandBuffer VulkanManager::beginTransferCommand()
{
    // We cannot make a transfer on another thread than the transfer thread
    assert(m_transferThread.get_id() == std::this_thread::get_id());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_transferCmdPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    m_pfnDev->vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    m_pfnDev->vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VulkanManager::endTransferCommand(VkCommandBuffer _cmdBuffer)
{
    // We cannot make a transfer on another thread than the transfer thread
    assert(m_transferThread.get_id() == std::this_thread::get_id());

    m_pfnDev->vkEndCommandBuffer(_cmdBuffer);
    submitTransferAndWait(_cmdBuffer);

    m_pfnDev->vkFreeCommandBuffers(m_device, m_transferCmdPool, 1, &_cmdBuffer);
}

bool VulkanManager::checkSwapchain(TlFramebuffer _fb)
{
    if (_fb->mustRecreateSwapchain.load() == true)
    {
        cleanupSizeDependantResources(_fb);

        if (!createSwapchain(_fb))
            return false;
        createDepthBuffers(_fb);
        createAttachments(_fb);
        createCopyBuffers(_fb);
        allocateDescriptorSet(_fb);
        createPcFramebuffer(_fb);
        createFinalFramebuffers(_fb);
        // NOTE - Recreate the command buffers to make sure the old commands are reset
        createCommandBuffers(_fb);
        createSemaphores(_fb);
        _fb->mustRecreateSwapchain.store(false);
    }

    return (_fb->extent.width != 0 && _fb->extent.height != 0);
}

//-----------------------------------------------------------------------------
//                       Helper Funtions
//-----------------------------------------------------------------------------

void VulkanManager::checkValidationLayerSupport()
{
    // Enumarate the layers
    uint32_t availableLayerCount;
    VkResult err = m_pfn.vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);
    check_vk_result(err, "Enumarate Instance Layer Properties 1");

    VkLayerProperties* pAvailableLayers = new VkLayerProperties[availableLayerCount];
    err = m_pfn.vkEnumerateInstanceLayerProperties(&availableLayerCount, pAvailableLayers);
    check_vk_result(err, "Enumarate Instance Layer Properties 2");

    SubLogger& log = Logger::log(LoggerMode::VKLog);
    log << "INFO --- Validation Layers supported:\n";
    for (uint32_t i = 0; i < availableLayerCount; i++) {
        log << "\"" << pAvailableLayers[i].layerName << "\" version = " << vkVersionToStr(pAvailableLayers[i].specVersion) << "\n";
    }

    log << Logger::endl;

    bool layersMissing = false;

    for (uint32_t i = 0; i < m_layerCount; i++) {
        bool layerFound = false;

        for (uint32_t j = 0; j < availableLayerCount; j++) {
            if (strcmp(m_layers[i], pAvailableLayers[j].layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        // Enumerate all the layers not found and send the VK_ERROR later
        if (!layerFound) {
            VKM_ERROR << "Validation Layer \"" << m_layers[i] << "\" requested but no found!" << Logger::endl;
            layersMissing = true;
        }
    }

    if (layersMissing)
    {
        check_vk_result(VK_ERROR_LAYER_NOT_PRESENT, "Validation Layer Support");
    }

    // clean
    delete[] pAvailableLayers;
}

void VulkanManager::checkInstanceExtensionSupport()
{
    // Enumerate the layers
    uint32_t availableExtensionCount;
    VkResult err = m_pfn.vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
    check_vk_result(err, "Enumarate Instance Extension Properties 1");

    VkExtensionProperties* pAvailableExtensions = new VkExtensionProperties[availableExtensionCount];
    err = m_pfn.vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, pAvailableExtensions);
    check_vk_result(err, "Enumarate Instance Extension Properties 2");

    SubLogger& log = Logger::log(LoggerMode::VKLog);
    log << "INFO --- Instance Extensions supported:\n";
    for (uint32_t i = 0; i < availableExtensionCount; i++) {
        log << "\"" << pAvailableExtensions[i].extensionName << "\" version = " << pAvailableExtensions[i].specVersion << "\n";
    }
    log << Logger::endl;

    bool extensionsMissing = false;

    for (uint32_t i = 0; i < m_extCount; i++) {
        bool extensionFound = false;

        for (uint32_t j = 0; j < availableExtensionCount; j++) {
            if (strcmp(m_extensions[i], pAvailableExtensions[j].extensionName) == 0) {
                extensionFound = true;
                break;
            }
        }

        // Enumerate all the layers not found and send the VK_ERROR later
        if (!extensionFound) {
            VKM_ERROR << "Instance Extension \"" << m_layers[i] << "\" requested but no found!" << Logger::endl;
            extensionsMissing = true;
        }
    }

    if (extensionsMissing)
    {
        check_vk_result(VK_ERROR_EXTENSION_NOT_PRESENT, "Instance Extension Support");
    }

    // clean
    delete[] pAvailableExtensions;

    return;
}

bool VulkanManager::checkDeviceExtensionSupport(VkPhysicalDevice _device, bool _printInfo)
{
    uint32_t devExtCount = 0;
    VkResult err = m_pfn.vkEnumerateDeviceExtensionProperties(_device, nullptr, &devExtCount, nullptr);
    check_vk_result(err, "Enumarate Device Extension Properties");

    VkExtensionProperties* pExtProperties = new VkExtensionProperties[devExtCount];
    err = m_pfn.vkEnumerateDeviceExtensionProperties(_device, nullptr, &devExtCount, pExtProperties);
    check_vk_result(err, "Enumarate Device Extension Properties");

    // Print informations about the Device Extensions {requested, supported}
    if (_printInfo) {
        SubLogger& log = Logger::log(VK_LOG);
        log << "INFO --- Device Extensions requested:\n";
        for (uint32_t i = 0; i < m_devExtCount; i++) {
            log << "\"" << m_devExtensions[i] << "\"\n";
        }
        log << Logger::endl;
    }

    for (uint32_t i = 0; i < m_devExtCount; i++) {
        bool extensionFound = false;
        for (uint32_t j = 0; j < devExtCount; j++) {
            if (strcmp(m_devExtensions[i], pExtProperties[j].extensionName) == 0)
                extensionFound = true;
        }
        if (!extensionFound) {
            VKM_WARNING << "Device Extension not available : " << m_devExtensions[i] << Logger::endl;
            return false;
        }
    }

    //clean
    delete[] pExtProperties;

    return true;
}

bool VulkanManager::checkAvailableFeatures(VkPhysicalDevice _device, bool select)
{
    VkPhysicalDeviceFeatures supportedFeatures;
    m_pfn.vkGetPhysicalDeviceFeatures(_device, &supportedFeatures);

    VKM_INFO << "Checking " << _device << " device features..." << Logger::endl;

    VkPhysicalDeviceFeatures minimumFeatures;
    memset(&minimumFeatures, 0, sizeof(VkPhysicalDeviceFeatures));
    minimumFeatures.fillModeNonSolid = VK_TRUE;
    minimumFeatures.largePoints = VK_TRUE;
    minimumFeatures.wideLines = VK_TRUE;
    minimumFeatures.geometryShader = VK_TRUE;
    minimumFeatures.shaderTessellationAndGeometryPointSize = VK_TRUE;
    // Feature required for not blending the "marker index" color attachment
    minimumFeatures.independentBlend = VK_TRUE;
    minimumFeatures.shaderStorageImageMultisample = VK_TRUE;

    for (int f = 0; f < sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32); ++f)
    {
        // Compare the values of minimum and supported
        VkBool32 minFt = (reinterpret_cast<VkBool32*>(&minimumFeatures))[f];
        VkBool32 supFt = (reinterpret_cast<VkBool32*>(&supportedFeatures))[f];
        if (minFt && !supFt)
        {
            VKM_WARNING << "A mandatory feature (vk_" << f << ") is missing on this device." << Logger::endl;
            return false;
        }

        // TODO: create a positional log
    }

    if (select)
    {
        m_features = minimumFeatures;
    }

    // TODO - Optimal features if necessary
    return true;
}

// Choose the family queue for each usage: graphics, compute, transfer and presentation
// -*- The graphics or compute queue can be used for the transfer
// -*- The surface must be tested for presentation
//
// Ideally want to find queue families that satisfy this criterias
// - The same queue for graphics and present
// - A queue family dedicated for transfer
// (no special requirement for the compute queue family)
bool VulkanManager::checkQueueFamilies(VkPhysicalDevice device, bool choose, bool printInfo)
{
    // Enumerate all queue families and their properties
    uint32_t queueFamilyCount = 0;
    m_pfn.vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    VkQueueFamilyProperties* queueFamilyProps = new VkQueueFamilyProperties[queueFamilyCount];
    m_pfn.vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProps);
    std::vector<uint32_t> queueUsed(queueFamilyCount);

    // Init the Queue Family Indices as not found
    // NOTES * From the "vkspec 1.1": a Graphics (and compute) queue always support transfer
    //       * We choose the queue in this order : Graphics, compute, transfer#1, transfer#2
    // FIXME * Some GPU (integrated) have only 1 queue available.
    //      ** We must be able to fit all queue usage on the same queue.
    //     *** We also must be able to use different queue when available.
    QueueID graphicsQID = findQueueAvailable(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, queueUsed, queueFamilyProps);
    QueueID computeQID = findQueueAvailable(VK_QUEUE_COMPUTE_BIT, queueUsed, queueFamilyProps);
    QueueID transferQID = findQueueAvailable(VK_QUEUE_TRANSFER_BIT, queueUsed, queueFamilyProps);
    QueueID streamingQID = findQueueAvailable(VK_QUEUE_TRANSFER_BIT, queueUsed, queueFamilyProps);

    bool validQueues = (graphicsQID.family != UINT32_MAX) &&
        (transferQID.family != UINT32_MAX) &&
        (computeQID.family != UINT32_MAX) &&
        (streamingQID.family != UINT32_MAX);
    bool singleQueueFallback = false;

    if (!validQueues && graphicsQID.family != UINT32_MAX)
    {
        computeQID = graphicsQID;
        transferQID = graphicsQID;
        streamingQID = graphicsQID;
        validQueues = true;
        singleQueueFallback = true;
        VKM_INFO << "Only one queue available. Using a shared queue for graphics/compute/transfer/streaming." << Logger::endl;
    }

    // Show the queue family available on this device and their properties
    if (printInfo)
    {
        SubLogger& log = Logger::log(VK_LOG);
        log << "INFO --- Queue Families properties ---\n";
        log << "   | Index | Flags: G C T S P | Count |\n";
        for (uint32_t i = 0; i < queueFamilyCount; i++)
        {
            // index
            log << " * | " << i << "     |        ";
            // flags
            LOG_FLAG(log, " ", queueFamilyProps[i].queueFlags, VK_QUEUE_GRAPHICS_BIT)
            LOG_FLAG(log, " ", queueFamilyProps[i].queueFlags, VK_QUEUE_COMPUTE_BIT)
            LOG_FLAG(log, " ", queueFamilyProps[i].queueFlags, VK_QUEUE_TRANSFER_BIT)
            LOG_FLAG(log, " ", queueFamilyProps[i].queueFlags, VK_QUEUE_SPARSE_BINDING_BIT)
            LOG_FLAG(log, " ", queueFamilyProps[i].queueFlags, VK_QUEUE_PROTECTED_BIT)
            // count
            log << "| " << queueFamilyProps[i].queueCount << "     |\n";
        }
        log << "\n" << Logger::endl;
    }

    if (choose && validQueues)
    {
        m_queueFamilyCount = queueFamilyCount;
        m_queueFamilyProps = queueFamilyProps;
        m_graphicsQID = graphicsQID;
        m_computeQID = computeQID;
        m_transferQID = transferQID;
        m_streamingQID = streamingQID;
        m_singleQueueFallback = singleQueueFallback;
    }
    else
    {
        delete[] queueFamilyProps;
    }

    return validQueues;
}

bool VulkanManager::checkMemoryType(VkPhysicalDevice device, bool select, bool printInfo)
{
    VkPhysicalDeviceMemoryProperties memProp;
    m_pfn.vkGetPhysicalDeviceMemoryProperties(device, &memProp);

    uint32_t memTypeIndex_local = findMemoryType(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    uint32_t memTypeIndex_host = findMemoryType(device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    bool validMem = memTypeIndex_local != UINT32_MAX && memTypeIndex_host != UINT32_MAX;

    if (select && !validMem)
        VKM_ERROR << "No valid memory type found for the memory pools" << Logger::endl;

    if (select && validMem)
    {
        m_memTypeIndex_local = memTypeIndex_local;
        m_memTypeIndex_host = memTypeIndex_host;
        m_deviceHeapIndex = memProp.memoryTypes[memTypeIndex_local].heapIndex;
        m_hostHeapIndex = memProp.memoryTypes[memTypeIndex_host].heapIndex;
    }

    // Print memory heaps & types
    if (printInfo)
    {
        SubLogger& log = Logger::log(VK_LOG);
        log << memProp.memoryTypeCount << " memory types:\n";
        log << "|       | Properties                      |      |\n";
        log << "| Index | DL  HV  HCo HCa LzA Pr  DCo DU  | Heap |\n";
        log << "|-------|---------------------------------|------|\n";
        for (size_t i = 0; i < memProp.memoryTypeCount; i++)
        {
            VkMemoryType memType = memProp.memoryTypes[i];
            log << "| " << i << "     | ";
            LOG_FLAG(log, "   ", memType.propertyFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            LOG_FLAG(log, "   ", memType.propertyFlags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            LOG_FLAG(log, "   ", memType.propertyFlags, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
            LOG_FLAG(log, "   ", memType.propertyFlags, VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
            LOG_FLAG(log, "   ", memType.propertyFlags, VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
            LOG_FLAG(log, "   ", memType.propertyFlags, VK_MEMORY_PROPERTY_PROTECTED_BIT)
            LOG_FLAG(log, "   ", memType.propertyFlags, VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD)
            LOG_FLAG(log, "   ", memType.propertyFlags, VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD)
            log << "| " << memType.heapIndex << "    |\n";
        }
        log << Logger::endl;

        log << "INFO --- Memory Heaps ---\n";
        log << "| Index | Size |\n";
        for (uint32_t i = 0; i < memProp.memoryHeapCount; i++)
        {
            VkMemoryHeap memHeap = memProp.memoryHeaps[i];
            // Index
            log << "| " << i << "     | ";
            // Size
            log << memHeap.size << "     |\n";
        }
        log << "\n" << Logger::endl;
    }

    return validMem;
}

bool VulkanManager::isDeviceSuitable(VkPhysicalDevice _device)
{
    // check the queue families
    bool queuesAvailable = checkQueueFamilies(_device, false, true);

    // check the memory
    bool memoryAvailable = checkMemoryType(_device, false, true);

    // check the device extensions required during the initialization
    bool extensionsSupported = checkDeviceExtensionSupport(_device, true);

    // check the physical device features
    bool featuresAvailable = checkAvailableFeatures(_device, false);

    return (queuesAvailable &&
            memoryAvailable &&
            extensionsSupported &&
            featuresAvailable);
}

VulkanManager::QueueID VulkanManager::findQueueAvailable(VkQueueFlags queueFlag, std::vector<uint32_t>& queueUsed, VkQueueFamilyProperties* queueFamilyProperties)
{
    QueueID queue = { UINT32_MAX, UINT32_MAX };
    for (uint32_t i = 0; i < queueUsed.size(); i++)
    {
        if (queueUsed[i] < queueFamilyProperties[i].queueCount
            && (queueFamilyProperties[i].queueFlags & queueFlag) == queueFlag)
        {
            queue.family = i;
            queue.index = queueUsed[i];
            queueUsed[i]++;
            break;
        }
    }
    return queue;
}

inline VkQueue VulkanManager::getQueue(QueueID queueID) const
{
    return m_ppQueue[queueID.family][queueID.index];
}

// TODO - Return a list of compatible memType.
//      - The heap size is not a sufficient critera, we may want to test other heap if the biggest one is full.
//      - Take into account the mem requirement for additional buffer/image features.
uint32_t VulkanManager::findMemoryType(VkPhysicalDevice device, VkMemoryPropertyFlags properties) const
{
    VkPhysicalDeviceMemoryProperties memProps;
    m_pfn.vkGetPhysicalDeviceMemoryProperties(device, &memProps);

    VkDeviceSize heapSize = 0;
    uint32_t typeIndex = UINT32_MAX;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if ((memProps.memoryTypes[i].propertyFlags & properties) == properties)
        {
            if (memProps.memoryHeaps[memProps.memoryTypes[i].heapIndex].size > heapSize)
            {
                typeIndex = i;
                heapSize = memProps.memoryHeaps[memProps.memoryTypes[i].heapIndex].size;
            }
        }
    }

    return typeIndex;
}

//-----------------------------------------------------------------------------
//             Vulkan Object Creation Shorthand Functions
//-----------------------------------------------------------------------------

VkResult VulkanManager::createBuffer(VkBuffer& _buf, VkBufferUsageFlags _usage, VkDeviceSize _size)
{
    VkBufferCreateInfo bufInfo = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,                    // pNext
        0,                          // flags
        _size,                      // size
        _usage,                     // usage
        VK_SHARING_MODE_EXCLUSIVE,  // sharingMode
        0,                          // queueFamilyIndexCount
        nullptr,                    // pQueueFamilyIndices
    };
    // NOTE(Robin) use VK_SHARING_MODE_CONCURRENT if we want to the same buffer in multiple queues

    VkResult err = m_pfnDev->vkCreateBuffer(m_device, &bufInfo, nullptr, &_buf);

    return err;
}

void VulkanManager::createImage(VkExtent2D _extent, uint32_t _layerCount, VkFormat _format, VkImageTiling _tiling, VkSampleCountFlagBits _sampleCount, VkImageUsageFlags _usage, VkMemoryPropertyFlags _properties, VkImage& _image, VkDeviceMemory& _imageMemory) const
{
    VkImageCreateInfo imageInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        nullptr,                // pNext
        0,                      // flags
        VK_IMAGE_TYPE_2D,       // imageType
        _format,                // format
        {_extent.width, _extent.height, 1}, // extent
        1,                      // mipLevels
        _layerCount,            // arrayLayers
        _sampleCount,           // samples
        _tiling,                // tiling
        _usage,                 // usage
        VK_SHARING_MODE_EXCLUSIVE,  // sharingMode
        0,                      // queueFamilyIndexCount
        nullptr,                // pQueueFamilyIndices
        VK_IMAGE_LAYOUT_UNDEFINED // initialLayout
    };

    VkResult err = m_pfnDev->vkCreateImage(m_device, &imageInfo, nullptr, &_image);
    check_vk_result(err, "Create Image");

    VkMemoryRequirements memRequirements;
    m_pfnDev->vkGetImageMemoryRequirements(m_device, _image, &memRequirements);

    allocateMemory(_imageMemory, memRequirements, _properties, "Image");

    m_pfnDev->vkBindImageMemory(m_device, _image, _imageMemory, 0);
}

VkImageView VulkanManager::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const
{
    VkImageViewCreateInfo viewInfo = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,                      // pNext
        0,                            // flags
        image,                        // image
        VK_IMAGE_VIEW_TYPE_2D,        // viewType
        format,                       // format
        {   // components
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY
        },
        { aspectFlags, 0, 1, 0, 1}     // SubresourceRange
    };

    VkImageView imageView;
    VkResult err = m_pfnDev->vkCreateImageView(m_device, &viewInfo, nullptr, &imageView);
    check_vk_result(err, "Create Image View");

    return imageView;
}

void VulkanManager::logFormatFeatures(VkFormat format, VkFormatFeatureFlags flags)
{
    // Test the point format available
    VkFormatProperties formatProp = {};
    m_pfn.vkGetPhysicalDeviceFormatProperties(m_physDev, format, &formatProp);

    std::set<VkFormatFeatureFlagBits> flagBits;
    for (int i = 0; i < 32; ++i)
    {
        uint32_t bit = 1 << i;
        if ((flags & bit) == bit)
            flagBits.insert(static_cast<VkFormatFeatureFlagBits>(bit));
    }

    SubLogger& slog = Logger::log(LoggerMode::VKLog);
    slog << "Info - Format properties for " << magic_enum::enum_name<VkFormat>(format) << "\n";
    slog << "      Feature        | Linear tiling | Optimal tiling | Buffer \n";
    for (auto it : flagBits)
    {
        slog << magic_enum::enum_name<VkFormatFeatureFlagBits>(it);
        slog << " | " << ((formatProp.linearTilingFeatures & it) == it);
        slog << "              | " << ((formatProp.optimalTilingFeatures & it) == it);
        slog << "               | " << ((formatProp.bufferFeatures & it) == it) << "\n";
    }
    slog << Logger::endl;
}

bool VulkanManager::checkBlitSupport(VkFormat srcFormat, VkFormat dstFormat, VkImageTiling tilling) const
{
    bool support = true;
    VkFormatProperties srcProp = {};
    VkFormatProperties dstProp = {};
    m_pfn.vkGetPhysicalDeviceFormatProperties(m_physDev, srcFormat, &srcProp);
    m_pfn.vkGetPhysicalDeviceFormatProperties(m_physDev, dstFormat, &dstProp);

    if (tilling == VK_IMAGE_TILING_LINEAR)
    {
        support &= (srcProp.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT) > 0;
        support &= (dstProp.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT) > 0;
    }
    else
    {
        support &= (srcProp.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT) > 0;
        support &= (dstProp.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT) > 0;
    }
    return support;
}

//-----------------------------------------------------------------------------
//              Cleanup Functions
//-----------------------------------------------------------------------------

void VulkanManager::cleanupAll()
{
    using namespace tls::vk;
    if (m_device && m_pfnDev)
    {
        destroyCommandPool(*m_pfnDev, m_device, m_graphicsCmdPool);
        destroyCommandPool(*m_pfnDev, m_device, m_computeCmdPool);
        freeCommandBuffer(*m_pfnDev, m_device, m_transferCmdPool, m_transferCmdBuf);
        destroyCommandPool(*m_pfnDev, m_device, m_transferCmdPool);

        destroyDescriptorPool(*m_pfnDev, m_device, m_descPool);

        destroyDescriptorSetLayout(*m_pfnDev, m_device, m_descSetLayout_inputDepth);
        destroyDescriptorSetLayout(*m_pfnDev, m_device, m_descSetLayout_filling);
        destroyDescriptorSetLayout(*m_pfnDev, m_device, m_descSetLayout_finalOutput);
        destroyDescriptorSetLayout(*m_pfnDev, m_device, m_descSetLayout_inputTransparentLayer);

        for (VkFence fence : m_renderFences)
            destroyFence(*m_pfnDev, m_device, fence);

        // Memory and Buffers
        destroyBuffer(*m_pfnDev, m_device, m_stagingBuf);
        freeMemory(*m_pfnDev, m_device, m_stagingMem);

        destroyBuffer(*m_pfnDev, m_device, m_uniformBuf);
        freeMemory(*m_pfnDev, m_device, m_uniformMem);

        for (auto format_rp : m_renderPass_pc)
        {
            destroyRenderPass(*m_pfnDev, m_device, format_rp.second);
        }
        destroyRenderPass(*m_pfnDev, m_device, m_renderPass_obj);

        // resources allocated with VMA
        if (m_localPool)
        {
            vmaDestroyPool(m_allocator, m_localPool);
            m_localPool = VK_NULL_HANDLE;
        }

        if (m_hostPool)
        {
            vmaDestroyPool(m_allocator, m_hostPool);
            m_hostPool = VK_NULL_HANDLE;
        }

        if (m_allocator)
        {
            vmaDestroyAllocator(m_allocator);
            m_allocator = VK_NULL_HANDLE;
        }

        for (uint32_t i = 0; i < m_queueFamilyCount; ++i)
        {
            delete m_ppQueue[i];
        }
        delete m_ppQueue;
        m_ppQueue = nullptr;

        delete[] m_queueFamilyProps;
        m_queueFamilyProps = nullptr;

        delete m_pfnDev;
        m_pfnDev = nullptr;

        m_pfn.vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }

    if (m_layerCount > 0) {
        m_pfn.vkDestroyDebugUtilsMessengerEXT(m_vkInstance, m_debugCallback, nullptr);
    }

    if (m_vkInstance) {
        m_pfn.vkDestroyInstance(m_vkInstance, nullptr);
        m_vkInstance = VK_NULL_HANDLE;
    }

    m_pfn.unloadInstanceFunctions();
}

void VulkanManager::cleanupSizeDependantResources(TlFramebuffer _fb)
{
    if (m_device && m_pfnDev) {
        m_pfnDev->vkDeviceWaitIdle(m_device);

        // pc color
        tls::vk::destroyImageView(*m_pfnDev, m_device, _fb->pcColorImageView);
        tls::vk::destroyImage(*m_pfnDev, m_device, _fb->pcColorImage);
        tls::vk::freeMemory(*m_pfnDev, m_device, _fb->pcColorMemory);

        // pc depth
        tls::vk::destroyImageView(*m_pfnDev, m_device, _fb->pcDepthImageView);
        tls::vk::destroyImage(*m_pfnDev, m_device, _fb->pcDepthImage);
        tls::vk::freeMemory(*m_pfnDev, m_device, _fb->pcDepthMemory);

        // corrected depth buffer
        tls::vk::destroyBuffer(*m_pfnDev, m_device, _fb->correctedDepthBuffer);
        tls::vk::freeMemory(*m_pfnDev, m_device, _fb->correctedDepthBufferMemory);

        // object depth
        tls::vk::destroyImageView(*m_pfnDev, m_device, _fb->objectDepthImageView);
        tls::vk::destroyImage(*m_pfnDev, m_device, _fb->objectDepthImage);
        tls::vk::freeMemory(*m_pfnDev, m_device, _fb->objectDepthMemory);

        // Id Attachment
        tls::vk::destroyImageView(*m_pfnDev, m_device, _fb->idAttImageView);
        tls::vk::destroyImage(*m_pfnDev, m_device, _fb->idAttImage);
        tls::vk::freeMemory(*m_pfnDev, m_device, _fb->idAttMemory);

        // Transparent object attachment
        tls::vk::destroyImageView(*m_pfnDev, m_device, _fb->transparentObjectImageView);
        tls::vk::destroyImage(*m_pfnDev, m_device, _fb->transparentObjectImage);
        tls::vk::freeMemory(*m_pfnDev, m_device, _fb->transparentObjectMemory);

        // Guizmo depth attachment
        tls::vk::destroyImageView(*m_pfnDev, m_device, _fb->gizmoDepthImageView);
        tls::vk::destroyImage(*m_pfnDev, m_device, _fb->gizmoDepthImage);
        tls::vk::freeMemory(*m_pfnDev, m_device, _fb->gizmoDepthMemory);

        // Copy Depth
        tls::vk::destroyBuffer(*m_pfnDev, m_device, _fb->copyBufDepth);
        tls::vk::destroyBuffer(*m_pfnDev, m_device, _fb->copyBufIndex);

        if (_fb->pMappedCopyDepth) {
            m_pfnDev->vkUnmapMemory(m_device, _fb->copyMemory);
            _fb->pMappedCopyDepth = nullptr;
        }

        tls::vk::freeMemory(*m_pfnDev, m_device, _fb->copyMemory);

        // Descriptor Set
        tls::vk::freeDescriptorSet(*m_pfnDev, m_device, m_descPool, _fb->descSetInputDepth);
        tls::vk::freeDescriptorSet(*m_pfnDev, m_device, m_descPool, _fb->descSetSamplers);
        tls::vk::freeDescriptorSet(*m_pfnDev, m_device, m_descPool, _fb->descSetCorrectedDepth);
        tls::vk::freeDescriptorSet(*m_pfnDev, m_device, m_descPool, _fb->descSetInputTransparentLayer);

        // Swap Chain
        tls::vk::destroyMultiImageView(*m_pfnDev, m_device, _fb->pImageViews, _fb->imageCount);

        tls::vk::destroyFramebuffer(*m_pfnDev, m_device, _fb->pcFramebuffer);
        if (_fb->finalFramebuffers) {
            for (uint32_t i = 0; i < _fb->imageCount; i++)
                tls::vk::destroyFramebuffer(*m_pfnDev, m_device, _fb->finalFramebuffers[i]);
            delete[] _fb->finalFramebuffers;
            _fb->finalFramebuffers = nullptr;
        }

        if (m_device && m_graphicsCmdPool && !_fb->graphicsCmdBuffers.empty()) {
            m_pfnDev->vkFreeCommandBuffers(m_device, m_graphicsCmdPool, (uint32_t)_fb->graphicsCmdBuffers.size(), _fb->graphicsCmdBuffers.data());
            _fb->graphicsCmdBuffers.clear();

        }

        if (_fb->swapchain)
        {
            tls::vk::destroySwapchain(*m_pfnDev, m_device, _fb->swapchain);
        }
        else
        {
            for (uint32_t i = 0; i < _fb->imageCount; ++i)
                tls::vk::destroyImage(*m_pfnDev, m_device, _fb->pImages[i]);
        }
        delete[] _fb->pImages;
        _fb->pImages = nullptr;

        tls::vk::freeMemory(*m_pfnDev, m_device, _fb->virtualImageMemory);

        for (VkSemaphore semaphore : _fb->imageAvailableSemaphore)
        {
            tls::vk::destroySemaphore(*m_pfnDev, m_device, semaphore);
        }
        _fb->imageAvailableSemaphore.clear();
        tls::vk::destroySemaphore(*m_pfnDev, m_device, _fb->renderFinishedSemaphore);

        tls::vk::destroySampler(*m_pfnDev, m_device, _fb->rawSampler);
    }

    // TODO - free the VkImages if there is no swapchain

}

void VulkanManager::cleanupPermanentResources(TlFramebuffer _fb)
{
    // Draw Buffers
    for (uint32_t i = 0; i < _fb->drawMarkerBuffers.size(); ++i)
    {
        freeAllocation(_fb->drawMarkerBuffers[i]);
    }
    _fb->drawMarkerBuffers.clear();
    for (uint32_t i = 0; i < _fb->drawMeasureBuffers.size(); ++i)
    {
        freeAllocation(_fb->drawMeasureBuffers[i]);
    }
    _fb->drawMeasureBuffers.clear();
}

template<class Buffer>
void pushToVectors(std::unordered_set<Buffer*>& sbuffers, std::vector<SimpleBuffer*>& tlBuffers, std::vector<VkBuffer>& buffers, std::vector<VmaAllocation>& allocations)
{
    for (Buffer* buffer : sbuffers)
    {
        buffers.push_back(buffer->buffer);
        allocations.push_back(buffer->alloc);
        tlBuffers.push_back(buffer);
    }
}

void VulkanManager::checkAllocations()
{
    assert(m_smartBufferAllocated.empty());
    assert(m_simpleBufferAllocated.empty());
    assert(m_pointsDevicePoolUsed == 0);
    assert(m_pointsHostPoolUsed == 0);
    assert(m_objectsDevicePoolUsed == 0);
    assert(m_objectsHostPoolUsed == 0);
}

void VulkanManager::defragmentMemory()
{
    //https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/defragmentation.html

    std::vector<VkBuffer> buffers;
    std::vector<VmaAllocation> allocations;
    std::vector<SimpleBuffer*> tlBuffers;

    pushToVectors(m_simpleBufferAllocated, tlBuffers, buffers, allocations);
    pushToVectors(m_smartBufferAllocated, tlBuffers, buffers, allocations);

    const uint32_t allocCount = (uint32_t)allocations.size();
    std::vector<VkBool32> allocationsChanged(allocCount);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_graphicsCmdPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    m_pfnDev->vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    m_pfnDev->vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VmaDefragmentationInfo defragInfo = {};
    defragInfo.flags;
    defragInfo.pool;
    defragInfo.maxBytesPerPass;
    defragInfo.maxAllocationsPerPass;

    VmaDefragmentationContext defragCtx;
    vmaBeginDefragmentation(m_allocator, &defragInfo, &defragCtx);

    m_pfnDev->vkEndCommandBuffer(commandBuffer);

    // Submit commandBuffer.
    // Wait for a fence that ensures commandBuffer execution finished.

    VmaDefragmentationStats defragStats;
    vmaEndDefragmentation(m_allocator, defragCtx, &defragStats);

    for (uint32_t i = 0; i < allocCount; ++i)
    {
        if (allocationsChanged[i])
        {
            // Destroy buffer that is immutably bound to memory region which is no longer valid.
            m_pfnDev->vkDestroyBuffer(m_device, buffers[i], nullptr);

            // Create new buffer with same parameters.

            VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
            bufferInfo.size = tlBuffers[i]->size;
            if(tlBuffers[i]->isLocalMem)
                bufferInfo.usage |=  VK_BUFFER_USAGE_TRANSFER_DST_BIT;

            m_pfnDev->vkCreateBuffer(m_device, &bufferInfo, nullptr, &tlBuffers[i]->buffer);

            // You can make dummy call to vkGetBufferMemoryRequirements here to silence validation layer warning.

            // Bind new buffer to new memory region. Data contained in it is already moved.
            VmaAllocationInfo allocInfo;
            vmaGetAllocationInfo(m_allocator, tlBuffers[i]->alloc, &allocInfo);
            vmaBindBufferMemory(m_allocator, tlBuffers[i]->alloc, tlBuffers[i]->buffer);
        }
    }
}
