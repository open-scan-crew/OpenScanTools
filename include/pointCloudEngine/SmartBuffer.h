#ifndef SMART_BUFFER_H
#define SMART_BUFFER_H

#include "vulkan/vulkan.h"
#include "VulkanMemoryAllocator/vk_mem_alloc.h"

#include <atomic>

typedef enum class TlDataState
{
    NOT_LOADED = 0,
    LOADING = 1,
    LOADED = 2,
    MAX_ENUM
} TlDataState;

constexpr uint32_t TL_FRAME_INDEX_LOST = UINT32_MAX;
constexpr uint32_t TL_PROCESS_LOST = UINT32_MAX;

class SimpleBuffer
{
public:
    VmaAllocation alloc = VK_NULL_HANDLE;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceSize size = 0;
    bool isLocalMem = false;
};

class SmartBuffer : public SimpleBuffer
{
public:
    std::atomic<TlDataState> state{ TlDataState::NOT_LOADED };
    std::atomic<uint32_t> lastUseFrameIndex{ 0 };
    std::atomic<uint32_t> ongoingProcesses{ 0 }; // works like a counter in a smart pointer
};

#endif