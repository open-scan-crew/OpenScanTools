#include "vulkan/Shader.h"
#include "vulkan/VulkanFunctions.h"

#include "utils/Logger.h"

#include <fstream>
#include <iostream>
#include <ios>

Shader::~Shader()
{
    reset();
}

/*
void Shader::load(VkDevice dev, VulkanDeviceFunctions* pfn, const EShader& shader)
{
	h_device = dev;
	h_pfn = pfn;

	if (ShaderMap.find(shader) == ShaderMap.end() || ShaderEntriesMap.find(shader) == ShaderEntriesMap.end())
	{
		std::cerr << "Shader: Failed to process file: " << magic_enum::enum_name(shader) << std::endl;
		return;
	}
	const std::vector<uint32_t>* spirv = ShaderMap.at(shader);
	VkShaderModuleCreateInfo shaderInfo = {};
	shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderInfo.codeSize = spirv->size() * sizeof(uint32_t);
	shaderInfo.pCode = spirv->data();
	VkResult err = h_pfn->vkCreateShaderModule(dev, &shaderInfo, nullptr, &m_shaderModule);
	if (err != VK_SUCCESS)
		std::cerr << "Failed to create shader module: " << magic_enum::enum_name(shader) << std::endl;
}*/

void Shader::load(VkDevice dev, VulkanDeviceFunctions* pfn, const char* filePath, const char* entryPoint, const std::vector<std::string>& preProcessorDef)
{
	uint64_t length = 0;
	char* pData = nullptr;

	std::ifstream inFile;
	inFile.open(filePath, std::ios::in | std::ios::binary | std::ios::ate);
	if (inFile.fail()) {
		std::cerr << "Shader: Failed to open file: " << filePath << std::endl;
		return;
	}

	length = inFile.tellg();
	pData = new char[length];

	inFile.seekg(0);
	inFile.read(pData, length);
	inFile.close();

	if (!length) {
		std::cerr << "Shader: Failed to process file: " << filePath << std::endl;
		if (pData)
			delete pData;
		return;
	}

	createModule(dev, pfn, reinterpret_cast<const uint32_t*>(pData), length);

	if(pData)
		 delete pData;
}

void Shader::createModule(VkDevice dev, VulkanDeviceFunctions* pfn, const uint32_t* binaryCode, size_t codeSize)
{
	h_device = dev;
	h_pfn = pfn;

	VkShaderModuleCreateInfo shaderInfo = {};
	shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderInfo.codeSize = codeSize;
	shaderInfo.pCode = binaryCode;

	if (m_shaderModule != VK_NULL_HANDLE)
	{
		VKLOG << "Warning: the shader module already exists. It will be overwritten." << Logger::endl;
		h_pfn->vkDestroyShaderModule(h_device, m_shaderModule, nullptr);
		m_shaderModule = VK_NULL_HANDLE;
	}

	VkResult err = h_pfn->vkCreateShaderModule(dev, &shaderInfo, nullptr, &m_shaderModule);
	if (err != VK_SUCCESS) {
		VKLOG << "ERROR: Failed to create shader module: " << err << Logger::endl;
		return;
	}
}

VkShaderModule Shader::module() const
{
    return m_shaderModule;
}

VkPipelineShaderStageCreateInfo Shader::getPipelineInfo(VkShaderStageFlagBits stage, const char* entrypoint)
{
    return {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        stage,
        m_shaderModule,
		entrypoint,
        nullptr
    };
}

void Shader::reset()
{
    if (m_shaderModule != VK_NULL_HANDLE) {
        h_pfn->vkDestroyShaderModule(h_device, m_shaderModule, nullptr);
		m_shaderModule = VK_NULL_HANDLE;
    }

    h_device = VK_NULL_HANDLE;
	h_pfn = nullptr;
}