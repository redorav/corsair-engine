#pragma once

#include "Core/Containers/CrFixedVector.h"
#include "Core/String/CrFixedString.h"
#include "Core/FileSystem/CrPath.h"

#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/CrHash.h"
#include "Core/Streams/CrFileStream.h"

#include "Rendering/CrVertexDescriptor.h"
#include "Rendering/CrShaderReflectionHeader.h"

#include "Core/CrCoreForwardDeclarations.h"
#include "Rendering/CrRenderingForwardDeclarations.h"

using bindpoint_t = uint8_t;

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

// We use this table to bucket resources before passing
// them on (sorted) to the shader binding layout.
struct CrShaderBindingLayoutResources
{
	CrFixedVector<CrShaderBinding, 64> constantBuffers;
	CrFixedVector<CrShaderBinding, 16> samplers;
	CrFixedVector<CrShaderBinding, 64> textures;
	CrFixedVector<CrShaderBinding, 32> rwTextures;

	CrFixedVector<CrShaderBinding, 32> storageBuffers;
	CrFixedVector<CrShaderBinding, 32> rwStorageBuffers;
	CrFixedVector<CrShaderBinding, 32> dataBuffers;
	CrFixedVector<CrShaderBinding, 32> rwDataBuffers;
};

// A class that represents the resources needed by every stage. It is designed to be 
// fast to loop through in order to rebuild tables quickly. Resources are sorted
// according to type. This is not the resource table, just the binding points. 
// It also doesn't contain any names, but points to the builtins that the engine knows
// about
class ICrShaderBindingLayout
{
public:

	ICrShaderBindingLayout(const CrShaderBindingLayoutResources& resources);

	virtual ~ICrShaderBindingLayout() {}

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

struct CrVertexInput
{
	uint16_t semantic   : 5; // CrVertexSemantic::T
	uint16_t components : 3;
};

static_assert(sizeof(CrVertexInput) == 2, "CrVertexInput size mismatch");

struct CrInputSignature
{
	CrFixedVector<CrVertexInput, cr3d::MaxVertexStreams> inputs;
};

// Bytecode represents a shader code, e.g. vertex, pixel, etc
class CrShaderBytecode
{
public:

	CrShaderBytecode() {}

	const CrVector<unsigned char>& GetBytecode() const
	{
		return m_bytecode;
	}

	const CrString& GetEntryPoint() const
	{
		return m_reflection.entryPoint;
	}

	cr3d::ShaderStage::T GetShaderStage() const
	{
		return m_reflection.shaderStage;
	}

	const CrShaderReflectionHeader& GetReflection() const
	{
		return m_reflection;
	}

	CrHash GetHash() const
	{
		return m_hash;
	}

	void ComputeHash()
	{
		m_hash = CrHash(m_bytecode.data(), m_bytecode.size());
	}

	template<typename StreamT>
	friend StreamT& operator << (StreamT& stream, CrShaderBytecode& bytecode);

private:

	CrVector<uint8_t> m_bytecode;
	CrShaderReflectionHeader m_reflection;
	CrHash m_hash; // A hash of the entire bytecode
};

template<typename StreamT>
StreamT& operator << (StreamT& stream, CrShaderBytecode& bytecode)
{
	stream << bytecode.m_reflection;
	stream << bytecode.m_bytecode;
	if (stream.IsReading())
	{
		bytecode.ComputeHash();
	}
	return stream;
}

struct CrGraphicsShaderDescriptor
{
	CrVector<CrShaderBytecodeSharedHandle> m_bytecodes;
};

class ICrShader
{
public:

	virtual ~ICrShader() {}

	CrHash GetHash() const
	{
		return m_hash;
	}

	const ICrShaderBindingLayout& GetBindingLayout() const
	{
		return *m_bindingTable.get();
	}	

protected:

	CrUniquePtr<ICrShaderBindingLayout> m_bindingTable;

	// Hash produced from the bytecodes belonging to this shader
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
			m_bytecodes.push_back(bytecode);
			m_hash <<= bytecode->GetHash();
		}
	}

	virtual ~ICrGraphicsShader() {}

	const CrVector<CrShaderBytecodeSharedHandle>& GetBytecodes() const
	{
		return m_bytecodes;
	}

	const CrInputSignature& GetInputSignature() const
	{
		return m_inputSignature;
	}

protected:

	CrInputSignature m_inputSignature;

	CrVector<CrShaderBytecodeSharedHandle> m_bytecodes;
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
		m_bytecode = computeShaderDescriptor.m_bytecode;
		m_hash = computeShaderDescriptor.m_bytecode->GetHash();
	}

	const CrShaderBytecodeSharedHandle& GetBytecode() const
	{
		return m_bytecode;
	}

	virtual ~ICrComputeShader() {}

private:

	CrShaderBytecodeSharedHandle m_bytecode;
};

struct CrShaderBytecodeCompilationDescriptor
{
	CrShaderBytecodeCompilationDescriptor
	(const CrPath& path, const CrFixedString128& entryPoint, cr3d::ShaderStage::T stage, cr3d::GraphicsApi::T graphicsApi, cr::Platform::T platform)
		: path(path), entryPoint(entryPoint), stage(stage), graphicsApi(graphicsApi), platform(platform) {}

	const CrPath                    path;
	const CrFixedString128          entryPoint;
	const cr3d::ShaderStage::T      stage;
	const cr3d::GraphicsApi::T      graphicsApi;
	const cr::Platform::T           platform;
};

// Add defines to a shader compilation
// Cannot be in bytecode descriptor because
// it's a shader-wide operation
struct CrShaderDefines
{
	void AddDefine(const CrString& define)
	{
		defines.push_back(define);
	}

	const CrVector<CrString>& GetDefines() const
	{
		return defines;
	}

	static CrShaderDefines Dummy;

private:

	CrVector<CrString> defines;
};

struct CrShaderCompilationDescriptor
{
	void AddBytecodeDescriptor(const CrShaderBytecodeCompilationDescriptor& bytecodeDescriptor)
	{
		m_stageBytecodeDescriptors.push_back(bytecodeDescriptor);
	}

	void AddDefine(const CrString& define)
	{
		m_defines.AddDefine(define);
	}

	const CrVector<CrShaderBytecodeCompilationDescriptor>& GetBytecodeDescriptors() const
	{
		return m_stageBytecodeDescriptors;
	}

	const CrShaderDefines& GetDefines() const
	{
		return m_defines;
	}

private:

	CrVector<CrShaderBytecodeCompilationDescriptor> m_stageBytecodeDescriptors;
	CrShaderDefines m_defines;
};
