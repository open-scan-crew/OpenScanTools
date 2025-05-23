#ifndef VULKAN_PLATFORM_H_
#define VULKAN_PLATFORM_H_

class VulkanPlatformLib
{
public:
    VulkanPlatformLib();
    ~VulkanPlatformLib();

    PFN_vkVoidFunction resolveFunction(const char* pName);

private:
    void* m_hDLL = nullptr;
};

#endif