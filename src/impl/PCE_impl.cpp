#include "impl/PCE_impl.h"

#include "vulkan/VulkanManager.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "gui/Dialog/DialogDeviceSelection.h"
#include "gui/texts/VulkanTexts.hpp"
#include "utils/Config.h"

#include <QtWidgets/qmessagebox.h>

bool tl_pce_init(bool _enableValidationLayers, bool _select_device)
{
    const uint32_t extCount = 2;
    const char* extensions[extCount];
    extensions[0] = "VK_KHR_surface";
    extensions[1] = "VK_KHR_win32_surface";

    // Initialize VkInstance with layers and extensions required
	if (!VulkanManager::getInstance().initVkInstance(_enableValidationLayers, extCount, extensions))
	{
		QMessageBox::warning(nullptr, TEXT_VULKAN_ERROR, TEXT_VULKAN_FAILED_TO_INIT);
		return false;
	}

	// Choose one of the physical device available
	std::string preferedDevice = _select_device ? "" : Config::getGraphicDevice();
	std::unordered_map<uint32_t, std::string> deviceAvailable;
	bool found = VulkanManager::getInstance().initPhysicalDevice(preferedDevice, deviceAvailable);

	if (!found && deviceAvailable.size() >= 1)
	{
		DialogDeviceSelection modal(deviceAvailable, nullptr);
		uint32_t deviceIndex = modal.exec();
		preferedDevice = deviceAvailable.at(deviceIndex);
		deviceAvailable.clear();
		found = VulkanManager::getInstance().initPhysicalDevice(preferedDevice, deviceAvailable);
		if (found)
			Config::setGraphicDevice(preferedDevice);
	}
	if (!found)
	{
		QMessageBox::warning(nullptr, TEXT_VULKAN_ERROR, TEXT_VULKAN_NO_SUITABLE_DEVICE);
		return false;
	}

	if (!VulkanManager::getInstance().initResources())
	{
		QMessageBox::warning(nullptr, TEXT_VULKAN_ERROR, TEXT_VULKAN_NOT_ENOUGH_RESOURCES);
		return false;
	}

    TlScanOverseer::getInstance().init();

    VulkanManager::getInstance().initStreaming();

    return true;
}

void tl_pce_shutdown()
{
    VulkanManager::getInstance().stopStreaming();

    VulkanManager::getInstance().stopAllRendering();

    TlScanOverseer::getInstance().shutdown();
}