#ifndef SHADER_COMPILER_H_
#define SHADER_COMPILER_H_

#include "vulkan/vulkan.h"
#include <vector>

class ShaderCompiler
{
public:
	//void load(VkDevice dev, VulkanDeviceFunctions* pfn, const EShader& shader);
	static bool compileFromFile(const char* path, std::vector<unsigned int>& spriv, const char* entryPoint, const std::vector<std::string>& preProcessorDef = {});
};

#endif