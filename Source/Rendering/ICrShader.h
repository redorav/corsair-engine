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

// Represents the resources needed by every stage. It is designed to be 
// fast iterate in order to rebuild tables quickly. Resources are sorted
// according to type. This is not a resource table, just the binding points,
// owned by the shader. It also doesn't contain any names, but points to the 
// builtins that the engine knows about
class ICrShaderBindingLayout
{
public:

	ICrShaderBindingLayout(const CrShaderBindingLayoutResources& resources);

	virtual ~ICrShaderBindingLayout() {}

	static const uint32_t MaxStageConstantBuffers = 14; // Maximum constant buffers per stage

	static const uint32_t MaxStageTextures = 64; // Maximum textures per stage

	static const uint32_t MaxStageSamplers = 16; // Maximum samplers per stage

	template<typename FunctionT>
	void ForEachConstantBuffer(const FunctionT& fn) const
	{
		for (uint8_t i = m_constantBufferOffset; i < m_constantBufferOffset + m_constantBufferCount; ++i)
		{
			fn(m_bindings[i].stage, m_bindings[i].constantBufferID, m_bindings[i].bindPoint);
		}
	}

	template<typename FunctionT>
	void ForEachSampler(const FunctionT& fn) const
	{
		for (uint8_t i = m_samplerOffset; i < m_samplerOffset + m_samplerCount; ++i)
		{
			fn(m_bindings[i].stage, m_bindings[i].samplerID, m_bindings[i].bindPoint);
		}
	}

	template<typename FunctionT>
	void ForEachTexture(const FunctionT& fn) const
	{
		for (uint8_t i = m_textureOffset; i < m_textureOffset + m_textureCount; ++i)
		{
			fn(m_bindings[i].stage, m_bindings[i].textureID, m_bindings[i].bindPoint);
		}
	}

	template<typename FunctionT>
	void ForEachRWTexture(const FunctionT& fn) const
	{
		for (uint8_t i = m_rwTextureOffset; i < m_rwTextureOffset + m_rwTextureCount; ++i)
		{
			fn(m_bindings[i].stage, m_bindings[i].rwTextureID, m_bindings[i].bindPoint);
		}
	}

	template<typename FunctionT>
	void ForEachStorageBuffer(const FunctionT& fn) const
	{
		for (uint8_t i = m_storageBufferOffset; i < m_storageBufferOffset + m_storageBufferCount; ++i)
		{
			fn(m_bindings[i].stage, m_bindings[i].storageBufferID, m_bindings[i].bindPoint);
		}
	}

	template<typename FunctionT>
	void ForEachRWStorageBuffer(const FunctionT& fn) const
	{
		for (uint8_t i = m_rwStorageBufferOffset; i < m_rwStorageBufferOffset + m_rwStorageBufferCount; ++i)
		{
			fn(m_bindings[i].stage, m_bindings[i].rwStorageBufferID, m_bindings[i].bindPoint);
		}
	}

	template<typename FunctionT>
	void ForEachRWDataBuffer(const FunctionT& function) const
	{
		for (uint8_t i = m_rwDataBufferOffset; i < m_rwDataBufferOffset + m_rwDataBufferCount; ++i)
		{
			function(m_bindings[i].stage, m_bindings[i].rwDataBufferID, m_bindings[i].bindPoint);
		}
	}

	template<typename FunctionT>
	static void AddResources(const CrShaderReflectionHeader& reflectionHeader, CrShaderBindingLayoutResources& resources, const FunctionT& function);

	uint8_t GetConstantBufferCount() const { return m_constantBufferCount; }

	uint8_t GetSamplerCount() const { return m_samplerCount; }

	uint8_t GetTextureCount() const { return m_textureCount; }

	uint8_t GetRWTextureCount() const { return m_rwTextureCount; }

	uint8_t GetStorageBufferCount() const { return m_storageBufferCount; }

	uint8_t GetRWStorageBufferCount() const { return m_rwStorageBufferCount; }

	uint8_t GetRWDataBufferCount() const { return m_rwDataBufferCount; }

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

typedef CrFixedString64 CrShaderDebugString;

struct CrGraphicsShaderDescriptor
{
	CrShaderDebugString m_debugName;

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
		return *m_bindingLayout.get();
	}

	const char* GetDebugName() const
	{
		return m_debugName.c_str();
	}

protected:

	CrShaderDebugString m_debugName;

	CrUniquePtr<ICrShaderBindingLayout> m_bindingLayout;

	// Hash produced from the bytecodes belonging to this shader
	CrHash m_hash;
};

// This shader represents a full linked shader. Therefore it knows about number of stages,
// and what these specific stages are. This is important to be able to pass it on to the PSO later on.
class ICrGraphicsShader : public ICrShader
{
public:

	ICrGraphicsShader(ICrRenderDevice* /*renderDevice*/, const CrGraphicsShaderDescriptor& graphicsShaderDescriptor);

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
	CrShaderDebugString m_debugName;

	CrShaderBytecodeSharedHandle m_bytecode;
};

class ICrComputeShader : public ICrShader
{
public:

	ICrComputeShader(ICrRenderDevice* /*renderDevice*/, const CrComputeShaderDescriptor& computeShaderDescriptor)
	{
		m_bytecode  = computeShaderDescriptor.m_bytecode;
		m_hash      = computeShaderDescriptor.m_bytecode->GetHash();
		m_debugName = computeShaderDescriptor.m_debugName;
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
struct CrShaderCompilerDefines
{
	void AddDefine(const CrString& define)
	{
		defines.push_back(define);
	}

	const CrVector<CrString>& GetDefines() const
	{
		return defines;
	}

	static CrShaderCompilerDefines Dummy;

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

	const CrShaderCompilerDefines& GetDefines() const
	{
		return m_defines;
	}

private:

	CrVector<CrShaderBytecodeCompilationDescriptor> m_stageBytecodeDescriptors;
	CrShaderCompilerDefines m_defines;
};
