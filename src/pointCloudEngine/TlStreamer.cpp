#include "pointCloudEngine/TlStreamer.h"
#include "pointCloudEngine/TlScanOverseer.h"

#include "vulkan/VulkanManager.h"
#include "vulkan/VulkanHelperFunctions.h"
#include "utils/ClockLock.hpp"
#include "utils/Logger.h"
#include "utils/System.h"

#include <chrono>


TlStreamer::TlStreamer(VkDevice device, VulkanDeviceFunctions* pfn, VkQueue tsfQueue, uint32_t queueFI, VkDeviceSize stageSize)
    : h_device(device)
    , h_pfn(pfn)
    , h_tsfQueue(tsfQueue)
    , m_stageSize(stageSize)
{
    VkResult err;

    // Create Stage memory
    VkBufferCreateInfo bufInfo = {};
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.size = m_stageSize;
    bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    err = h_pfn->vkCreateBuffer(h_device, &bufInfo, nullptr, &m_stageVkBuf);
    check_vk_result(err, "Create Staging Buffer");

    VkMemoryRequirements memReq;
    h_pfn->vkGetBufferMemoryRequirements(h_device, m_stageVkBuf, &memReq);

    err = VulkanManager::getInstance().allocateMemory(m_stageVkMem, memReq, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "Streamer Staging Buffer");
    check_vk_result(err, "Allocate Staging Memory");

    err = h_pfn->vkBindBufferMemory(h_device, m_stageVkBuf, m_stageVkMem, 0);
    check_vk_result(err, "Bind Buffer and Staging Memory");

    err = h_pfn->vkMapMemory(h_device, m_stageVkMem, 0, m_stageSize, 0, &m_pStage);
    check_vk_result(err, "Map staging memory");

    // Create Command Buffers
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmdPoolInfo.queueFamilyIndex = queueFI;
    err = h_pfn->vkCreateCommandPool(h_device, &cmdPoolInfo, nullptr, &m_tsfCmdPool);
    check_vk_result(err, "Create Transfer Command Pool");

    VkCommandBufferAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool = m_tsfCmdPool;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = 1;
    err = h_pfn->vkAllocateCommandBuffers(h_device, &info, &m_tsfCmdBuf);
    check_vk_result(err, "Allocate Transfer Command Buffer");

    // Create transfer fence
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    err = h_pfn->vkCreateFence(h_device, &fenceInfo, nullptr, &m_transferFence);
    check_vk_result(err, "Create Fence");

    m_vulkanInitialized.store(true);
}

TlStreamer::~TlStreamer()
{
    Logger::log(LoggerMode::VKLog) << "Destroying TlStreamer..." << Logger::endl;
    printTimes();

    // Unmap & Free Memory
    if (m_pStage != nullptr)
    {
        h_pfn->vkUnmapMemory(h_device, m_stageVkMem);
        m_pStage = nullptr;
    }

    tls::vk::destroyBuffer(*h_pfn, h_device, m_stageVkBuf);
    tls::vk::freeMemory(*h_pfn, h_device, m_stageVkMem);

    if (h_device && m_tsfCmdPool && m_tsfCmdBuf) {
        h_pfn->vkFreeCommandBuffers(h_device, m_tsfCmdPool, 1, &m_tsfCmdBuf);
        m_tsfCmdBuf = VK_NULL_HANDLE;
    }

    tls::vk::destroyCommandPool(*h_pfn, h_device, m_tsfCmdPool);
    tls::vk::destroyFence(*h_pfn, h_device, m_transferFence);
}

void TlStreamer::start()
{
    bool vFalse = false;
    if (m_processRunning.compare_exchange_strong(vFalse, true) == false)
        return;

    ClockLock clock(TL_STREAMER_FRAMERATE);
    m_stopProcess = false;

    while (m_stopProcess.load() == false)
    {
        process();

        clock.frame();
    }

    m_sleepedTime = (float)clock.getSecondsSleeped();

    m_processRunning.store(false);
}

