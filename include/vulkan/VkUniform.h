#ifndef VK_UNIFORM_H
#define VK_UNIFORM_H

// \brief
// Manage the Uniform descriptor in Vulkan
//  - Allocate the uniform
//  - Multibuffer the uniform

#include <cstdint>

typedef uint32_t VkUniformOffset;

struct VkMultiUniform
{
    uint32_t frameCount = 0;
    uint32_t size = 0;
    uint32_t offset = 0;

    VkUniformOffset operator[](uint32_t frameIndex) const
    {
        return (offset + size * frameIndex);
    }
};

#endif