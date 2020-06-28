#include <fstream>
#include <sstream>

#include "CrSPIRVCompiler.h"

#include "Rendering/CrRendering.h"

#pragma warning (push, 0)
// Glslang
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>

// SPIR-V
#include <spirv_cross.hpp>
#include <spirv_cpp.hpp>
#pragma warning (pop)

#include "Core/FileSystem/CrFileSystem.h"

static const TBuiltInResource s_resourceLimits =
{
	32,    // MaxLights
	6,     // MaxClipPlanes
	32,    // MaxTextureUnits
	32,    // MaxTextureCoords
	64,    // MaxVertexAttribs
	4096,  // MaxVertexUniformComponents
	64,    // MaxVaryingFloats
	32,    // MaxVertexTextureImageUnits
	80,    // MaxCombinedTextureImageUnits
	32,    // MaxTextureImageUnits
	4096,  // MaxFragmentUniformComponents
	32,    // MaxDrawBuffers
	128,   // MaxVertexUniformVectors
	8,     // MaxVaryingVectors
	16,    // MaxFragmentUniformVectors
	16,    // MaxVertexOutputVectors
	15,    // MaxFragmentInputVectors
	-8,    // MinProgramTexelOffset
	7,     // MaxProgramTexelOffset
	8,     // MaxClipDistances
	65535, // MaxComputeWorkGroupCountX
	65535, // MaxComputeWorkGroupCountY
	65535, // MaxComputeWorkGroupCountZ
	1024,  // MaxComputeWorkGroupSizeX
	1024,  // MaxComputeWorkGroupSizeY
	64,    // MaxComputeWorkGroupSizeZ
	1024,  // MaxComputeUniformComponents
	16,    // MaxComputeTextureImageUnits
	8,     // MaxComputeImageUniforms
	8,     // MaxComputeAtomicCounters
	1,     // MaxComputeAtomicCounterBuffers
	60,    // MaxVaryingComponents
	64,    // MaxVertexOutputComponents
	64,    // MaxGeometryInputComponents
	128,   // MaxGeometryOutputComponents
	128,   // MaxFragmentInputComponents
	8,     // MaxImageUnits
	8,     // MaxCombinedImageUnitsAndFragmentOutputs
	8,     // MaxCombinedShaderOutputResources
	0,     // MaxImageSamples
	0,     // MaxVertexImageUniforms
	0,     // MaxTessControlImageUniforms
	0,     // MaxTessEvaluationImageUniforms
	0,     // MaxGeometryImageUniforms
	8,     // MaxFragmentImageUniforms
	8,     // MaxCombinedImageUniforms
	16,    // MaxGeometryTextureImageUnits
	256,   // MaxGeometryOutputVertices
	1024,  // MaxGeometryTotalOutputComponents
	1024,  // MaxGeometryUniformComponents
	64,    // MaxGeometryVaryingComponents
	128,   // MaxTessControlInputComponents
	128,   // MaxTessControlOutputComponents
	16,    // MaxTessControlTextureImageUnits
	1024,  // MaxTessControlUniformComponents
	4096,  // MaxTessControlTotalOutputComponents
	128,   // MaxTessEvaluationInputComponents
	128,   // MaxTessEvaluationOutputComponents
	16,    // MaxTessEvaluationTextureImageUnits
	1024,  // MaxTessEvaluationUniformComponents
	120,   // MaxTessPatchComponents
	32,    // MaxPatchVertices
	64,    // MaxTessGenLevel
	16,    // MaxViewports
	0,     // MaxVertexAtomicCounters
	0,     // MaxTessControlAtomicCounters
	0,     // MaxTessEvaluationAtomicCounters
	0,     // MaxGeometryAtomicCounters
	8,     // MaxFragmentAtomicCounters
	8,     // MaxCombinedAtomicCounters
	1,     // MaxAtomicCounterBindings
	0,     // MaxVertexAtomicCounterBuffers
	0,     // MaxTessControlAtomicCounterBuffers
	0,     // MaxTessEvaluationAtomicCounterBuffers
	0,     // MaxGeometryAtomicCounterBuffers
	1,     // MaxFragmentAtomicCounterBuffers
	1,     // MaxCombinedAtomicCounterBuffers
	16384, // MaxAtomicCounterBufferSize
	4,     // MaxTransformFeedbackBuffers
	64,    // MaxTransformFeedbackInterleavedComponents
	8,     // MaxCullDistances
	8,     // MaxCombinedClipAndCullDistances
	4,     // MaxSamples
	0,     // maxMeshOutputVerticesNV;
	0,     // maxMeshOutputPrimitivesNV;
	0,     // maxMeshWorkGroupSizeX_NV;
	0,     // maxMeshWorkGroupSizeY_NV;
	0,     // maxMeshWorkGroupSizeZ_NV;
	0,     // maxTaskWorkGroupSizeX_NV;
	0,     // maxTaskWorkGroupSizeY_NV;
	0,     // maxTaskWorkGroupSizeZ_NV;
	0,     // maxMeshViewCountNV

	{ // limits
		true, // nonInductiveForLoops
		true, // whileLoops
		true, // doWhileLoops
		true, // generalUniformIndexing
		true, // generalAttributeMatrixVectorIndexing
		true, // generalVaryingIndexing
		true, // generalSamplerIndexing
		true, // generalVariableIndexing
		true, // generalConstantMatrixVectorIndexing
	},
};

class BasicIncluder : public glslang::TShader::Includer
{
public:
	// For the "system" or <>-style includes; search the "system" paths.
	virtual IncludeResult* includeSystem(const char* /*headerName*/, const char* /*includerName*/, size_t /*inclusionDepth*/) override
	{
		return nullptr;
	}