void TlStreamer::stop()
{
    m_stopProcess.store(true);
}

void TlStreamer::waitIdle(uint32_t frameIndex)
{
    std::promise<bool> result;
    std::future<bool> result_future = result.get_future();
    m_traceResult = std::move(result);
    m_traceFrameIndex = frameIndex;
    bool watwat = result_future.get();
}

void TlStreamer::startRecordingFrame()
{
    m_recording.store(true);
}

void TlStreamer::stopRecordingFrame()
{
    m_recording.store(false);
    m_exportFrameRecorded.store(true);
}

void TlStreamer::process()
{
    if (m_vulkanInitialized.load() == false)
        return;

    m_frameCount++;
    uint32_t vkmFrameIndex = VulkanManager::getInstance().getCurrentFrameIndex();
    bool trace = (vkmFrameIndex == m_traceFrameIndex);

    std::chrono::steady_clock::time_point t1, t2, t3, t4;

    // 1. Copy files
    t1 = std::chrono::steady_clock::now();
    TlScanOverseer::getInstance().refreshResources();

    // 2. Stream scans (alloc & free)
    t2 = std::chrono::steady_clock::now();
    std::vector<TlStagedTransferInfo> stagedTransfer;
    TlScanOverseer::getInstance().streamScans(m_stageSize, (char*)m_pStage, stagedTransfer);

    // 3. Submit the Vulkan transfer from the staging buffer to the local buffers
    t3 = std::chrono::steady_clock::now();
    submitVulkanTransfers(stagedTransfer);

    t4 = std::chrono::steady_clock::now();

    if (trace && stagedTransfer.empty())
    {
        m_traceResult.set_value(true);
        m_traceFrameIndex = -1;
    }

    // ------------- STATS ------------------------ //

    Timing currTiming = {
        std::chrono::duration<float, std::ratio<1>>(t4 - t1).count(),
        std::chrono::duration<float, std::ratio<1>>(t2 - t1).count(),
        std::chrono::duration<float, std::ratio<1>>(t3 - t2).count(),
        std::chrono::duration<float, std::ratio<1>>(t4 - t3).count(),
    };

    m_cumulTiming += currTiming;
    m_maxTiming.max(currTiming);


    if (m_recording.load() == true)
    {
        m_recordedFrame.push_back({ currTiming, m_currData });
    }

    if (m_exportFrameRecorded.load() == true)
    {
        exportStats();

        m_recordedFrame.clear();
        m_exportFrameRecorded.store(false);
    }
}

void TlStreamer::printTimes()
{
    double totalTime = m_cumulTiming.worked + m_sleepedTime;

    double freeGb = (double)m_totalFreeSpace / 1073741824.0;
    double refreshRatio = m_cumulTiming.refresh / m_cumulTiming.worked * 100.f;
    double transferFileRatio = m_cumulTiming.transferFileAndFreeAlloc / m_cumulTiming.worked * 100.f;
    double transferGpuRatio = m_cumulTiming.transferGpu / m_cumulTiming.worked * 100.f;

    Logger::log(LoggerMode::VKLog) << "//--------- Streamer Working Stats ---------//" << std::setprecision(3) << Logger::endl;
    Logger::log(LoggerMode::VKLog) << "// Free space (Gb):   " << freeGb << Logger::endl;
    Logger::log(LoggerMode::VKLog) << "// Worked time (s):   " << m_cumulTiming.worked << Logger::endl;
    Logger::log(LoggerMode::VKLog) << "// Sleeped time (s):  " << m_sleepedTime << Logger::endl;
    Logger::log(LoggerMode::VKLog) << "// Frame per second:  " << (float)m_frameCount / totalTime << Logger::endl;
    Logger::log(LoggerMode::VKLog) << "//- - - - - - - - - - - - - - - - - - - - - //" << Logger::endl;
    Logger::log(LoggerMode::VKLog) << "// Refresh time:      " << m_cumulTiming.refresh << " (" << refreshRatio << "%) [" << m_maxTiming.refresh * 1000 << "ms]" << Logger::endl;
    Logger::log(LoggerMode::VKLog) << "// Free, Alloc, Read file time:    " << m_cumulTiming.transferFileAndFreeAlloc << " (" << transferFileRatio << "%) [" << m_maxTiming.transferFileAndFreeAlloc * 1000 << "ms]" << Logger::endl;
    Logger::log(LoggerMode::VKLog) << "// Transfer GPU time: " << m_cumulTiming.transferGpu << " (" << transferGpuRatio << "%) [" << m_maxTiming.transferGpu * 1000 << "ms]" << Logger::endl;
    Logger::log(LoggerMode::VKLog) << "//- - - - - - - - - - - - - - - - - - - - - //" << Logger::endl;
}

