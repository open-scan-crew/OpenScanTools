#include "vulkan/ShaderCompiler.h"
#include "utils/Logger.h"

#define VKLOG Logger::log(LoggerMode::VKLog)

#include "glslang/glslang/Public/ShaderLang.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include <cctype>
#include <regex>
//#include "../../src/vulkan/shaders/shaders.hxx"
#define OUTPUT_FILE "../../include/shaders/generated/shaders.hxx"
#define OUTPUT_FILE2 "../../include/shaders/generated/shadersMap.hxx"

static std::vector<std::string> enumDef;

bool generateEnumDefAndMap()
{
	std::ofstream outFile;
	outFile.open(OUTPUT_FILE2, std::ios::binary);
	if (outFile.fail()) {
		std::cerr << "Shader: Failed to open file: " << OUTPUT_FILE2 << std::endl;
		return false;
	}

	outFile << "enum class EShader{";
	for (const std::string& def : enumDef)
		outFile << "\n\tE" << def << ",";
	outFile.seekp(-1, std::ios_base::end); // overwrite last coma
	outFile.write("\n};\n", 3);

	outFile << "static const std::map<EShader, const std::vector<unsigned int>*> ShaderMap = {";
	for (const std::string& def : enumDef)
		outFile << "\n\t{EShader::E" << def << ", &" << def << "},";
	outFile.seekp(-1, std::ios_base::end); // overwrite last coma
	outFile.write("\n};\n", 4);
	return true;
}

bool generateHXXShaderFile(std::vector<unsigned int>& spirv, const EShLanguage& type, const char* filename, const char* entryPoint)
{
	static bool firstWrite = true;
	std::ofstream outFile;
	if (firstWrite)
	{
		outFile.open(OUTPUT_FILE, std::ios::binary);
		firstWrite = false;
	}
	else
		outFile.open(OUTPUT_FILE, std::ios::binary | std::ofstream::app);
	if (outFile.fail()) {
		std::cerr << "Shader: Failed to open file: " << OUTPUT_FILE << std::endl;
		return false;
	}

	const char header[] = "#include <cstdint>\n\n";
	outFile.write(header, sizeof(header) - 1);

	std::string define("static const uint32_t ");
	std::string strFilename(filename);
	strFilename.erase(0, strFilename.find_last_of("/") + 1);
	strFilename.erase(strFilename.find("."), std::string::npos);
	std::string begin(entryPoint);
	begin.erase(0, 4);
	switch (type)
	{
	case EShLanguage::EShLangVertex:
		begin += "_VERTEX";
		break;
	case  EShLanguage::EShLangFragment:
		begin += "_FRAGMENT";
		break;
	case EShLanguage::EShLangGeometry:
		begin += "_GEOMETRY";
		break;
	}
	begin.insert(begin.begin(), strFilename.begin(), strFilename.end());
	std::transform(begin.begin(), begin.end(), begin.begin(), [](unsigned char c) { return std::toupper(c); });
	enumDef.push_back(begin);
	begin.insert(begin.begin(), define.begin(), define.end());
	begin += "[] =\n{";
	outFile.write(begin.c_str(), begin.size());
	int dataCount = 0;
	for (const unsigned int& data : spirv)
	{
		if (dataCount % 8 == 0)
			outFile << "\n\t";
		outFile << "0x" << std::hex << data;
		outFile << ",";
		dataCount++;
	}
	outFile.seekp(-1, std::ios_base::end);
	outFile.write("\n};\n\n", 5);
	outFile.close();
	return true;
}


