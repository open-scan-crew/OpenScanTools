#ifndef IMAGE_TRANSFER_EVENT_H
#define IMAGE_TRANSFER_EVENT_H

#include "vulkan/vulkan.h"

struct ImageTransferEvent
{
    VkImage image;
    VkDeviceMemory memory;
    VkEvent transferEvent;
    uint32_t width;
    uint32_t height;
    VkFormat format = VK_FORMAT_UNDEFINED;
};

#endif