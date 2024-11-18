#ifndef MEMORY_RETURN_CODE_H
#define MEMORY_RETURN_CODE_H

#include <unordered_map>
#include "vulkan/vulkan.h"
#include <QtCore/qstring.h>

namespace ObjectAllocation
{
	enum class ReturnCode {
		Success,
		Failed,
		Aborted,
		Load_File_Error,
		Out_Of_Memory,
		Invalid_Address,
		MeshAllocFail
	};

	QString getText(ReturnCode code);
	QString getVulkanText(VkResult vkRes);
	ReturnCode getVulkanReturnCode(VkResult vkRes);
}

#endif //! MEMORY_RETURN_CODE_H_
