#ifndef TL_SWAPCHAIN_H
#define TL_SWAPCHAIN_H

#include "vulkan/vulkan.h"

#include <atomic>
#include <vector>

struct TlImage
{
    VkImage image = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
};

struct TlBuffer
{
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceSize size = 0;
    VkDeviceMemory memory = VK_NULL_HANDLE;
};

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

struct TlFramebuffer_T
{
    std::atomic<bool> mustRecreateSwapchain = false;
    int initLayout = 0;
    bool initColorLayout = true;

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSurfaceFormatKHR surfaceFormat;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkPresentModeKHR presentMode;
    VkFormat pcFormat = VK_FORMAT_UNDEFINED;
    VkFormat dsFormat = VK_FORMAT_UNDEFINED;
    uint32_t currentFrame;
    std::vector<VkSemaphore> imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
    uint32_t presentQFI = UINT32_MAX;
    VkExtent2D extent;
    VkExtent2D requestedExtent;

    uint32_t imageCount;
    uint32_t currentImage;
    VkImage* pImages = nullptr;
    VkImageView* pImageViews = nullptr;
    // Memory Optional for Virtual Viewport
    VkDeviceMemory virtualImageMemory = VK_NULL_HANDLE;

    std::vector<VkCommandBuffer> graphicsCmdBuffers;
    //VkCommandBuffer* computeCmdBuffers = nullptr;

    // Resources that are not presented -> they can be recycled for each frame
    // Intermidiary Color & depth Image for multipass rendering
    VkDeviceMemory pcColorMemory = VK_NULL_HANDLE;
    VkImage pcColorImage = VK_NULL_HANDLE;
    VkImageView pcColorImageView = VK_NULL_HANDLE;

    // Depth Usage :
    // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    // VK_IMAGE_USAGE_TRANSFER_SRC_BIT
    // VK_IMAGE_USAGE_SAMPLED_BIT
    // VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
    // Layout :
    // VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
    VkDeviceMemory pcDepthMemory = VK_NULL_HANDLE;
    VkImage pcDepthImage = VK_NULL_HANDLE;
    VkImageView pcDepthImageView = VK_NULL_HANDLE;

    // Depth buffer after gap correction
    VkDeviceMemory correctedDepthBufferMemory = VK_NULL_HANDLE;
    VkDeviceSize correctedDepthSize = 0;
    VkBuffer correctedDepthBuffer = VK_NULL_HANDLE;

    // Ambient Occlusion targets
    VkDeviceMemory aoMemory = VK_NULL_HANDLE;
    VkImage aoImage = VK_NULL_HANDLE;
    VkImageView aoImageView = VK_NULL_HANDLE;
    VkDeviceMemory aoBlurMemory = VK_NULL_HANDLE;
    VkImage aoBlurImage = VK_NULL_HANDLE;
    VkImageView aoBlurImageView = VK_NULL_HANDLE;

    // Depth & Stencil objects
    VkDeviceMemory objectDepthMemory = VK_NULL_HANDLE;
    VkImage objectDepthImage = VK_NULL_HANDLE;
    VkImageView objectDepthImageView = VK_NULL_HANDLE;

    // Attachments
    VkDeviceMemory idAttMemory = VK_NULL_HANDLE;
    VkImage idAttImage = VK_NULL_HANDLE;
    VkImageView idAttImageView = VK_NULL_HANDLE;

    // Transparent Render target
    VkDeviceMemory transparentObjectMemory = VK_NULL_HANDLE;
    VkImage transparentObjectImage = VK_NULL_HANDLE;
    VkImageView transparentObjectImageView = VK_NULL_HANDLE;

    // Depth (Stencil) gizmos 
    VkDeviceMemory gizmoDepthMemory = VK_NULL_HANDLE;
    VkImage gizmoDepthImage = VK_NULL_HANDLE;
    VkImageView gizmoDepthImageView = VK_NULL_HANDLE;

    // Copy Buffers (for picking and index)
    VkDeviceMemory copyMemory = VK_NULL_HANDLE;
    VkDeviceSize copyMemSize;
    VkBuffer copyBufDepth = VK_NULL_HANDLE;
    VkBuffer copyBufIndex = VK_NULL_HANDLE;
    void* pMappedCopyDepth = nullptr;
    void* pMappedCopyIndex = nullptr;

    VkFramebuffer pcFramebuffer = VK_NULL_HANDLE;
    VkFramebuffer* finalFramebuffers = nullptr;

    // Descriptor Set (for marker rendering input attachment)
    VkSampler rawSampler = VK_NULL_HANDLE;
    VkDescriptorSet descSetInputDepth = VK_NULL_HANDLE;
    VkDescriptorSet descSetSamplers = VK_NULL_HANDLE;
    VkDescriptorSet descSetCorrectedDepth = VK_NULL_HANDLE;
    VkDescriptorSet descSetInputTransparentLayer = VK_NULL_HANDLE;
    VkDescriptorSet descSetAO = VK_NULL_HANDLE;
    VkDescriptorSet descSetAOBlur = VK_NULL_HANDLE;
    VkDescriptorSet descSetAOBlurSwap = VK_NULL_HANDLE;
    VkDescriptorSet descSetAOCompose = VK_NULL_HANDLE;

    std::vector<SimpleBuffer> drawMarkerBuffers; // size of 'imageCount'
    std::vector<SimpleBuffer> drawMeasureBuffers; // size of 'imageCount'

    bool initAOLayout = true;
    bool initAOBlurLayout = true;
};


// Images Memory size
// 2 swap images  : 2 * color(R8G8B8A8)
// 6 attachements : pcColor (RGBA8/RGBA16), pcDepth (D32),
//                  correctedDepth (R32),
//                  objectDepth (D32), id (R32), transparent (RGBA16)
// 2 buffers :      copyDepth (D32), copyId (R32)
//
// Full HD (1920*1080) 9 * 8 Mio = 72Mio
// 4K (3840*2160) 9 * 32 Mio = 298Mio


#endif
