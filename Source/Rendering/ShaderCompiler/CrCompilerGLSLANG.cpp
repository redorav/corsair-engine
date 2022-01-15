//#include <fstream>
//#include <sstream>

#include "CrShaderCompiler.h"
#include "CrShaderCompilerUtilities.h"
#include "CrCompilerGLSLANG.h"

#include "Rendering/CrRendering.h"

#pragma warning (push, 0)
// Glslang
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#pragma warning (pop)

#include "Core/FileSystem/CrPath.h"
#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/FileSystem/ICrFile.h"

static const TBuiltInResource s_resourceLimits =
{
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
    /* .maxDualSourceDrawBuffersEXT = */ 1,

    /* .limits = */
	{
        /* .nonInductiveForLoops = */ 1,
        /* .whileLoops = */ 1,
        /* .doWhileLoops = */ 1,
        /* .generalUniformIndexing = */ 1,
        /* .generalAttributeMatrixVectorIndexing = */ 1,
        /* .generalVaryingIndexing = */ 1,
        /* .generalSamplerIndexing = */ 1,
        /* .generalVariableIndexing = */ 1,
        /* .generalConstantMatrixVectorIndexing = */ 1,
    }
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
		CrPath headerPath = includerDirectory / headerName;

		CrFileSharedHandle file = ICrFile::OpenFile(headerPath.c_str(), FileOpenFlags::Read);

		if (file)
		{
			size_t headerSize = file->GetSize();
			char* headerData = new char[headerSize];
			file->Read(headerData, headerSize);
			file = nullptr;
			IncludeResult* includeResult = new IncludeResult(headerPath.c_str(), headerData, headerSize, nullptr);
			return includeResult;
		}
		else
		{
			CrString errorMessage;
			errorMessage.append_sprintf("Include %s not found\n", headerPath.c_str());
			CrShaderCompilerUtilities::QuitWithMessage(errorMessage);
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

bool CrCompilerGLSLANG::HLSLtoSPIRV(const CompilationDescriptor& compilationDescriptor, std::vector<uint32_t>& spirvBytecode)
{
	CrFileSharedHandle file = ICrFile::OpenFile(compilationDescriptor.inputPath.c_str(), FileOpenFlags::Read);

	if (!file)
	{
		return false;
	}

	std::vector<char> shaderSource;
	shaderSource.resize(file->GetSize());
	file->Read(shaderSource.data(), shaderSource.size());
	shaderSource.push_back(0);

	int defaultVersion = 450;

	EShLanguage stage = EShLangCount;
	switch (compilationDescriptor.shaderStage)
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
	const char* shaderNames[] = { compilationDescriptor.inputPath.c_str() };

	shader->setStringsWithLengthsAndNames(shaderSources, nullptr, shaderNames, 1);
	shader->setEntryPoint(compilationDescriptor.entryPoint.c_str());

	shader->setAutoMapBindings(true);
	//shader->setHlslIoMapping(true);

	// TODO This needs revisiting
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
		const char* infoLog = shader->getInfoLog();
		const char* infoDebugLog = shader->getInfoDebugLog();
		printf("%s", infoLog);
		printf("%s", infoDebugLog);
		//CrLogError(infoLog);
		//CrLogError(infoDebugLog);
		return false;
	}

	bool ioMapped = program.mapIO();
	if (!ioMapped)
	{
		const char* infoLog = shader->getInfoLog();
		const char* infoDebugLog = shader->getInfoDebugLog();
		printf("%s", infoLog);
		printf("%s", infoDebugLog);
		//CrLogError(shader->getInfoLog());
		//CrLogError(shader->getInfoDebugLog());
		return false;
	}

	const glslang::TIntermediate* intermediate = program.getIntermediate(stage);

	// Generate the SPIR-V bytecode
	spv::SpvBuildLogger logger;
	glslang::GlslangToSpv(*intermediate, spirvBytecode, &logger);

	static bool readableSpirv = false;
	if (readableSpirv) // Optionally disassemble into human-readable format
	{
		
	}

	return true;
}