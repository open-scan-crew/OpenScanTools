// TODO(robin) Find a name for the Engine
// TODO(robin) Pass the command for the memory barrier of the depth buffer
// TODO(robin) Try to create manually the images for the swapchain with vkBindImageMemory2KHR() & VkBindImageMemorySwapchainInfoKHR()

// TODO(robin) find a better synchronisation mechanism with the transfer queue than vkQueueWaitIdle in copyBuffer()

#ifndef VULKAN_MANAGER_H
#define VULKAN_MANAGER_H

#include <thread>
#include <unordered_set>
#include <queue>
#include <future>
#include <type_traits>

#include "vulkan/vulkan.h"
#include "vulkan/VulkanFunctions.h"
#include "vulkan/VkUniformAllocator.h"
#include "vulkan/TlFramebuffer.h"
#include "vulkan/ImageTransferEvent.h"
#include "VulkanMemoryAllocator/vk_mem_alloc.h"
#include "pointCloudEngine/TlStreamer.h"
#include "pointCloudEngine/SmartBuffer.h"

#include "utils/Logger.h"

#define VK_LOG LoggerMode::VKLog

typedef void(*checkFct)(VkResult);
typedef void(*checkFctMsg)(VkResult, const char*);

static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    else if (err > 0)
        Logger::log(VK_LOG) << "WARNING Vkm: result" << err << Logger::endl;
    else if (err < 0)
    {
        Logger::log(VK_LOG) << "ERROR Vkm: result" << err << Logger::endl;
        assert(0);
    }
}

static void check_vk_result(VkResult err, const char* msg)
{
    if (err == 0)
        return;
    else if (err > 0)
        Logger::log(VK_LOG) << "WARNING Vkm: " << msg << " result " << err << Logger::endl;
    else if (err < 0)
    {
        Logger::log(VK_LOG) << "ERROR Vkm: " << msg << " result " << err << Logger::endl;
        assert(0);
    }
}


static inline VkDeviceSize aligned(VkDeviceSize v, VkDeviceSize byteAlign)
{
    return (v + byteAlign - 1) & ~(byteAlign - 1);
}

class VulkanManager
{
public:
    static VulkanManager& getInstance()
    {
        static VulkanManager instance;
        return instance;
    }

    // C++ 11 // Explicitly avoid to copy the singleton
    VulkanManager(VulkanManager const&) = delete;
    void operator=(VulkanManager const&) = delete;

private:
    VulkanManager() {};
    ~VulkanManager();

public:
    //std::unordered_map<uint32_t, std::string> getCompatibleDevice(bool enableValidationLayers, uint32_t extCount, const char** extensions);
    bool initVkInstance(bool enableValidationLayers, uint32_t extCount, const char** extensions);
    bool initPhysicalDevice(std::string preferedDevice, std::unordered_map<uint32_t, std::string>& allDevices);
    bool initResources();
    void stopAllRendering();

    void initStreaming();
    void stopStreaming();
    void startTransferThread();

    // Framebuffer management
    bool initFramebuffer(TlFramebuffer& framebuffer, uint64_t winId, int _width, int _height, bool preciseColor);
    void destroyFramebuffer(TlFramebuffer& framebuffer);
    bool createVirtualViewport(uint32_t width, uint32_t height, int multisample, TlFramebuffer& virtualViewport);

    ImageTransferEvent transferFramebufferImage(TlFramebuffer _framebuffer, VkCommandBuffer _cmdBuffer) const;
    bool doImageTransfer(ImageTransferEvent ite, uint32_t dstW, uint32_t dstH, char* dstBuffer, size_t dstSize, uint32_t dstOffsetW, uint32_t dstOffsetH, uint32_t border) const;
    static void resizeFramebuffer(TlFramebuffer, int _width, int _height);
    static VkCommandBuffer getGraphicsCmdBuffer(TlFramebuffer framebuffer);
    static uint32_t getImageCount(TlFramebuffer framebuffer);

