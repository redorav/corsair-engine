#include "CrRendering_pch.h"

#include "Rendering/ICrRenderDevice.h"
#include "ICrShaderManager.h"
#include "ICrShaderReflection.h"
#include "ICrShader.h"
#include "CrResourceManager.h"
#include "ShaderResources.h"

#include "Core/CrMacros.h"
#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/FileSystem/ICrFile.h"
#include "Core/Logging/ICrDebug.h"

// TODO Delete
#include "vulkan/CrShaderReflection_vk.h"
#include "vulkan/CrShaderManager_vk.h"

// TODO Delete once we call CrShaderCompiler
#pragma warning (push, 0)
// Glslang
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>

// SPIRV-Cross
#include <spirv_cross.hpp>
#include <spirv_cpp.hpp>

#include <fstream>

#pragma warning (pop)

static CrShaderManagerVulkan g_shaderManager;

CrShaderResource CrShaderResource::Invalid = {};

ICrShaderManager* ICrShaderManager::Get()
{
	return &g_shaderManager;
}

CrShaderBytecodeSharedHandle ICrShaderManager::LoadShaderBytecode(const CrPath& path, const CrShaderBytecodeDescriptor& bytecodeDescriptor) const
{
	CrFileSharedHandle file = ICrFile::Create(path, FileOpenFlags::Read);
	return LoadShaderBytecode(file, bytecodeDescriptor);
}

CrShaderBytecodeSharedHandle ICrShaderManager::LoadShaderBytecode(const CrFileSharedHandle& file, const CrShaderBytecodeDescriptor& bytecodeDescriptor) const
{
	switch (bytecodeDescriptor.format)
	{
		case cr3d::ShaderCodeFormat::Binary:
		{
			CrShaderBytecodeSharedHandle bytecode;
			{
				CrVector<unsigned char> bytecodeData;
				bytecodeData.resize(file->GetSize());
				file->Read(bytecodeData.data(), bytecodeData.size());
				bytecode = CrShaderBytecodeSharedHandle(new CrShaderBytecode(std::move(bytecodeData), bytecodeDescriptor.entryPoint, bytecodeDescriptor.stage));
			}
			return bytecode;
		}
		case cr3d::ShaderCodeFormat::SourceHLSL:
		{
			return CompileShaderBytecode(file, bytecodeDescriptor);
		}
	}

	return nullptr;
}

CrGraphicsShaderHandle ICrShaderManager::LoadGraphicsShader(const CrGraphicsShaderCreate& shaderCreateInfo) const
{
	CrGraphicsShaderHandle graphicsShader(new ICrGraphicsShader());
	CrShaderReflectionVulkan reflection; // TODO Remove this

	graphicsShader->m_bytecodes.reserve(shaderCreateInfo.GetBytecodeDescriptors().size());

	for (const CrShaderBytecodeDescriptor& bytecodeDescriptor : shaderCreateInfo.GetBytecodeDescriptors())
	{
		CrShaderBytecodeSharedHandle bytecode = LoadShaderBytecode(bytecodeDescriptor.path, bytecodeDescriptor);

		// Compute hashes based on bytecode
		//CrHash bytecodeHash = CrHash(stageCreate.bytecode.data(), stageCreate.bytecode.size());
		//graphicsShader->m_hash <<= bytecodeHash;

		graphicsShader->m_bytecodes.push_back(bytecode);

		// 4. Add to the reflection structure (we'll build the necessary resource tables using this later)
		reflection.AddBytecode(bytecode);
	}

	CreateShaderResourceSet(shaderCreateInfo, reflection, graphicsShader->m_resourceSet);

	return graphicsShader;
}

ConstantBufferMetadata& ICrShaderManager::GetConstantBufferMetadata(const CrString& name)
{
	auto cBuffer = ConstantBufferTable.find(name);

	if (cBuffer != ConstantBufferTable.end())
	{
		return (*cBuffer).second;
	}

	return InvalidConstantBufferMetaInstance;
}

ConstantBufferMetadata& ICrShaderManager::GetConstantBufferMetadata(ConstantBuffers::T id)
{
	return ConstantBufferMetaTable[id];
}

