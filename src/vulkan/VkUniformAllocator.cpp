#include "vulkan/VkUniformAllocator.h"

#include <cassert>
#include <cstring>

static inline uint32_t aligned(uint32_t v, uint32_t byteAlign)
{
    return (v + byteAlign - 1) & ~(byteAlign - 1);
}

void VkUniformAllocator::init(uint32_t memorySize, uint32_t alignment, uint32_t minSize)
{
    m_memorySize = memorySize;
    m_blockSize = aligned(minSize, alignment);
    m_blockCount = m_memorySize / m_blockSize;

    assert(m_memorySize);
    assert(m_blockCount);
    assert(!m_blockStates);

    m_blockStates = new bool[m_blockCount];
    memset(m_blockStates, 0, sizeof(bool) * m_blockCount);
}

VkUniformAllocator::~VkUniformAllocator()
{
    // Check that all the uniform have been freed
    assert(!m_firstFreeBlock);
#ifdef _DEBUG_
    for (uint32_t i = 0; i < m_blockCount; i++)
    {
        assert(!m_blockStates[i]);
    }
#endif
    if (m_blockStates)
        delete[] m_blockStates;
}

bool VkUniformAllocator::alloc(uint32_t size, uint32_t frameCount, VkMultiUniform& uniform) noexcept
{
    uint32_t sizeAligned = aligned(size, m_blockSize);
    uint32_t blockNeeded = sizeAligned / m_blockSize * frameCount;
    // NOTE - (i+j) must not be greater than m_blockCount
    for (uint32_t i = 0; i < m_blockCount - (blockNeeded - 1); i++) {
        for (uint32_t j = 0; j < blockNeeded; j++) {
            if (m_blockStates[i + j]) {
                i += j;
                goto endFirstFor;
            }
        }
        // space found
        memset(m_blockStates + i, 1, blockNeeded);
        uniform.frameCount = frameCount;
        uniform.size = sizeAligned;
        uniform.offset = i * m_blockSize;
        return true;

    endFirstFor:;
    }
    return false;
}

void VkUniformAllocator::free(VkMultiUniform& uniform) noexcept
{
    if (uniform.size == 0)
        return;
    uint32_t blockCount = uniform.size / m_blockSize * uniform.frameCount;
    uint32_t blockOffset = uniform.offset / m_blockSize;
    memset(m_blockStates + blockOffset, 0, blockCount);
}