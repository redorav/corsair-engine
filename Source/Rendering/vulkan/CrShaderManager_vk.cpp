#include "CrRendering_pch.h"

#include "CrShaderManager_vk.h"
#include "CrShaderReflection_vk.h"
#include "CrRenderDevice_vk.h"
#include "ShaderResources.h"
#include "CrShaderGen.h" // TODO remove
#include "CrVulkan.h"

#include "Core/Logging/ICrDebug.h"

#pragma warning (push, 0)
// Glslang
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>

// SPIRV-Cross
#include <spirv_cross.hpp>
#include <spirv_cpp.hpp>

#include <fstream>

#pragma warning (pop)

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

void CrShaderManagerVulkan::InitPS()
{

}

void CrShaderManagerVulkan::CompileStagePS(CrGraphicsShaderStageCreate& shaderStageInfo)
{
	glslang::InitializeProcess(); // TODO Put in Init() function

	int defaultVersion = 450;

	EShLanguage stage = EShLangCount;
	switch (shaderStageInfo.stage)
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

	std::string pathString = shaderStageInfo.path.string();
	const char* shaderSources[] = { shaderStageInfo.source.data() };
	const char* shaderNames[] = { pathString.data() };

	shader->setStringsWithLengthsAndNames(shaderSources, nullptr, shaderNames, 1);
	shader->setEntryPoint(shaderStageInfo.entryPoint.data());

	shader->setAutoMapBindings(true);
	//shader->setHlslIoMapping(true);

	shader->setShiftTextureBinding(10);
	shader->setShiftSamplerBinding(20);

	int ClientInputSemanticsVersion = 100;
	shader->setEnvInput(glslang::EShSourceHlsl,	stage, glslang::EShClientVulkan, ClientInputSemanticsVersion);

	shader->setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);

	shader->setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_0);

	//shader->preprocess(&s_resourceLimits, defaultVersion, )

	CrString extension = shaderStageInfo.path.extension().string().c_str();

	int msg = EShMsgSpvRules;

	if (extension.compare("hlsl") == 0)
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
	bool parsed = shader->parse(&s_resourceLimits, defaultVersion, ENoProfile, false, false, (EShMessages) msg, includer);
	if (!parsed)
	{
		CrLogError(shader->getInfoLog());
		CrLogError(shader->getInfoDebugLog());
	}

	glslang::TProgram& program = *new glslang::TProgram;

	program.addShader(shader);

	bool linked = program.link((EShMessages) msg);
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
	const unsigned char* bytes = reinterpret_cast<const unsigned char*>(spirvBytecode.data());
	shaderStageInfo.bytecode = CrVector<unsigned char>(bytes, bytes + spirvBytecode.size() * 4);

	glslang::FinalizeProcess();
}

void CrShaderManagerVulkan::CreateShaderResourceSetPS
(
	const CrGraphicsShaderCreate& shaderCreateInfo, 
	const CrShaderReflectionVulkan& reflection, 
	CrShaderResourceSet& resourceSet
)
{
	VkDevice vkDevice = static_cast<const CrRenderDeviceVulkan*>(m_renderDevice)->GetVkDevice();

	CrVector<VkDescriptorSetLayoutBinding> layoutBindings;

	for (const CrGraphicsShaderStageCreate& shaderStageCreate : shaderCreateInfo.GetStages())
	{
		cr3d::ShaderStage::T stage = shaderStageCreate.stage;

		for (cr3d::ShaderResourceType::T resourceType = cr3d::ShaderResourceType::Start; resourceType < cr3d::ShaderResourceType::Count; ++resourceType)
		{
			for (uint32_t i = 0; i < reflection.GetResourceCount(stage, resourceType); ++i)
			{
				CrShaderResource resource = reflection.GetResource(stage, resourceType, i);

				VkDescriptorSetLayoutBinding layoutBinding;
				layoutBinding.binding = resource.bindPoint;
				layoutBinding.descriptorType = crvk::GetVkDescriptorType(resourceType);
				layoutBinding.descriptorCount = 1; // TODO Get array size from reflection
				layoutBinding.stageFlags = crvk::GetVkShaderStage(stage);
				layoutBinding.pImmutableSamplers = nullptr;
				layoutBindings.push_back(layoutBinding);
			}
		}
	}

	VkDescriptorSetLayoutCreateInfo descriptorLayout;
	descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayout.pNext = nullptr;
	descriptorLayout.flags = 0;
	descriptorLayout.bindingCount = (uint32_t) layoutBindings.size();
	descriptorLayout.pBindings = layoutBindings.data();

	VkResult result = vkCreateDescriptorSetLayout(vkDevice, &descriptorLayout, nullptr, &resourceSet.m_vkDescriptorSetLayout);
	CrAssert(result == VK_SUCCESS);
}

VkShaderModule CrShaderManagerVulkan::CreateGraphicsShaderStagePS(const unsigned char* byteCode, size_t codeSize, cr3d::ShaderStage::T/* stage*/)
{
	CrAssert(byteCode != nullptr && codeSize > 0);

	VkDevice vkDevice = static_cast<const CrRenderDeviceVulkan*>(m_renderDevice)->GetVkDevice();

	VkShaderModule shaderModule;
	VkShaderModuleCreateInfo moduleCreateInfo;
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.pNext = nullptr;
	moduleCreateInfo.codeSize = codeSize;
	moduleCreateInfo.pCode = (uint32_t*)byteCode;
	moduleCreateInfo.flags = 0;

	VkResult result = vkCreateShaderModule(vkDevice, &moduleCreateInfo, nullptr, &shaderModule);

	CrAssertMsg(result == VK_SUCCESS, "Did not successfully create shader module!");

	return shaderModule;
}
