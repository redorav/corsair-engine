#pragma once

#include "Core/Containers/CrFixedVector.h"
#include "Core/String/CrFixedString.h"
#include "Core/FileSystem/CrFileSystem.h"
#include "Core/CrHash.h"
#include "Core/CrCoreForwardDeclarations.h"

#if defined(VULKAN_API)

#include <vulkan/vulkan.h>

using CrNativeShaderStage		= VkShaderModule;

struct VkVertexInputAttributeDescription;
using CrNativeInputLayout		= VkVertexInputAttributeDescription;

#endif

using bindpoint_t = uint8_t;

namespace ConstantBuffers { enum T : uint8_t; }
namespace Textures { enum T : uint8_t; }
namespace Samplers { enum T : uint8_t; }

// A class that represents both the input layout for the vertex shader
// and the constant resources needed by every stage
class CrShaderResourceSet
{
public:

	CrShaderResourceSet();

#if defined(VULKAN_API)

	// We store the descriptor set layout to connect it later on to the pipeline resource layout when creating it. The layout is also needed when allocating
	// descriptor sets from a pool.
	VkDescriptorSetLayout descriptorSetLayout;

#endif

	uint32_t GetConstantBufferTotalCount() const;

	uint32_t GetConstantBufferCount(cr3d::ShaderStage::T stage) const;

	ConstantBuffers::T GetConstantBufferID(cr3d::ShaderStage::T stage, uint32_t index) const;

	bindpoint_t GetConstantBufferBindPoint(cr3d::ShaderStage::T stage, uint32_t index) const;

	uint32_t GetTextureTotalCount() const;

	uint32_t GetTextureCount(cr3d::ShaderStage::T stage) const;

	Textures::T GetTextureID(cr3d::ShaderStage::T stage, uint32_t index) const;

	bindpoint_t GetTextureBindPoint(cr3d::ShaderStage::T stage, uint32_t index) const;

	uint32_t GetSamplerTotalCount() const;

	uint32_t GetSamplerCount(cr3d::ShaderStage::T stage) const;

	Samplers::T GetSamplerID(cr3d::ShaderStage::T stage, uint32_t index) const;

	bindpoint_t GetSamplerBindPoint(cr3d::ShaderStage::T stage, uint32_t index) const;

	uint32_t GetBufferCount() const;

	uint32_t GetImageCount() const;

	void AddConstantBuffer(cr3d::ShaderStage::T stage, ConstantBuffers::T id, bindpoint_t bindPoint);

	void AddTexture(cr3d::ShaderStage::T stage, Textures::T id, bindpoint_t bindPoint);

	void AddSampler(cr3d::ShaderStage::T stage, Samplers::T id, bindpoint_t bindPoint);

	static const uint32_t MaxStageConstantBuffers = 14; // Maximum constant buffers per stage

	static const uint32_t MaxStageTextures = 64; // Maximum textures per stage

	static const uint32_t MaxStageSamplers = 16; // Maximum samplers per stage

	ConstantBuffers::T	m_usedConstantBuffers[cr3d::ShaderStage::GraphicsStageCount][MaxStageConstantBuffers]; // Buffer ID
	Samplers::T			m_usedSamplers[cr3d::ShaderStage::GraphicsStageCount][MaxStageSamplers]; // IDs of the samplers this table uses
	Textures::T			m_usedTextures[cr3d::ShaderStage::GraphicsStageCount][MaxStageTextures]; // IDs of the textures this table uses

	bindpoint_t			m_usedConstantBufferBindPoints[cr3d::ShaderStage::GraphicsStageCount][MaxStageConstantBuffers];
	bindpoint_t			m_usedTextureBindPoints[cr3d::ShaderStage::GraphicsStageCount][MaxStageTextures];
	bindpoint_t			m_usedSamplerBindPoints[cr3d::ShaderStage::GraphicsStageCount][MaxStageSamplers];
	uint8_t				m_usedConstantBufferCount[cr3d::ShaderStage::GraphicsStageCount] = {};
	uint8_t				m_usedConstantBufferTotalCount = 0;

	uint8_t				m_usedTextureCount[cr3d::ShaderStage::GraphicsStageCount] = {};
	uint8_t				m_usedTextureTotalCount = 0;

	uint8_t				m_usedSamplerCount[cr3d::ShaderStage::GraphicsStageCount] = {};
	uint8_t				m_usedSamplerTotalCount = 0;

	uint8_t				m_usedBufferTotalCount = 0; // Structured buffers, constant buffers, etc.
	uint8_t				m_usedImageTotalCount = 0; // Images, samplers, combined image samplers, etc.
};

class CrShaderStageInfo
{
public:
	CrFixedString64 m_entryPointName;
	CrNativeShaderStage m_shader;
	cr3d::ShaderStage::T m_stage;
};

class CrGraphicsShader;
using CrGraphicsShaderHandle = CrSharedPtr<CrGraphicsShader>;

class CrShader
{
public:

	const CrHash& GetHash() const;

	const CrShaderResourceSet& GetResourceSet() const;

	cr3d::ShaderStage::T ShaderStageBegin() const;

	cr3d::ShaderStage::T ShaderStageEnd() const;

	CrFixedVector<CrShaderStageInfo, cr3d::ShaderStage::Count> m_shaderStages;

	CrHash		m_hash; // TODO Make private

	cr3d::ShaderStage::T shaderStageBegin;
	cr3d::ShaderStage::T shaderStageEnd;

private:
public: // HACK
	CrShaderResourceSet m_resourceSet;
};

inline cr3d::ShaderStage::T CrShader::ShaderStageBegin() const
{
	return shaderStageBegin;
}

inline cr3d::ShaderStage::T CrShader::ShaderStageEnd() const
{
	return shaderStageEnd;
}

// This shader represents a full linked shader. Therefore it knows about number of stages,
// and what these specific stages are. This is important to be able to pass it on to the PSO later on.
class CrGraphicsShader : public CrShader
{
public:

	CrGraphicsShader()
	{
		shaderStageBegin = cr3d::ShaderStage::Vertex;
		shaderStageEnd = cr3d::ShaderStage::GraphicsStageCount;
	}

	~CrGraphicsShader() {}

	// TODO HACK to get stuff going delete soon
	VkRenderPass m_vkRenderPass;
};

class CrComputeShader : public CrShader
{
public:

	CrComputeShader()
	{
		shaderStageBegin = cr3d::ShaderStage::Compute;
		shaderStageEnd = cr3d::ShaderStage::Count;
	}
};

struct CrGraphicsShaderStageCreate
{
	CrGraphicsShaderStageCreate() {}

	CrGraphicsShaderStageCreate(const CrPath& path, const CrFixedString64& entryPoint, cr3d::ShaderStage::T stage, cr3d::ShaderCodeFormat format) 
		: path(path), entryPoint(entryPoint), stage(stage), format(format) {}

	CrPath					path;
	CrVector<char>			source;
	CrVector<unsigned char>	bytecode;
	CrFixedString64			entryPoint;
	cr3d::ShaderCodeFormat	format;
	cr3d::ShaderStage::T	stage;
};

struct CrGraphicsShaderCreate
{
	friend class ICrShaderManager;
	friend class CrShaderManagerVulkan;

	void AddShaderStage(const CrGraphicsShaderStageCreate&& stage)
	{
		stages.push_back(stage);
	}

private:
	CrVector<CrGraphicsShaderStageCreate> stages;
};
