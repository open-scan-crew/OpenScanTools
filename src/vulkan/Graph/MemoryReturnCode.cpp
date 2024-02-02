#include "MemoryReturnCode.h"
#include <QObject>

// Errors
#define TEXT_OBJECT_ALLOC_SUCCESS QObject::tr("Object importation successful.")
#define TEXT_OBJECT_ALLOC_ABORTED_BY_USER QObject::tr("Object importation aborted by the user.")
#define TEXT_OBJECT_ALLOC_FAILED QObject::tr("Failed to allocate object (unknow error).")
#define TEXT_LOAD_FILE_ERROR QObject::tr("Failed to load file.")
#define TEXT_OUT_OF_MEMORY QObject::tr("Not enough memory to create object.")
#define TEXT_INVALID_ADDRESS QObject::tr("Failed to create object (internal error).")


namespace ObjectAllocation
{
	static const std::unordered_map<ReturnCode, QString> ObjectCreationTextDictionnary = {
		{ ReturnCode::Success, TEXT_OBJECT_ALLOC_SUCCESS },
		{ ReturnCode::Failed, TEXT_OBJECT_ALLOC_FAILED },
		{ ReturnCode::Aborted, TEXT_OBJECT_ALLOC_ABORTED_BY_USER},
		{ ReturnCode::Load_File_Error, TEXT_LOAD_FILE_ERROR },
		{ ReturnCode::Out_Of_Memory, TEXT_OUT_OF_MEMORY },
		{ ReturnCode::Invalid_Address, TEXT_INVALID_ADDRESS }
	};

	static const std::unordered_map<VkResult, ReturnCode> VulkanDictionnary = {
		{ VkResult::VK_SUCCESS, ReturnCode::Success },
		{ VkResult::VK_ERROR_OUT_OF_HOST_MEMORY, ReturnCode::Out_Of_Memory },
		{ VkResult::VK_ERROR_OUT_OF_DEVICE_MEMORY, ReturnCode::Out_Of_Memory },
		{ VkResult::VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR, ReturnCode::Invalid_Address }
	};

	QString getText(ReturnCode code)
	{
		if (ObjectCreationTextDictionnary.find(code) != ObjectCreationTextDictionnary.end())
			return ObjectCreationTextDictionnary.at(code);
		else
			return TEXT_OBJECT_ALLOC_FAILED;
	}

	QString getVulkanText(VkResult vkRes)
	{
		return getText(getVulkanReturnCode(vkRes));
	}

	ReturnCode getVulkanReturnCode(VkResult vkRes)
	{
		if (VulkanDictionnary.find(vkRes) != VulkanDictionnary.end())
		{
			ReturnCode code = VulkanDictionnary.at(vkRes);
			return (code);
		}
		else
			return ReturnCode::Failed;
	}
}