    VkInstance getVkInstance() const;
    VkRenderPass getPCRenderPass(VkFormat imageFormat) const;
    VkRenderPass getObjectRenderPass() const;
    uint32_t getGraphicsQFI() const;
    VkQueue getGraphicsQueue() const;
    VkPhysicalDevice getPhysicalDevice() const;
    VkDevice getDevice() const;
    VulkanDeviceFunctions* getDeviceFunctions() const;
    VkDescriptorPool getDescriptorPool() const;
    static VkDescriptorSetLayout getDSLayout_inputDepth();
    static VkDescriptorSetLayout getDSLayout_fillingSamplers();
    static VkDescriptorSetLayout getDSLayout_finalOutput();
    static VkDescriptorSetLayout getDSLayout_inputTransparentLayer();

    VkBuffer getUniformBuffer() const;
    checkFct getCheckFunction() const;

    // Limits
    VkPhysicalDeviceLimits getPhysicalDeviceLimits();

    float sampleDepth(TlFramebuffer framebuffer);
    uint32_t sampleIndex(TlFramebuffer framebuhher, uint32_t posX, uint32_t posY);
    std::vector<uint32_t> sampleIndexList(TlFramebuffer _fb, uint32_t posX, uint32_t posY, uint32_t range);

    // Frame drawing functions
    // Return the frame index of the frame in preparation
    void startNextFrame();
    bool acquireNextImage(TlFramebuffer framebuffer);
    void beginCommandBuffer(VkCommandBuffer cmdBuffer);
    void resetCommandBuffer(VkCommandBuffer cmdBuffer);
    void endCommandBuffer(VkCommandBuffer cmdBuffer);
    void beginScanRenderPass(TlFramebuffer framebuffer, VkClearColorValue clearColor);
    void endScanRenderPass(TlFramebuffer _fb);

    // Point cloud post treatment
    void beginPostTreatmentFilling(TlFramebuffer framebuffer);
    void beginPostTreatmentNormal(TlFramebuffer framebuffer);
    void beginPostTreatmentTransparency(TlFramebuffer framebuffer);

    // 3D Objects Render pass
    void beginObjectRenderPass(TlFramebuffer framebuffer);
    void endObjectRenderPass(TlFramebuffer framebuffer);
    void applyPicking(TlFramebuffer framebuffer, glm::ivec2 pickingPos, VkCommandBuffer* singleCommandBuffer);

    void submitMultipleFramebuffer(std::vector<TlFramebuffer> fbs);
    void submitVirtualFramebuffer(TlFramebuffer fb);
    void waitIdle();
    void waitForStreamingIdle();

    uint32_t getCurrentFrameIndex() const;
    // For all the frame index inferior or equal to the "safe frame index" it is guaranted
    // that all the command buffers are completed and the gpu resources are free to manipulate
    uint32_t getSafeFrameIndex();

    // Uniform
    uint32_t getUniformSizeAligned(uint32_t dataSize);
    bool allocUniform(uint32_t dataSize, uint32_t imageCount, VkMultiUniform& uniform);
    bool allocUniform(uint32_t dataSize, TlFramebuffer framebuffer, VkMultiUniform& uniform);
    void freeUniform(VkMultiUniform& uniform);
    void loadUniformData(uint32_t dataSize, const void* pData, uint32_t localOffset, uint32_t swapIndex, const VkMultiUniform& uniform) const;
    void loadConstantUniform(uint32_t dataSize, const void* pData, VkMultiUniform& uniform);

    VkResult allocateMemory(VkDeviceMemory& memory, VkMemoryRequirements requirements, VkMemoryPropertyFlags properties, const char* logName) const;

    // Transfer functions
    bool loadInSimpleBuffer(SimpleBuffer& smpBuf, VkDeviceSize dataSize, const void* pData, VkDeviceSize& bufOffset, VkDeviceSize byteAlign);
    bool downloadSimpleBuffer(const SimpleBuffer& smpBuf, void* pData, VkDeviceSize dataSize);
    void* getMappedPointer(SmartBuffer& sbuf);

    // Allocation
    void printAllocationStats(std::string& stats);

    // SmartBuffer Managment
    VkResult allocSmartBuffer(VkDeviceSize dataSize, SmartBuffer& sbuf, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags);
    VkResult allocSimpleBuffer(VkDeviceSize dataSize, SimpleBuffer& sbuf, VkBufferUsageFlags usageFlags);

