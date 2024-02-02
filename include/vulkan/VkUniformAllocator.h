#ifndef VK_UNIFORM_ALLOCATOR_H
#define VK_UNIFORM_ALLOCATOR_H

#include "vulkan/vulkan.h"
#include "vulkan/VkUniform.h"

// A simple buffer sub allocator for allocating small spaces inside a single Vulkan Memory
//  - offset and alignment are local.
//

class VkUniformAllocator
{
public:
    VkUniformAllocator() {};
    ~VkUniformAllocator();

    void init(uint32_t totalMemory, uint32_t memAlignment, uint32_t minSize = 0);
    bool alloc(uint32_t size, uint32_t frameCount, VkMultiUniform& uniform) noexcept;
    void free(VkMultiUniform& uniform) noexcept;

private:
    // Constant Memory allocation
    uint32_t m_memorySize = 0;
    uint32_t m_blockSize = 0;
    uint32_t m_blockCount = 0;
    uint32_t m_firstFreeBlock = 0;
    bool* m_blockStates = nullptr;
};

#endif