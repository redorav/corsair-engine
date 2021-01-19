#pragma once

#include "Core/Containers/CrFixedVector.h"
#include "Core/String/CrFixedString.h"
#include "Core/FileSystem/CrFileSystem.h"
#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/CrHash.h"
#include "Core/CrCoreForwardDeclarations.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

using bindpoint_t = uint8_t;

namespace ConstantBuffers { enum T : uint8_t; }
namespace Textures { enum T : uint8_t; }
namespace Samplers { enum T : uint8_t; }

namespace cr { namespace Platform { enum T : uint8_t; } }
namespace cr3d { namespace GraphicsApi { enum T : uint8_t; } }

// A class that represents both the input layout for the vertex shader
// and the constant resources needed by every stage
class ICrShaderResourceTable
{
public:

	ICrShaderResourceTable();

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

	void AddConstantBuffer(cr3d::ShaderStage::T stage, ConstantBuffers::T id, bindpoint_t bindPoint);

	void AddTexture(cr3d::ShaderStage::T stage, Textures::T id, bindpoint_t bindPoint);

	void AddSampler(cr3d::ShaderStage::T stage, Samplers::T id, bindpoint_t bindPoint);

	static const uint32_t MaxStageConstantBuffers = 14; // Maximum constant buffers per stage

	static const uint32_t MaxStageTextures = 64; // Maximum textures per stage

	static const uint32_t MaxStageSamplers = 16; // Maximum samplers per stage

private:

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
};

struct CrShaderStageInfo
{
	CrFixedString128 entryPoint;
	cr3d::ShaderStage::T stage;
};

// Bytecode represents a shader code, e.g. vertex, pixel, etc
class CrShaderBytecode
{
public:

	// We take ownership of the bytecode to avoid all the coming and going of data
	CrShaderBytecode(const CrVector<unsigned char>&& bytecode, const CrFixedString128& entryPoint, cr3d::ShaderStage::T shaderStage)
	{
		m_bytecode = std::move(bytecode);
		m_entryPoint = entryPoint;
		m_shaderStage = shaderStage;
	}

	const CrVector<unsigned char>& GetBytecode() const
	{
		return m_bytecode;
	}

	const CrFixedString128& GetEntryPoint() const
	{
		return m_entryPoint;
	}

	cr3d::ShaderStage::T GetShaderStage() const
	{
		return m_shaderStage;
	}

private:

	CrVector<unsigned char> m_bytecode;
	CrFixedString128 m_entryPoint;
	cr3d::ShaderStage::T m_shaderStage;
};

struct CrGraphicsShaderDescriptor
{
	CrVector<CrShaderBytecodeSharedHandle> m_bytecodes;
};

class ICrGraphicsShader;
using CrGraphicsShaderHandle = CrSharedPtr<ICrGraphicsShader>;

class ICrShader
{
public:

	// TODO We need a way to build the hash for a shader. The hash doesn't necessarily
	// want to be the bytecode (perhaps a stripped version of the bytecode?) We need to
	// give this some thought.

	const CrHash& GetHash() const
	{
		return m_hash;
	}

	const ICrShaderResourceTable& GetResourceTable() const
	{
		return *m_resourceTable.get();
	}	

protected:

	CrUniquePtr<ICrShaderResourceTable> m_resourceTable;

	CrHash m_hash;
};

// This shader represents a full linked shader. Therefore it knows about number of stages,
// and what these specific stages are. This is important to be able to pass it on to the PSO later on.
class ICrGraphicsShader : public ICrShader
{
public:

	ICrGraphicsShader(const ICrRenderDevice* renderDevice, const CrGraphicsShaderDescriptor& graphicsShaderDescriptor)
	{
		renderDevice;

		for (const CrShaderBytecodeSharedHandle& bytecode : graphicsShaderDescriptor.m_bytecodes)
		{
			CrShaderStageInfo info;
			info.entryPoint = bytecode->GetEntryPoint();
			info.stage = bytecode->GetShaderStage();
			m_stageInfos.push_back(info);
		}
	}

	~ICrGraphicsShader() {}

	const CrVector<CrShaderStageInfo>& GetStages() const
	{
		return m_stageInfos;
	}

private:

	CrVector<CrShaderStageInfo> m_stageInfos;
};

class ICrComputeShader : public ICrShader
{
public:

	ICrComputeShader() {}

	~ICrComputeShader() {}
};

struct CrShaderBytecodeDescriptor
{
	CrShaderBytecodeDescriptor
	(
		const CrPath& path, const CrFixedString128& entryPoint, cr3d::ShaderStage::T stage, 
		cr3d::ShaderCodeFormat format, cr3d::GraphicsApi::T graphicsApi, cr::Platform::T platform
	)
		: path(path), entryPoint(entryPoint), stage(stage), format(format), graphicsApi(graphicsApi), platform(platform) {}

	const CrPath                 path;
	const CrFixedString128       entryPoint;
	const cr3d::ShaderCodeFormat format;
	const cr3d::ShaderStage::T   stage;
	const cr3d::GraphicsApi::T   graphicsApi;
	const cr::Platform::T        platform;
};

struct CrBytecodeLoadDescriptor
{
	void AddBytecodeDescriptor(const CrShaderBytecodeDescriptor& bytecodeDescriptor)
	{
		m_stageBytecodeDescriptors.push_back(bytecodeDescriptor);
	}

	const CrVector<CrShaderBytecodeDescriptor>& GetBytecodeDescriptors() const
	{
		return m_stageBytecodeDescriptors;
	}

private:

	CrVector<CrShaderBytecodeDescriptor> m_stageBytecodeDescriptors;
};