    bool touchSmartBuffer(SmartBuffer& sbuf);
    static bool lockSmartBuffer(SmartBuffer& sbuf);
    static bool unlockSmartBuffer(SmartBuffer& sbuf);
    bool safeFreeAllocation(SmartBuffer& sbuf);

    void freeAllocation(SmartBuffer& sbuf);
    void freeAllocation(SimpleBuffer& sbuf);
    bool freeDeviceMemory(VkDeviceSize memoryNeeded);
    bool freeHostMemory(VkDeviceSize memoryNeeded);

    // Texture
    bool createTextureArray(uint32_t width, uint32_t height, uint32_t layerCount, VkImage& image, VkImageView& imageView, VkSampler& sampler, VkDeviceMemory& memory);
    bool uploadTextureArray(const void* data, uint32_t width, uint32_t height, VkImage& image, uint32_t layer, uint32_t layerCount);

    // benchmark
    void startRecordingStats();
    void stopRecordingStats();

private:
    bool createVulkanInstance();
    bool setupDebugCallback();
    bool selectPhysicalDevice(std::string preferedDevice, std::unordered_map<uint32_t, std::string>& compliantDevices);
    bool createLogicalDevice();

    bool initVma();

    void findDepthFormat();
    bool createPCRenderPass(VkFormat imageFormat);
    bool createObjectRenderPasses();
    bool createCommandPools();
    bool createDescriptorPool();
    bool createDescriptorSetLayouts();
    bool createFences();
    bool createStagingMemory();
    bool createUniformBuffer();

    // Framebuffer Resources Creation (independant of the size)
    void createDrawBuffers(TlFramebuffer _fb);

    // Framebuffer Resource Creation
    bool createSwapchain(TlFramebuffer _fb);
    void createVirtualRenderImages(TlFramebuffer _fb);
    void createDepthBuffers(TlFramebuffer _fb);
    void createAttachments(TlFramebuffer _fb);
    void createCopyBuffers(TlFramebuffer _fb);
    void allocateDescriptorSet(TlFramebuffer _fb);

    void createPcFramebuffer(TlFramebuffer _fb);
    void createFinalFramebuffers(TlFramebuffer _fb);
    void createCommandBuffers(TlFramebuffer _fb);
    void createSemaphores(TlFramebuffer _fb);

    // Swapchain Configuration Functions
    VkSurfaceFormatKHR selectSwapchainSurfaceFormat(VkSurfaceKHR surface, VkFormat preferedFormat);
    VkPresentModeKHR selectSwapchainPresentMode(VkSurfaceKHR surface);
    VkExtent2D selectSwapchainExtent(VkSurfaceKHR surface, VkExtent2D requestedExtent);
    uint32_t findPresentQueueFamily(VkSurfaceKHR surface);
    uint32_t selectImageCount(VkSurfaceKHR surface);

    void copyBufferToImage(VkCommandBuffer cmdBuf, VkBuffer buffer, VkImage image, VkImageAspectFlags aspectFlags, uint32_t layer, VkExtent3D extent);
    void copyImageToBuffer(VkCommandBuffer cmdBuf, VkImage image, VkImageAspectFlags, uint32_t layer, VkExtent3D extent, VkBuffer buffer);
    void pipelineBarrier(VkCommandBuffer cmdBuf, VkImage image, VkImageLayout inLayout, VkImageLayout outLayout, VkImageAspectFlags aspectFlags);

    VkCommandBuffer beginTransferCommand();
    void endTransferCommand(VkCommandBuffer cmdBuffer);

