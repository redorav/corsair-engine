#pragma once

#include "Core/Containers/CrFixedVector.h"
#include "Core/String/CrFixedString.h"
#include "Core/FileSystem/CrPathString.h"

#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/CrHash.h"

#include "Core/CrCoreForwardDeclarations.h"
#include "Rendering/CrRenderingForwardDeclarations.h"

using bindpoint_t = uint8_t;

// TODO Move to a forward declaration file ShaderResourcesForwardDeclarations.h
namespace ConstantBuffers { enum T : uint8_t; }
namespace Samplers { enum T : uint8_t; }
namespace Textures { enum T : uint8_t; }
namespace RWTextures { enum T : uint8_t; }
namespace StorageBuffers { enum T : uint8_t; }
namespace RWStorageBuffers { enum T : uint8_t; }
namespace RWDataBuffers { enum T : uint8_t; }

namespace cr { namespace Platform { enum T : uint8_t; } }
namespace cr3d { namespace GraphicsApi { enum T : uint8_t; } }

struct CrShaderBinding
{
	CrShaderBinding() {}

	CrShaderBinding(bindpoint_t bindPoint, cr3d::ShaderStage::T stage, ConstantBuffers::T constantBufferID)
		: bindPoint(bindPoint), stage(stage), type(cr3d::ShaderResourceType::ConstantBuffer), constantBufferID(constantBufferID) {}

	CrShaderBinding(bindpoint_t bindPoint, cr3d::ShaderStage::T stage, Samplers::T samplerID)
		: bindPoint(bindPoint), stage(stage), type(cr3d::ShaderResourceType::Sampler), samplerID(samplerID) {}

	CrShaderBinding(bindpoint_t bindPoint, cr3d::ShaderStage::T stage, Textures::T textureID)
		: bindPoint(bindPoint), stage(stage), type(cr3d::ShaderResourceType::Texture), textureID(textureID) {}

	CrShaderBinding(bindpoint_t bindPoint, cr3d::ShaderStage::T stage, RWTextures::T rwTextureID)
		: bindPoint(bindPoint), stage(stage), type(cr3d::ShaderResourceType::RWTexture), rwTextureID(rwTextureID) {}

	CrShaderBinding(bindpoint_t bindPoint, cr3d::ShaderStage::T stage, StorageBuffers::T storageBufferID)
		: bindPoint(bindPoint), stage(stage), type(cr3d::ShaderResourceType::StorageBuffer), storageBufferID(storageBufferID) {}

	CrShaderBinding(bindpoint_t bindPoint, cr3d::ShaderStage::T stage, RWStorageBuffers::T rwStorageBufferID)
		: bindPoint(bindPoint), stage(stage), type(cr3d::ShaderResourceType::RWStorageBuffer), rwStorageBufferID(rwStorageBufferID) {}

	CrShaderBinding(bindpoint_t bindPoint, cr3d::ShaderStage::T stage, RWDataBuffers::T rwDataBufferID)
		: bindPoint(bindPoint), stage(stage), type(cr3d::ShaderResourceType::RWDataBuffer), rwDataBufferID(rwDataBufferID) {}

	bindpoint_t bindPoint;
	cr3d::ShaderStage::T stage : 4;
	cr3d::ShaderResourceType::T type : 4;
	union
	{
		ConstantBuffers::T constantBufferID;
		Samplers::T samplerID;
		Textures::T textureID;
		RWTextures::T rwTextureID;
		StorageBuffers::T storageBufferID;
		RWStorageBuffers::T rwStorageBufferID;
		RWDataBuffers::T rwDataBufferID;
	};
};

struct CrShaderBindingTableResources
{
	CrFixedVector<CrShaderBinding, 64> constantBuffers;
	CrFixedVector<CrShaderBinding, 64> samplers;
	CrFixedVector<CrShaderBinding, 64> textures;
	CrFixedVector<CrShaderBinding, 64> rwTextures;

	CrFixedVector<CrShaderBinding, 64> storageBuffers;
	CrFixedVector<CrShaderBinding, 64> rwStorageBuffers;
	CrFixedVector<CrShaderBinding, 64> dataBuffers;
	CrFixedVector<CrShaderBinding, 64> rwDataBuffers;
};

// A class that represents both the input layout for the vertex shader
// and the constant resources needed by every stage
class ICrShaderBindingTable
{
public:

	ICrShaderBindingTable(const CrShaderBindingTableResources& resources);

	virtual ~ICrShaderBindingTable() {}

	static const uint32_t MaxStageConstantBuffers = 14; // Maximum constant buffers per stage

	static const uint32_t MaxStageTextures = 64; // Maximum textures per stage

	static const uint32_t MaxStageSamplers = 16; // Maximum samplers per stage

	template<typename Fn>
	void ForEachConstantBuffer(const Fn& fn) const
	{
		for (uint8_t i = m_constantBufferOffset; i < m_constantBufferOffset + m_constantBufferCount; ++i)
		{
			fn(m_bindings[i].stage, m_bindings[i].constantBufferID, m_bindings[i].bindPoint);
		}
	}

	template<typename Fn>
	void ForEachSampler(const Fn& fn) const
	{
		for (uint8_t i = m_samplerOffset; i < m_samplerOffset + m_samplerCount; ++i)
		{
			fn(m_bindings[i].stage, m_bindings[i].samplerID, m_bindings[i].bindPoint);
		}
	}

