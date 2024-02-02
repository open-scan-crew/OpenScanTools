#ifndef TL_STREAMER_H
#define TL_STREAMER_H

#include "models/pointCloud/TLS.h"
#include "pointCloudEngine/PCE_stream.h"
#include "vulkan/vulkan.h"

#include <unordered_map>
#include <set>
#include <vector>
#include <atomic>
#include <algorithm>
#include <future>


#define TL_STREAMER_FRAMERATE 50

class VulkanDeviceFunctions;

/** \brief Create a implementation of the TlStreamer
- Responsibilities of the Streamer:
  - manage all the transfers that occur between the system and the GPU
  - ask reserved memory to the host and the GPU via VulkanManager
*/
class TlStreamer
{
public:
    TlStreamer(VkDevice hDevice, VulkanDeviceFunctions* pfn, VkQueue tsfQueue, uint32_t queueFI, VkDeviceSize stageSize);
    ~TlStreamer();

    // Disable copy
    TlStreamer(TlStreamer const&) = delete;
    void operator=(TlStreamer const&) = delete;

    // main function
    void start();
    void stop();

    // Activity tracker
    void waitIdle(uint32_t frameIndex);

    // benchmark
    void startRecordingFrame();
    void stopRecordingFrame();

private:
    void process();
    void printTimes();
    void exportStats();

    // GPU commands and transfer finilization
    void submitVulkanTransfers(const std::vector<TlStagedTransferInfo>& _stagedTransfers);

    std::atomic<bool> m_processRunning = false;
    std::atomic<bool> m_stopProcess = false;

    std::atomic<bool> m_recording = false;
    std::atomic<bool> m_exportFrameRecorded = false;

    // Vulkan Resources
    std::atomic<bool> m_vulkanInitialized = false;
    VkDevice h_device = VK_NULL_HANDLE;
    VulkanDeviceFunctions* h_pfn = nullptr;
    VkQueue h_tsfQueue = VK_NULL_HANDLE;
    VkCommandPool m_tsfCmdPool = VK_NULL_HANDLE;
    VkCommandBuffer m_tsfCmdBuf = VK_NULL_HANDLE;

    //uint32_t m_stageCount;
    VkDeviceMemory m_stageVkMem = VK_NULL_HANDLE;
    VkBuffer m_stageVkBuf = VK_NULL_HANDLE;
    void *m_pStage = nullptr;
    const VkDeviceSize m_stageSize = 32 * 1024 * 1024;
    VkFence m_transferFence = VK_NULL_HANDLE;

    // stats
    struct DataStats
    {
        uint64_t transferSize;
        uint64_t missingSize;
        uint64_t freeSize;
        uint32_t nodeCount;
        uint32_t contiguousNodes;
        uint32_t fileCount;
    };

    DataStats m_currData = { 0, 0, 0, 0, 0, 0 };

    uint64_t m_totalFreeSpace = 0;
    uint32_t m_frameCount = 0;
    uint32_t m_traceFrameIndex = -1;
    std::promise<bool> m_traceResult;

    struct Timing
    {
        double worked = 0.0;
        //double freeSpace = 0.0; // free in the same step as alloc
        double refresh = 0.0;
        double transferFileAndFreeAlloc = 0.0;
        double transferGpu = 0.0;

        void operator+=(const Timing& ft)
        {
            this->worked += ft.worked;
            this->refresh += ft.refresh;
            this->transferFileAndFreeAlloc += ft.transferFileAndFreeAlloc;
            this->transferGpu += ft.transferGpu;
        }

        void max(const Timing& ft)
        {
            worked = std::max(worked, ft.worked);
            refresh = std::max(refresh, ft.refresh);
            transferFileAndFreeAlloc = std::max(transferFileAndFreeAlloc, ft.transferFileAndFreeAlloc);
            transferGpu = std::max(transferGpu, ft.transferGpu);
        }
    };

    struct FrameStats
    {
        Timing timing;
        DataStats dataStats;
    };

    double m_sleepedTime = 0.0;
    Timing m_cumulTiming;
    Timing m_maxTiming;
    std::vector<FrameStats> m_recordedFrame;
};

#endif // TL_STREAMER_H