    template<class F, class... Args>
    auto enqueueTransfer(F&& f, Args&&... args)
        ->std::future<typename std::invoke_result_t<F, Args...>>
    {
        using return_type = typename std::invoke_result_t<F, Args... >;

        auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(m_transferMutex);

            // don't allow enqueueing after stopping the pool
            if (m_stopTransferThread)
                throw std::runtime_error("enqueue on stopped ThreadPool");

            m_transferTasks.emplace([task]() { (*task)(); });
        }
        m_transfer_cv.notify_one();
        return res;
    };


    bool loadInSimpleBuffer_local(SimpleBuffer& smpBuf, VkDeviceSize dataSize, const void* pData, VkDeviceSize& bufOffset, VkDeviceSize byteAlign);
    bool loadInSimpleBuffer_host(SimpleBuffer& smpBuf, VkDeviceSize dataSize, const void* pData, VkDeviceSize& bufOffset, VkDeviceSize byteAlign);
    bool loadTextureArray_async(const void* imageData, uint32_t width, uint32_t height, VkImage& image, uint32_t layer, uint32_t layerCount);
    bool downloadSimpleBuffer_async(const SimpleBuffer& smpBuf, void* pData, VkDeviceSize dataSize);

    bool checkSwapchain(TlFramebuffer _fb);


    // Buffer allocation
    VkResult tryAllocBuffer(VkBufferCreateInfo& bufferCreateInfo, VmaAllocationCreateInfo& allocationCreateInfo, SimpleBuffer* sbuf);

    // Helper Functions
    void checkValidationLayerSupport();
    void checkInstanceExtensionSupport();
    bool checkDeviceExtensionSupport(VkPhysicalDevice device, bool printInfo);
    bool checkAvailableFeatures(VkPhysicalDevice device, bool select);
    bool checkQueueFamilies(VkPhysicalDevice device, bool select, bool printInfo);
    bool checkMemoryType(VkPhysicalDevice device, bool select, bool printInfo);

    bool isDeviceSuitable(VkPhysicalDevice device);
    struct QueueID
    {
        uint32_t family;
        uint32_t index;
    };
    QueueID findQueueAvailable(VkQueueFlags queueFlag, std::vector<uint32_t>& queueUsed, VkQueueFamilyProperties* queueFamilyProperties);
    inline VkQueue getQueue(QueueID queueID) const;
    uint32_t findMemoryType(VkPhysicalDevice device, VkMemoryPropertyFlags properties) const;

    // Vulkan Object creation helper functions
    VkResult createBuffer(VkBuffer& _buf, VkBufferUsageFlags _usage, VkDeviceSize _size);
    void createImage(VkExtent2D extent, uint32_t layerCount, VkFormat format, VkImageTiling tiling, VkSampleCountFlagBits sampleCount, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) const;
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const;
    VkSampler createTextureSampler();

    // FormatProperties
    void logFormatFeatures(VkFormat format, VkFormatFeatureFlags flags);
    bool checkBlitSupport(VkFormat srcFormat, VkFormat dstFormat, VkImageTiling tilling) const;

    // Memory utilities
    void checkAllocations();
    void defragmentMemory();

    // Cleanup Functions
    void cleanupAll();
    void cleanupSizeDependantResources(TlFramebuffer _fb);
    void cleanupPermanentResources(TlFramebuffer _fb);