static const TBuiltInResource Resources = {
	/* .MaxLights = */ 32,
	/* .MaxClipPlanes = */ 6,
	/* .MaxTextureUnits = */ 32,
	/* .MaxTextureCoords = */ 32,
	/* .MaxVertexAttribs = */ 64,
	/* .MaxVertexUniformComponents = */ 4096,
	/* .MaxVaryingFloats = */ 64,
	/* .MaxVertexTextureImageUnits = */ 32,
	/* .MaxCombinedTextureImageUnits = */ 80,
	/* .MaxTextureImageUnits = */ 32,
	/* .MaxFragmentUniformComponents = */ 4096,
	/* .MaxDrawBuffers = */ 32,
	/* .MaxVertexUniformVectors = */ 128,
	/* .MaxVaryingVectors = */ 8,
	/* .MaxFragmentUniformVectors = */ 16,
	/* .MaxVertexOutputVectors = */ 16,
	/* .MaxFragmentInputVectors = */ 15,
	/* .MinProgramTexelOffset = */ -8,
	/* .MaxProgramTexelOffset = */ 7,
	/* .MaxClipDistances = */ 8,
	/* .MaxComputeWorkGroupCountX = */ 65535,
	/* .MaxComputeWorkGroupCountY = */ 65535,
	/* .MaxComputeWorkGroupCountZ = */ 65535,
	/* .MaxComputeWorkGroupSizeX = */ 1024,
	/* .MaxComputeWorkGroupSizeY = */ 1024,
	/* .MaxComputeWorkGroupSizeZ = */ 64,
	/* .MaxComputeUniformComponents = */ 1024,
	/* .MaxComputeTextureImageUnits = */ 16,
	/* .MaxComputeImageUniforms = */ 8,
	/* .MaxComputeAtomicCounters = */ 8,
	/* .MaxComputeAtomicCounterBuffers = */ 1,
	/* .MaxVaryingComponents = */ 60,
	/* .MaxVertexOutputComponents = */ 64,
	/* .MaxGeometryInputComponents = */ 64,
	/* .MaxGeometryOutputComponents = */ 128,
	/* .MaxFragmentInputComponents = */ 128,
	/* .MaxImageUnits = */ 8,
	/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
	/* .MaxCombinedShaderOutputResources = */ 8,
	/* .MaxImageSamples = */ 0,
	/* .MaxVertexImageUniforms = */ 0,
	/* .MaxTessControlImageUniforms = */ 0,
	/* .MaxTessEvaluationImageUniforms = */ 0,
	/* .MaxGeometryImageUniforms = */ 0,
	/* .MaxFragmentImageUniforms = */ 8,
	/* .MaxCombinedImageUniforms = */ 8,
	/* .MaxGeometryTextureImageUnits = */ 16,
	/* .MaxGeometryOutputVertices = */ 256,
	/* .MaxGeometryTotalOutputComponents = */ 1024,
	/* .MaxGeometryUniformComponents = */ 1024,
	/* .MaxGeometryVaryingComponents = */ 64,
	/* .MaxTessControlInputComponents = */ 128,
	/* .MaxTessControlOutputComponents = */ 128,
	/* .MaxTessControlTextureImageUnits = */ 16,
	/* .MaxTessControlUniformComponents = */ 1024,
	/* .MaxTessControlTotalOutputComponents = */ 4096,
	/* .MaxTessEvaluationInputComponents = */ 128,
	/* .MaxTessEvaluationOutputComponents = */ 128,
	/* .MaxTessEvaluationTextureImageUnits = */ 16,
	/* .MaxTessEvaluationUniformComponents = */ 1024,
	/* .MaxTessPatchComponents = */ 120,
	/* .MaxPatchVertices = */ 32,
	/* .MaxTessGenLevel = */ 64,
	/* .MaxViewports = */ 16,
	/* .MaxVertexAtomicCounters = */ 0,
	/* .MaxTessControlAtomicCounters = */ 0,
	/* .MaxTessEvaluationAtomicCounters = */ 0,
	/* .MaxGeometryAtomicCounters = */ 0,
	/* .MaxFragmentAtomicCounters = */ 8,
	/* .MaxCombinedAtomicCounters = */ 8,
	/* .MaxAtomicCounterBindings = */ 1,
	/* .MaxVertexAtomicCounterBuffers = */ 0,
	/* .MaxTessControlAtomicCounterBuffers = */ 0,
	/* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
	/* .MaxGeometryAtomicCounterBuffers = */ 0,
	/* .MaxFragmentAtomicCounterBuffers = */ 1,
	/* .MaxCombinedAtomicCounterBuffers = */ 1,
	/* .MaxAtomicCounterBufferSize = */ 16384,
	/* .MaxTransformFeedbackBuffers = */ 4,
	/* .MaxTransformFeedbackInterleavedComponents = */ 64,
	/* .MaxCullDistances = */ 8,
	/* .MaxCombinedClipAndCullDistances = */ 8,
	/* .MaxSamples = */ 4,
	/* .maxMeshOutputVerticesNV = */ 256,
	/* .maxMeshOutputPrimitivesNV = */ 512,
	/* .maxMeshWorkGroupSizeX_NV = */ 32,
	/* .maxMeshWorkGroupSizeY_NV = */ 1,
	/* .maxMeshWorkGroupSizeZ_NV = */ 1,
	/* .maxTaskWorkGroupSizeX_NV = */ 32,
	/* .maxTaskWorkGroupSizeY_NV = */ 1,
	/* .maxTaskWorkGroupSizeZ_NV = */ 1,
	/* .maxMeshViewCountNV = */ 4,

	/* .limits = */ {
	/* .nonInductiveForLoops = */ 1,
	/* .whileLoops = */ 1,
	/* .doWhileLoops = */ 1,
	/* .generalUniformIndexing = */ 1,
	/* .generalAttributeMatrixVectorIndexing = */ 1,
	/* .generalVaryingIndexing = */ 1,
	/* .generalSamplerIndexing = */ 1,
	/* .generalVariableIndexing = */ 1,
	/* .generalConstantMatrixVectorIndexing = */ 1,
} };