	template<typename Fn>
	void ForEachTexture(const Fn& fn) const
	{
		for (uint8_t i = m_textureOffset; i < m_textureOffset + m_textureCount; ++i)
		{
			fn(m_bindings[i].stage, m_bindings[i].textureID, m_bindings[i].bindPoint);
		}
	}

	template<typename Fn>
	void ForEachRWTexture(const Fn& fn) const
	{
		for (uint8_t i = m_rwTextureOffset; i < m_rwTextureOffset + m_rwTextureCount; ++i)
		{
			fn(m_bindings[i].stage, m_bindings[i].rwTextureID, m_bindings[i].bindPoint);
		}
	}

	template<typename Fn>
	void ForEachStorageBuffer(const Fn& fn) const
	{
		for (uint8_t i = m_storageBufferOffset; i < m_storageBufferOffset + m_storageBufferCount; ++i)
		{
			fn(m_bindings[i].stage, m_bindings[i].storageBufferID, m_bindings[i].bindPoint);
		}
	}

	template<typename Fn>
	void ForEachRWStorageBuffer(const Fn& fn) const
	{
		for (uint8_t i = m_rwStorageBufferOffset; i < m_rwStorageBufferOffset + m_rwStorageBufferCount; ++i)
		{
			fn(m_bindings[i].stage, m_bindings[i].rwStorageBufferID, m_bindings[i].bindPoint);
		}
	}

	template<typename Fn>
	void ForEachRWDataBuffer(const Fn& fn) const
	{
		for (uint8_t i = m_rwDataBufferOffset; i < m_rwDataBufferOffset + m_rwDataBufferCount; ++i)
		{
			fn(m_bindings[i].stage, m_bindings[i].rwDataBufferID, m_bindings[i].bindPoint);
		}
	}

private:

	uint8_t				m_constantBufferOffset = 0;
	uint8_t				m_constantBufferCount = 0;

	uint8_t				m_samplerOffset = 0;
	uint8_t				m_samplerCount = 0;

	uint8_t				m_textureOffset = 0;
	uint8_t				m_textureCount = 0;

	uint8_t				m_rwTextureOffset = 0;
	uint8_t				m_rwTextureCount = 0;

	uint8_t				m_storageBufferOffset = 0;
	uint8_t				m_storageBufferCount = 0;

	uint8_t				m_rwStorageBufferOffset = 0;
	uint8_t				m_rwStorageBufferCount = 0;

	uint8_t				m_rwDataBufferOffset = 0;
	uint8_t				m_rwDataBufferCount = 0;

	CrFixedVector<CrShaderBinding, 64> m_bindings;
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

class ICrShader
{
public:

	virtual ~ICrShader() {}

	// TODO We need a way to build the hash for a shader. The hash doesn't necessarily
	// want to be the bytecode (perhaps a stripped version of the bytecode?) We need to
	// give this some thought.

	const CrHash& GetHash() const
	{
		return m_hash;
	}

	const ICrShaderBindingTable& GetBindingTable() const
	{
		return *m_bindingTable.get();
	}	

protected:

	CrUniquePtr<ICrShaderBindingTable> m_bindingTable;

	CrHash m_hash;
};

// This shader represents a full linked shader. Therefore it knows about number of stages,
// and what these specific stages are. This is important to be able to pass it on to the PSO later on.
class ICrGraphicsShader : public ICrShader
{
public:

	ICrGraphicsShader(const ICrRenderDevice* /*renderDevice*/, const CrGraphicsShaderDescriptor& graphicsShaderDescriptor)
	{
		for (const CrShaderBytecodeSharedHandle& bytecode : graphicsShaderDescriptor.m_bytecodes)
		{
			CrShaderStageInfo info;
			info.entryPoint = bytecode->GetEntryPoint();
			info.stage = bytecode->GetShaderStage();
			m_stageInfos.push_back(info);
		}
	}

	virtual ~ICrGraphicsShader() {}

	const CrVector<CrShaderStageInfo>& GetStages() const
	{
		return m_stageInfos;
	}

private:

	CrVector<CrShaderStageInfo> m_stageInfos;
};

struct CrComputeShaderDescriptor
{
	CrShaderBytecodeSharedHandle m_bytecode;
};

class ICrComputeShader : public ICrShader
{
public:

	ICrComputeShader(const ICrRenderDevice* /*renderDevice*/, const CrComputeShaderDescriptor& computeShaderDescriptor)
	{
		m_stageInfo.entryPoint = computeShaderDescriptor.m_bytecode->GetEntryPoint();
		m_stageInfo.stage = cr3d::ShaderStage::Compute;
	}

	virtual ~ICrComputeShader() {}

	CrShaderStageInfo m_stageInfo;
};

struct CrShaderBytecodeDescriptor
{
	CrShaderBytecodeDescriptor
	(
		const CrPathString& path, const CrFixedString128& entryPoint, cr3d::ShaderStage::T stage, 
		cr3d::ShaderCodeFormat format, cr3d::GraphicsApi::T graphicsApi, cr::Platform::T platform
	)
		: path(path), entryPoint(entryPoint), stage(stage), format(format), graphicsApi(graphicsApi), platform(platform) {}

	CrPathString              path;
	CrFixedString128          entryPoint;
	CrVector<CrFixedString64> defines;
	cr3d::ShaderCodeFormat    format;
	cr3d::ShaderStage::T      stage;
	cr3d::GraphicsApi::T      graphicsApi;
	cr::Platform::T           platform;
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