	// For the "local"-only aspect of a "" include. Should not search in the "system" paths, because on returning a failure, the parser will
	// call includeSystem() to look in the "system" locations.
	virtual IncludeResult* includeLocal(const char* headerName, const char* includerName, size_t /*inclusionDepth*/) override
	{
		// We assume the file is in the same directory as us. If not, the include should have the required folder
		CrPath includerPath = includerName;
		CrPath includerDirectory = includerPath.parent_path();
		CrPath headerPath = includerDirectory /= headerName;

		std::ifstream fileStream(headerPath.string(), std::ios::binary);

		if (fileStream.is_open())
		{
			fileStream.seekg(0, std::ios::end);
			size_t headerSize = fileStream.tellg();
			char* headerData = new char[headerSize];
			//headerData.resize(fileStream.tellg());
			fileStream.seekg(0, std::ios::beg);
			fileStream.read(&headerData[0], headerSize);
			fileStream.close();

			IncludeResult* includeResult = new IncludeResult(headerPath.string(), headerData, headerSize, nullptr);
			return includeResult;
		}
		else
		{
			return nullptr;
		}
	}

	// Signals that the parser will no longer use the contents of the specified IncludeResult.
	virtual void releaseInclude(IncludeResult* includeResult) override
	{
		delete[] includeResult->headerData;
		delete includeResult;
	}

	virtual ~BasicIncluder() override
	{

	}
};

bool CrSPIRVCompiler::HLSLtoSPIRV(const std::string& shaderPath, const std::string& entryPoint, const cr3d::ShaderStage::T shaderStage, std::vector<uint32_t>& spirvBytecode)
{
	// TODO Move this higher up. We don't want file loading logic here
	std::ifstream fileStream;
	fileStream.open(shaderPath, std::ios::binary);

	if (!fileStream.is_open())
	{
		return false;
	}

	std::vector<char> shaderSource;
	fileStream.seekg(0, std::ios::end);
	shaderSource.resize(fileStream.tellg());
	fileStream.seekg(0, std::ios::beg);
	fileStream.read(&shaderSource[0], shaderSource.size());
	fileStream.close();
	shaderSource.push_back(0);

	int defaultVersion = 450;

	EShLanguage stage = EShLangCount;
	switch (shaderStage)
	{
		case cr3d::ShaderStage::Vertex:
			stage = EShLangVertex;
			break;
		case cr3d::ShaderStage::Geometry:
			stage = EShLangGeometry;
			break;
		case cr3d::ShaderStage::Hull:
			stage = EShLangTessControl;
			break;
		case cr3d::ShaderStage::Domain:
			stage = EShLangTessEvaluation;
			break;
		case cr3d::ShaderStage::Pixel:
			stage = EShLangFragment;
			break;
		case cr3d::ShaderStage::Compute:
			stage = EShLangCompute;
			break;
		default:
			stage = EShLangCount;
			break;
	}

	if (stage == EShLangCount)
	{
		return false;
	}

	glslang::TShader* shader = new glslang::TShader(stage);

	const char* shaderSources[] = { shaderSource.data() };
	const char* shaderNames[] = { shaderPath.data() };

	shader->setStringsWithLengthsAndNames(shaderSources, nullptr, shaderNames, 1);
	shader->setEntryPoint(entryPoint.c_str());

	shader->setAutoMapBindings(true);

	shader->setShiftTextureBinding(10);
	shader->setShiftSamplerBinding(20);

	int clientInputSemanticsVersion = 100;
	shader->setEnvInput(glslang::EShSourceHlsl, stage, glslang::EShClientVulkan, clientInputSemanticsVersion);
	
	shader->setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);

	// Specify SPIR-V version for the module
	shader->setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_0);

	//shader->preprocess(&s_resourceLimits, defaultVersion, )

	int msg = EShMsgSpvRules;
	msg |= EShMsgReadHlsl;
	msg |= EShMsgHlslOffsets;

	BasicIncluder includer;
	bool parsed = shader->parse(&s_resourceLimits, defaultVersion, ENoProfile, false, false, (EShMessages)msg, includer);
	if (!parsed)
	{
		const char* infoLog = shader->getInfoLog();
		const char* infoDebugLog = shader->getInfoDebugLog();
		printf("%s", infoLog);
		printf("%s", infoDebugLog);
		//CrLogError(infoLog);
		//CrLogError(infoDebugLog);
		return false;
	}

	glslang::TProgram& program = *new glslang::TProgram;

	program.addShader(shader);

	bool linked = program.link((EShMessages)msg);
	if (!linked)
	{
		//const char* infoLog = shader->getInfoLog();
		//const char* infoDebugLog = shader->getInfoDebugLog();
		//CrLogError(infoLog);
		//CrLogError(infoDebugLog);
		return false;
	}

	//bool ioMapped = program.mapIO();
	//if (!ioMapped)
	//{
	//	//CrLogError(shader->getInfoLog());
	//	//CrLogError(shader->getInfoDebugLog());
	//	return false;
	//}

	// Generate the SPIR-V bytecode
	spv::SpvBuildLogger logger;

	glslang::GlslangToSpv(*program.getIntermediate(stage), spirvBytecode, &logger);

	static bool readableSpirv = true;
	if (readableSpirv) // Optionally disassemble into human-readable format
	{
		std::ostringstream outstr;
		//spv::Disassemble(outstr, spirvBytecode);
		std::string spirvString = outstr.str();
	}

	return true;
}