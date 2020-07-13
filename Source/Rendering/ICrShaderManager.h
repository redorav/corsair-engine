#pragma once

#include "ICrShaderReflection.h"

#include "Core/CrCoreForwardDeclarations.h"

class ICrRenderDevice;
struct CrGraphicsShaderCreate;
class CrShaderReflectionVulkan;
class CrShaderResourceSet;
struct ConstantBufferMetadata;
struct TextureMetadata;
struct SamplerMetadata;

class CrGraphicsShader;
using CrGraphicsShaderHandle = CrSharedPtr<CrGraphicsShader>;
struct CrGraphicsShaderCreate;
struct CrGraphicsShaderStageCreate;

// TODO Delete all this
struct VkShaderModule_T;
typedef struct VkShaderModule_T* VkShaderModule;

class CrShaderManagerVulkan;

namespace ConstantBuffers { enum T : uint8_t; }
namespace Textures { enum T : uint8_t; }
namespace Samplers { enum T : uint8_t; }

class ICrShaderManager
{
public:

	static ICrShaderManager* Get();

	void Init(const ICrRenderDevice* renderDevice);

	CrGraphicsShaderHandle LoadGraphicsShader(CrGraphicsShaderCreate& shaderCreateInfo);

	// Loads shader bytecode from disk based on the information provided by shaderStage
	void LoadShaderBytecode(CrGraphicsShaderStageCreate& shaderStage);

	static ConstantBufferMetadata& GetConstantBufferMetadata(const CrString& name);

	static ConstantBufferMetadata& GetConstantBufferMetadata(ConstantBuffers::T id);

	static TextureMetadata& GetTextureMetadata(const CrString& name);

	static TextureMetadata& GetTextureMetadata(Textures::T id);

	static SamplerMetadata& GetSamplerMetadata(const CrString& name);

	static SamplerMetadata& GetSamplerMetadata(Samplers::T id);

protected:

	void CompileStage(CrGraphicsShaderStageCreate& shaderStageInfo);

	VkShaderModule CreateGraphicsShaderStage(const unsigned char* byteCode, size_t codeSize, cr3d::ShaderStage::T stage);

	// TODO Remove this reference to shader reflection vulkan
	void CreateShaderResourceSet(const CrGraphicsShaderCreate& shaderCreateInfo, const CrShaderReflectionVulkan& reflection, CrShaderResourceSet& resourceSet);

	virtual void InitPS() = 0;

	virtual void CompileStagePS(CrGraphicsShaderStageCreate& shaderStageInfo) = 0;

	virtual VkShaderModule CreateGraphicsShaderStagePS(const unsigned char* byteCode, size_t codeSize, cr3d::ShaderStage::T stage) = 0;

	virtual void CreateShaderResourceSetPS(const CrGraphicsShaderCreate& shaderCreateInfo, const CrShaderReflectionVulkan& reflection, CrShaderResourceSet& resourceSet) = 0;

	const ICrRenderDevice* m_renderDevice;
};