TextureMetadata& ICrShaderManager::GetTextureMetadata(const CrString& name)
{
	auto textureMetadata = TextureTable.find(name);

	if (textureMetadata != TextureTable.end())
	{
		return (*textureMetadata).second;
	}

	return InvalidTextureMetaInstance;
}

TextureMetadata& ICrShaderManager::GetTextureMetadata(Textures::T id)
{
	return TextureMetaTable[id];
}

SamplerMetadata& ICrShaderManager::GetSamplerMetadata(const CrString& name)
{
	auto samplerMetadata = SamplerTable.find(name);

	if (samplerMetadata != SamplerTable.end())
	{
		return (*samplerMetadata).second;
	}

	return InvalidSamplerMetaInstance;
}

SamplerMetadata& ICrShaderManager::GetSamplerMetadata(Samplers::T id)
{
	return SamplerMetaTable[id];
}

void ICrShaderManager::CreateShaderResourceSet(const CrGraphicsShaderCreate& shaderCreateInfo, const CrShaderReflectionVulkan& reflection, CrShaderResourceSet& resourceSet) const
{
	for (const CrShaderBytecodeDescriptor& bytecodeDescriptor : shaderCreateInfo.GetBytecodeDescriptors())
	{
		cr3d::ShaderStage::T stage = bytecodeDescriptor.stage;

		uint32_t numConstantBuffers = reflection.GetResourceCount(stage, cr3d::ShaderResourceType::ConstantBuffer);

		for (uint32_t i = 0; i < numConstantBuffers; ++i)
		{
			CrShaderResource res = reflection.GetResource(stage, cr3d::ShaderResourceType::ConstantBuffer, i);
			ConstantBufferMetadata& metadata = GetConstantBufferMetadata(res.name);
			resourceSet.AddConstantBuffer(stage, metadata.id, res.bindPoint);
		}

		uint32_t numTextures = reflection.GetResourceCount(stage, cr3d::ShaderResourceType::Texture);

		for (uint32_t i = 0; i < numTextures; ++i)
		{
			CrShaderResource res = reflection.GetResource(stage, cr3d::ShaderResourceType::Texture, i);
			TextureMetadata& metadata = GetTextureMetadata(res.name);
			resourceSet.AddTexture(stage, metadata.id, res.bindPoint);
		}

		uint32_t numSamplers = reflection.GetResourceCount(stage, cr3d::ShaderResourceType::Sampler);

		for (uint32_t i = 0; i < numSamplers; ++i)
		{
			CrShaderResource res = reflection.GetResource(stage, cr3d::ShaderResourceType::Sampler, i);
			SamplerMetadata& metadata = GetSamplerMetadata(res.name);
			resourceSet.AddSampler(stage, metadata.id, res.bindPoint);
		}
	}

	CreateShaderResourceSetPS(shaderCreateInfo, reflection, resourceSet);
}

void ICrShaderManager::Init(const ICrRenderDevice* renderDevice)
{
	m_renderDevice = renderDevice;
	InitPS();
}

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