void TlStreamer::exportStats()
{
    std::ofstream ostr;

    if (Utils::System::openFileStream(std::string("Benchmark"), std::string("stream_data"), true, std::string("csv"), ostr) == false)
    {
        Logger::log(LoggerMode::VKLog) << "Streamer - Error while exporting the benchmark stats, cannot create the output file" << Logger::endl;
        return;
    }

    // Timing headers
    ostr << "Worked time, ";
    ostr << "Free space, ";
    ostr << "Refresh resources, ";
    ostr << "Read File, ";
    ostr << "Transfer GPU, ";
    // Data headers
    ostr << "Transfer Size, ";
    ostr << "Miss Size, ";
    ostr << "Free Size, ";
    ostr << "Node Count, ";
    ostr << "Contiguous Nodes, ";
    ostr << "File Count";
    // EOL
    ostr << "\n";

    for (FrameStats frame : m_recordedFrame)
    {
        // Timing
        ostr << frame.timing.worked << ", ";
        ostr << frame.timing.refresh << ", ";
        ostr << frame.timing.transferFileAndFreeAlloc << ", ";
        ostr << frame.timing.transferGpu << ", ";
        // Data
        ostr << frame.dataStats.transferSize << ", ";
        ostr << frame.dataStats.missingSize << ", ";
        ostr << frame.dataStats.freeSize << ", ";
        ostr << frame.dataStats.nodeCount << ", ";
        ostr << frame.dataStats.contiguousNodes << ", ";
        ostr << frame.dataStats.fileCount << ", ";
        // end of line
        ostr << "\n";
    }

    ostr.flush();
    ostr.close();
}


//--------------------------------------------------------//

void TlStreamer::submitVulkanTransfers(const std::vector<TlStagedTransferInfo>& _stagedTransfers)
{
    if (_stagedTransfers.empty())
        return;

    h_pfn->vkWaitForFences(h_device, 1, &m_transferFence, VK_TRUE, UINT64_MAX);
    h_pfn->vkResetFences(h_device, 1, &m_transferFence);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    // Begin (and reset) the transfer command buffer
    h_pfn->vkBeginCommandBuffer(m_tsfCmdBuf, &beginInfo);

    for (uint32_t i = 0; i < _stagedTransfers.size(); i++)
    {
        // Copy the buffers
        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = _stagedTransfers[i].stageOffset;
        copyRegion.dstOffset = 0;
        copyRegion.size = _stagedTransfers[i].dataSize;
        h_pfn->vkCmdCopyBuffer(m_tsfCmdBuf, m_stageVkBuf, _stagedTransfers[i].sbuf.buffer, 1, &copyRegion);
    }

    // End the command buffer
    h_pfn->vkEndCommandBuffer(m_tsfCmdBuf);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_tsfCmdBuf;

    h_pfn->vkQueueSubmit(h_tsfQueue, 1, &submitInfo, m_transferFence);

    // TODO - try to remove the wait -> a fence should suffice
    h_pfn->vkQueueWaitIdle(h_tsfQueue);

    for (uint32_t i = 0; i < _stagedTransfers.size(); i++)
    {
        _stagedTransfers[i].sbuf.state.store(TlDataState::LOADED);
    }
}