char* ReadFileData(const char* fileName)
{
	FILE* in = nullptr;
	int errorCode = fopen_s(&in, fileName, "r");

	int count = 1;
	try
	{
		fgetc(in);
	}
	catch (...)
	{
		return nullptr;
	}
	while (fgetc(in) != EOF)
		count++;

	fseek(in, 0, SEEK_SET);

	char* return_data = new char[count + 1];  // freed in FreeFileData() 
	if ((int)fread(return_data, 1, count, in) != count) {
		free(return_data);
	}

	return_data[count] = '\0';
	fclose(in);

	return return_data;
}

bool ShaderCompiler::compileFromFile(const char* filepath, std::vector<unsigned int>& spirv, const char* entryPoint, const std::vector<std::string>& preProcessorDef)
{
	//std::vector<unsigned int> spirv;
	EShLanguage type;
	switch (filepath[strlen(filepath) - 4])
	{
	case 'v':
		type = EShLanguage::EShLangVertex;
		break;
	case 'f':
		type = EShLanguage::EShLangFragment;
		break;
	case 'g':
		type = EShLanguage::EShLangGeometry;
		break;
	default:
		type = EShLanguage::EShLangCompute;
		break;
	}

	glslang::InitializeProcess();
	char* source = ReadFileData(filepath);
	int sourceLength = (int)strlen(source);
	if (!source || !sourceLength)
		return false;
	EShMessages messages = (EShMessages)(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules | EShMsgCascadingErrors);
	glslang::TShader shader(type);
	shader.setStringsWithLengths(&source, &sourceLength, 1);
	shader.setAutoMapBindings(true);
	shader.setAutoMapLocations(true);
	shader.addProcesses(preProcessorDef);
	shader.setEntryPoint(entryPoint);
	shader.setSourceEntryPoint(entryPoint);
	spv::SpvBuildLogger logger;
	if (shader.parse(&Resources, 110, false, messages))
		glslang::GlslangToSpv(*shader.getIntermediate(), spirv, &logger);
	else
		std::cerr << filepath << "\n" << shader.getInfoLog() << shader.getInfoDebugLog() << std::endl;

	std::cout << filepath << "\n" << shader.getInfoLog() << shader.getInfoDebugLog() << "\n SPIRV LOG:\t" << logger.getAllMessages() << std::endl;
	delete source;
	generateHXXShaderFile(spirv, type, filepath, entryPoint);
	//generateEnumDefAndMap();

	return (!spirv.empty());
}