#include "vulkan/vk_platform.h"
#include "utils/Logger.h"

// redefine the vulkan void function pointer without including all "vulkan_core.h"
typedef void (VKAPI_PTR *PFN_vkVoidFunction)(void);

#include "vulkan/VulkanPlatform.h"

#if defined(_WIN32)
#include <windows.h>
#include <libloaderapi.h>

VulkanPlatformLib::VulkanPlatformLib()
{
    m_hDLL = LoadLibrary("vulkan-1.dll");

    if (m_hDLL == NULL)
    {
        Logger::log(LoggerMode::VKLog) << "ERROR - vulkan-1.dll is missing." << Logger::endl;
        MessageBox(NULL, "Error: vulkan-1.dll is missing.", "DLL not found!", MB_OK | MB_ICONERROR);
        exit(1);
    }
}

VulkanPlatformLib::~VulkanPlatformLib()
{
    FreeLibrary((HINSTANCE)m_hDLL);
}

PFN_vkVoidFunction VulkanPlatformLib::resolveFunction(const char* pName)
{
    return (PFN_vkVoidFunction)GetProcAddress((HINSTANCE)m_hDLL, pName);
}

#else

// TODO - other platforms if needed
#endif