CrShaderBytecodeSharedHandle ICrShaderManager::CompileShaderBytecode(const CrFileSharedHandle& file, const CrShaderBytecodeDescriptor& bytecodeDescriptor) const
{
	glslang::InitializeProcess(); // TODO Put in Init() function

	int defaultVersion = 450;

	EShLanguage stage = EShLangCount;
	switch (bytecodeDescriptor.stage)
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
	}

	CrAssert(stage != EShLangCount);

	glslang::TShader* shader = new glslang::TShader(stage);

	CrString shaderSource;
	shaderSource.resize(file->GetSize());
	file->Read(shaderSource.data(), shaderSource.size());

	std::string pathString = file->GetFilePath();
	const char* shaderSources[] = { shaderSource.data() };
	const char* shaderNames[] = { pathString.data() };

	shader->setStringsWithLengthsAndNames(shaderSources, nullptr, shaderNames, 1);
	shader->setEntryPoint(bytecodeDescriptor.entryPoint.data());

	shader->setAutoMapBindings(true);
	//shader->setHlslIoMapping(true);

	shader->setShiftTextureBinding(10);
	shader->setShiftSamplerBinding(20);

	int ClientInputSemanticsVersion = 100;
	shader->setEnvInput(glslang::EShSourceHlsl, stage, glslang::EShClientVulkan, ClientInputSemanticsVersion);

	shader->setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);

	shader->setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_0);

	//shader->preprocess(&s_resourceLimits, defaultVersion, )

	int msg = EShMsgSpvRules;

	if (bytecodeDescriptor.format == cr3d::ShaderCodeFormat::SourceHLSL)
	{
		msg |= EShMsgReadHlsl;
		msg |= EShMsgHlslOffsets;
	}

	class HeaderIncluder : public glslang::TShader::Includer
	{
	public:

		// Find in system paths
		virtual IncludeResult* includeSystem(const char* /*headerName*/, const char* /*includerName*/, size_t /*inclusionDepth*/) override
		{
			return nullptr;
		}

		// Find in local paths
		virtual IncludeResult* includeLocal(const char* headerName, const char* includerName, size_t /*inclusionDepth*/) override
		{
			std::string includerPath = includerName;
			includerPath = includerPath.substr(0, includerPath.find_last_of("/"));

			std::string headerPath = includerPath + "/" + headerName;

			std::replace(headerPath.begin(), headerPath.end(), '\\', '/');
			std::ifstream file(headerPath, std::ios_base::binary | std::ios_base::ate);
			if (file)
			{
				int length = (int)file.tellg();
				char* content = new char[length];
				file.seekg(0, file.beg);
				file.read(content, length);
				return new IncludeResult(headerPath, content, length, content);
			}

			return nullptr;
		}

		// Signals that the parser will no longer use the contents of the
		// specified IncludeResult.
		virtual void releaseInclude(IncludeResult* includeResult) override
		{
			if (includeResult != nullptr)
			{
				delete[] static_cast<char*>(includeResult->userData);
				delete includeResult;
			}
		}

		virtual ~HeaderIncluder() override
		{

		}
	};

	HeaderIncluder includer;
	bool parsed = shader->parse(&s_resourceLimits, defaultVersion, ENoProfile, false, false, (EShMessages)msg, includer);
	if (!parsed)
	{
		CrLogError(shader->getInfoLog());
		CrLogError(shader->getInfoDebugLog());
	}

	glslang::TProgram& program = *new glslang::TProgram;

	program.addShader(shader);

	bool linked = program.link((EShMessages)msg);
	if (!linked)
	{
		CrLogError(shader->getInfoLog());
		CrLogError(shader->getInfoDebugLog());
	}

	bool ioMapped = program.mapIO();
	if (!ioMapped)
	{
		CrLogError(shader->getInfoLog());
		CrLogError(shader->getInfoDebugLog());
	}

	// Generate the SPIR-V bytecode
	std::vector<uint32_t> spirvBytecode;
	spv::SpvBuildLogger logger;

	glslang::GlslangToSpv(*program.getIntermediate(stage), spirvBytecode, &logger);

	CrAssert(spirvBytecode.size() > 0);

	static bool readableSpirv = false;
	if (readableSpirv) // Optionally disassemble into human-readable format
	{
		//std::ostringstream outstr;
		//spv::Disassemble(outstr, spirvBytecode);
		//std::string spirvString = outstr.str();
	}

	// TODO Optimize SPIR-V bytecode

	// Copy shader bytecode
	//const unsigned char* bytes = reinterpret_cast<const unsigned char*>(spirvBytecode.data());
	//shaderStageInfo.bytecode = CrVector<unsigned char>(bytes, bytes + spirvBytecode.size() * 4);

	glslang::FinalizeProcess();

	const unsigned char* bytes = reinterpret_cast<const unsigned char*>(spirvBytecode.data());
	CrVector<unsigned char> spirvBytecodeBytes;
	spirvBytecodeBytes = CrVector<unsigned char>(bytes, bytes + spirvBytecode.size() * 4);

	CrShaderBytecodeSharedHandle bytecode = CrShaderBytecodeSharedHandle(new CrShaderBytecode
	(
		std::move(spirvBytecodeBytes),
		bytecodeDescriptor.entryPoint,
		bytecodeDescriptor.stage
	));

	return bytecode;
}