private:
    // Validation layers
    uint32_t m_layerCount = 0;
    const char** m_layers = nullptr;

    // Extensions
    uint32_t m_extCount = 0;
    char** m_extensions = nullptr;

    // Device extensions
    uint32_t m_devExtCount = 0;
    const char** m_devExtensions = nullptr;

    // Vulkan Ressources
    VulkanFunctions m_pfn;
    VulkanDeviceFunctions* m_pfnDev = nullptr;

    VkInstance m_vkInstance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugCallback = VK_NULL_HANDLE;
    VkPhysicalDevice m_physDev = VK_NULL_HANDLE;
    VkPhysicalDeviceFeatures m_features;
    VkPhysicalDeviceProperties m_physDevProperties;
    VkDevice m_device = VK_NULL_HANDLE;
    VmaAllocator m_allocator = VK_NULL_HANDLE;
    std::atomic<uint32_t> m_currentFrameIndex = 0;

    // Queue Families
    uint32_t m_queueFamilyCount = 0;
    VkQueueFamilyProperties* m_queueFamilyProps = nullptr;
    VkQueue** m_ppQueue = nullptr;

    // Queue family indices (QFI)
    QueueID m_graphicsQID;
    QueueID m_transferQID;
    QueueID m_computeQID;
    QueueID m_streamingQID;

    VkFormat m_imageFormat = VK_FORMAT_UNDEFINED;
    VkFormat m_depthFormat = VK_FORMAT_UNDEFINED;
    VkRenderPass m_renderPass_obj = VK_NULL_HANDLE;
    std::unordered_map<VkFormat, VkRenderPass> m_renderPass_pc;

    // Commons Descriptor Layouts
    VkDescriptorSetLayout m_descSetLayout_inputDepth = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descSetLayout_filling = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descSetLayout_finalOutput = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descSetLayout_inputTransparentLayer = VK_NULL_HANDLE;

    // Commons Resources pools
    VkCommandPool m_graphicsCmdPool = VK_NULL_HANDLE;
    VkCommandPool m_transferCmdPool = VK_NULL_HANDLE;
    VkCommandPool m_computeCmdPool = VK_NULL_HANDLE;
    // TODO - Gérer les command pool thread par thread
    //std::unordered_map<std::thread::id, VkCommandPool> cmdPools;
    //std::vector<std::unordered_map<std::thread::id, VkCommandBuffer>> cmdBuffers;
    /* The transfer command buffer is allocated once.
       It is then reset with the begin instruction after each submission.
     */
    VkCommandBuffer m_transferCmdBuf = VK_NULL_HANDLE;
    VkDescriptorPool m_descPool = VK_NULL_HANDLE;

    // Constant Memory allocation
    VkDeviceMemory m_uniformMem = VK_NULL_HANDLE;
    VkBuffer m_uniformBuf = VK_NULL_HANDLE;
    VkUniformAllocator m_uniformAllocator;

    // Staging Memory
    VkDeviceMemory m_stagingMem = VK_NULL_HANDLE;
    VkBuffer m_stagingBuf = VK_NULL_HANDLE;
    const VkDeviceSize m_stagingSize = 2 * 1024 * 1024;
    void *m_pStaging = nullptr;

    // Synchronization Objects
    VkFence m_renderFence = VK_NULL_HANDLE;

    // Vulkan Memory Allocator
    VmaPool m_hostPool = VK_NULL_HANDLE;
    VkDeviceSize m_hostPoolMaxSize = 0;
    VmaPool m_localPool = VK_NULL_HANDLE;
    VkDeviceSize m_localPoolMaxSize = 0;

    uint32_t m_memTypeIndex_local = 0;
    uint32_t m_memTypeIndex_host = 0;
    uint32_t m_deviceHeapIndex = 0;
    uint32_t m_hostHeapIndex = 0;

    std::mutex m_mutexBufferAllocated;
    std::unordered_set<SmartBuffer*> m_smartBufferAllocated;
    std::unordered_set<SimpleBuffer*> m_simpleBufferAllocated;
    
    VkDeviceSize m_pointsDevicePoolUsed = 0;
    VkDeviceSize m_pointsHostPoolUsed = 0;
    VkDeviceSize m_objectsDevicePoolUsed = 0;
    VkDeviceSize m_objectsHostPoolUsed = 0;
    std::atomic<bool> m_noMoreFreeMemoryForFrame_device = false;
    std::atomic<bool> m_noMoreFreeMemoryForFrame_host = false;
    uint32_t m_missedDeviceAllocations = 0;
    uint32_t m_missedHostAllocations = 0;

    // Streaming Resources
    TlStreamer* m_streamer = nullptr;
    std::thread m_streamingThread;

    // Transfer Resources
    bool m_stopTransferThread;
    std::mutex m_transferMutex;
    std::condition_variable m_transfer_cv;
    std::queue<std::function<void()>> m_transferTasks;
    std::thread m_transferThread;

    // Use to free safely any buffers rendered on any framebuffer
    uint32_t m_maxSafeFrame = 0;
};

/*
template<class F, class... Args>
auto VulkanManager::enqueueTransfer(F&& f, Args&&... args)
//-> std::future<decltype(f(args...))>
-> std::future<typename std::invoke_result_t<F, Args...>>
{
    using return_type = typename std::invoke_result_t<F, Args... >;

    auto task = std::make_shared< std::packaged_task<return_type()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(m_transferMutex);

        // don't allow enqueueing after stopping the pool
        if (m_stopTransferThread)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        m_transferTasks.emplace([task]() { (*task)(); });
    }
    m_transfer_cv.notify_one();
    return res;
};*/

#endif