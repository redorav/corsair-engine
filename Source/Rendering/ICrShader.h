#pragma once

#include "Core/Containers/CrFixedVector.h"
#include "Core/String/CrFixedString.h"
#include "Core/FileSystem/CrFixedPath.h"

#include "Core/CrHash.h"
#include "Core/Streams/CrFileStream.h"
#include "Core/SmartPointers/CrUniquePtr.h"

#include "Rendering/CrVertexDescriptor.h"
#include "Rendering/CrShaderReflectionHeader.h"

#include "Core/CrCoreForwardDeclarations.h"
#include "Rendering/CrRenderingForwardDeclarations.h"

using bindpoint_t = uint8_t;

struct CrShaderBinding
{
	CrShaderBinding() {}

	CrShaderBinding(bindpoint_t bindPoint, cr3d::ShaderStage::T stage, ConstantBuffers::T constantBufferID)
		: bindPoint(bindPoint), stage((uint8_t)stage), type(cr3d::ShaderResourceType::ConstantBuffer), constantBufferID(constantBufferID) {}

	CrShaderBinding(bindpoint_t bindPoint, cr3d::ShaderStage::T stage, Samplers::T samplerID)
		: bindPoint(bindPoint), stage((uint8_t)stage), type(cr3d::ShaderResourceType::Sampler), samplerID(samplerID) {}

	CrShaderBinding(bindpoint_t bindPoint, cr3d::ShaderStage::T stage, Textures::T textureID)
		: bindPoint(bindPoint), stage((uint8_t)stage), type(cr3d::ShaderResourceType::Texture), textureID(textureID) {}

	CrShaderBinding(bindpoint_t bindPoint, cr3d::ShaderStage::T stage, RWTextures::T rwTextureID)
		: bindPoint(bindPoint), stage((uint8_t)stage), type(cr3d::ShaderResourceType::RWTexture), rwTextureID(rwTextureID) {}

	CrShaderBinding(bindpoint_t bindPoint, cr3d::ShaderStage::T stage, StorageBuffers::T storageBufferID)
		: bindPoint(bindPoint), stage((uint8_t)stage), type(cr3d::ShaderResourceType::StorageBuffer), storageBufferID(storageBufferID) {}

	CrShaderBinding(bindpoint_t bindPoint, cr3d::ShaderStage::T stage, RWStorageBuffers::T rwStorageBufferID)
		: bindPoint(bindPoint), stage((uint8_t)stage), type(cr3d::ShaderResourceType::RWStorageBuffer), rwStorageBufferID(rwStorageBufferID) {}

	CrShaderBinding(bindpoint_t bindPoint, cr3d::ShaderStage::T stage, RWTypedBuffers::T rwTypedBufferID)
		: bindPoint(bindPoint), stage((uint8_t)stage), type(cr3d::ShaderResourceType::RWTypedBuffer), rwTypedBufferID(rwTypedBufferID) {}

	bindpoint_t bindPoint;
	uint8_t stage : 4; // cr3d::ShaderStage::T
	uint8_t type : 4; // cr3d::ShaderResourceType::T
	union
	{
		ConstantBuffers::T constantBufferID;
		Samplers::T samplerID;
		Textures::T textureID;
		RWTextures::T rwTextureID;
		StorageBuffers::T storageBufferID;
		RWStorageBuffers::T rwStorageBufferID;
		RWTypedBuffers::T rwTypedBufferID;
		uint8_t id;
	};
};

static_assert(cr3d::ShaderStage::Count < 16);
static_assert(cr3d::ShaderResourceType::Count < 16);

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
	CrFixedVector<CrShaderBinding, 32> rwTypedBuffers;
};

// Represents the resources needed by every stage. It is designed to be 
// fast iterate in order to rebuild tables quickly. Resources are sorted
// according to type. This is not a resource table, just the binding points,
// owned by the shader. It also doesn't contain any names, but points to the 
// builtins that the engine knows about
class ICrShaderBindingLayout final
{
public:

	ICrShaderBindingLayout(const CrShaderBindingLayoutResources& resources);

	struct ShaderResourceOffset
	{
		uint8_t offset;
		uint8_t count;
	};

	template<typename FunctionT, typename ResourceID>
	void ForEachResourceType(cr3d::ShaderResourceType::T resourceType, const FunctionT& function) const
	{
		const ShaderResourceOffset& resourceOffset = m_resourceOffsets[resourceType];
		for (uint8_t i = resourceOffset.offset; i < resourceOffset.offset + resourceOffset.count; ++i)
		{
			function((cr3d::ShaderStage::T)m_bindings[i].stage, (ResourceID)m_bindings[i].id, m_bindings[i].bindPoint);
		}
	}

	template<typename FunctionT, typename ResourceID>
	void ForEachResourceType(cr3d::ShaderStage::T shaderStage, cr3d::ShaderResourceType::T resourceType, const FunctionT& function) const
	{
		const ShaderResourceOffset& resourceOffset = m_stageResourceOffsets[resourceType][GetStageIndex(shaderStage)];
		for (uint8_t i = resourceOffset.offset; i < resourceOffset.offset + resourceOffset.count; ++i)
		{
			function((cr3d::ShaderStage::T)m_bindings[i].stage, (ResourceID)m_bindings[i].id, m_bindings[i].bindPoint);
		}
	}

	template<typename FunctionT>
	void ForEachConstantBuffer(const FunctionT& function) const
	{
		ForEachResourceType<FunctionT, ConstantBuffers::T>(cr3d::ShaderResourceType::ConstantBuffer, function);
	}

	template<typename FunctionT>
	void ForEachConstantBuffer(cr3d::ShaderStage::T shaderStage, const FunctionT& function) const
	{
		ForEachResourceType<FunctionT, ConstantBuffers::T>(shaderStage, cr3d::ShaderResourceType::ConstantBuffer, function);
	}

	template<typename FunctionT>
	void ForEachSampler(const FunctionT& function) const
	{
		ForEachResourceType<FunctionT, Samplers::T>(cr3d::ShaderResourceType::Sampler, function);
	}

	template<typename FunctionT>
	void ForEachSampler(cr3d::ShaderStage::T shaderStage, const FunctionT& function) const
	{
		ForEachResourceType<FunctionT, Samplers::T>(shaderStage, cr3d::ShaderResourceType::Sampler, function);
	}

	template<typename FunctionT>
	void ForEachTexture(const FunctionT& function) const
	{
		ForEachResourceType<FunctionT, Textures::T>(cr3d::ShaderResourceType::Texture, function);
	}

	template<typename FunctionT>
	void ForEachTexture(cr3d::ShaderStage::T shaderStage, const FunctionT& function) const
	{
		ForEachResourceType<FunctionT, Textures::T>(shaderStage, cr3d::ShaderResourceType::Texture, function);
	}

	template<typename FunctionT>
	void ForEachRWTexture(const FunctionT& function) const
	{
		ForEachResourceType<FunctionT, RWTextures::T>(cr3d::ShaderResourceType::RWTexture, function);
	}

	template<typename FunctionT>
	void ForEachRWTexture(cr3d::ShaderStage::T shaderStage, const FunctionT& function) const
	{
		ForEachResourceType<FunctionT, RWTextures::T>(shaderStage, cr3d::ShaderResourceType::RWTexture, function);
	}

	template<typename FunctionT>
	void ForEachStorageBuffer(const FunctionT& function) const
	{
		ForEachResourceType<FunctionT, StorageBuffers::T>(cr3d::ShaderResourceType::StorageBuffer, function);
	}

	template<typename FunctionT>
	void ForEachStorageBuffer(cr3d::ShaderStage::T shaderStage, const FunctionT& function) const
	{
		ForEachResourceType<FunctionT, StorageBuffers::T>(shaderStage, cr3d::ShaderResourceType::StorageBuffer, function);
	}

	template<typename FunctionT>
	void ForEachRWStorageBuffer(const FunctionT& function) const
	{
		ForEachResourceType<FunctionT, RWStorageBuffers::T>(cr3d::ShaderResourceType::RWStorageBuffer, function);
	}

	template<typename FunctionT>
	void ForEachRWStorageBuffer(cr3d::ShaderStage::T shaderStage, const FunctionT& function) const
	{
		ForEachResourceType<FunctionT, RWStorageBuffers::T>(shaderStage, cr3d::ShaderResourceType::RWStorageBuffer, function);
	}

	template<typename FunctionT>
	void ForEachRWTypedBuffer(const FunctionT& function) const
	{
		ForEachResourceType<FunctionT, RWTypedBuffers::T>(cr3d::ShaderResourceType::RWTypedBuffer, function);
	}

	template<typename FunctionT>
	void ForEachRWTypedBuffer(cr3d::ShaderStage::T shaderStage, const FunctionT& function) const
	{
		ForEachResourceType<FunctionT, RWTypedBuffers::T>(shaderStage, cr3d::ShaderResourceType::RWTypedBuffer, function);
	}

	template<typename FunctionT>
	static void AddResources(const CrShaderReflectionHeader& reflectionHeader, CrShaderBindingLayoutResources& resources, const FunctionT& function);

	uint8_t GetTotalResourceCount() const { return m_totalResourceCount; }

	uint8_t GetConstantBufferCount()  const { return m_resourceOffsets[cr3d::ShaderResourceType::ConstantBuffer].count; }
	uint8_t GetSamplerCount()         const { return m_resourceOffsets[cr3d::ShaderResourceType::Sampler].count; }
	uint8_t GetTextureCount()         const { return m_resourceOffsets[cr3d::ShaderResourceType::Texture].count; }
	uint8_t GetRWTextureCount()       const { return m_resourceOffsets[cr3d::ShaderResourceType::RWTexture].count; }
	uint8_t GetStorageBufferCount()   const { return m_resourceOffsets[cr3d::ShaderResourceType::StorageBuffer].count; }
	uint8_t GetRWStorageBufferCount() const { return m_resourceOffsets[cr3d::ShaderResourceType::RWStorageBuffer].count; }
	uint8_t GetRWTypedBufferCount()   const { return m_resourceOffsets[cr3d::ShaderResourceType::RWTypedBuffer].count; }

	uint8_t GetConstantBufferCount(cr3d::ShaderStage::T shaderStage)  const { return m_stageResourceOffsets[cr3d::ShaderResourceType::ConstantBuffer][GetStageIndex(shaderStage)].count; }
	uint8_t GetSamplerCount(cr3d::ShaderStage::T shaderStage)         const { return m_stageResourceOffsets[cr3d::ShaderResourceType::Sampler][GetStageIndex(shaderStage)].count; }
	uint8_t GetTextureCount(cr3d::ShaderStage::T shaderStage)         const { return m_stageResourceOffsets[cr3d::ShaderResourceType::Texture][GetStageIndex(shaderStage)].count; }
	uint8_t GetRWTextureCount(cr3d::ShaderStage::T shaderStage)       const { return m_stageResourceOffsets[cr3d::ShaderResourceType::RWTexture][GetStageIndex(shaderStage)].count; }
	uint8_t GetStorageBufferCount(cr3d::ShaderStage::T shaderStage)   const { return m_stageResourceOffsets[cr3d::ShaderResourceType::StorageBuffer][GetStageIndex(shaderStage)].count; }
	uint8_t GetRWStorageBufferCount(cr3d::ShaderStage::T shaderStage) const { return m_stageResourceOffsets[cr3d::ShaderResourceType::RWStorageBuffer][GetStageIndex(shaderStage)].count; }
	uint8_t GetRWTypedBufferCount(cr3d::ShaderStage::T shaderStage)   const { return m_stageResourceOffsets[cr3d::ShaderResourceType::RWTypedBuffer][GetStageIndex(shaderStage)].count; }

private:

	template<typename ResourcesT>
	void ProcessResourceArray(cr3d::ShaderResourceType::T resourceType, const ResourcesT& resources);

	static uint32_t GetStageIndex(cr3d::ShaderStage::T stage)
	{
		switch (stage)
		{
			case cr3d::ShaderStage::Compute: return 0;
			default: return (uint32_t)stage;
		}
	}

	// Total number of resources
	uint8_t m_totalResourceCount = 0;

	// Number of resources and offset per resource type
	CrArray<ShaderResourceOffset, cr3d::ShaderResourceType::Count> m_resourceOffsets = {};

	// This might seem a little wasteful but it can address the array below at a much smaller memory cost
	// than trying to pack resources indexed by stage count directly. There is a size and an offset for 
	// every resource type, for every stage. Some resource stages overlap, such as compute and graphics, 
	// so we don't take up unnecessary space
	CrArray<CrArray<ShaderResourceOffset, cr3d::ShaderStage::GraphicsStageCount>, cr3d::ShaderResourceType::Count> m_stageResourceOffsets = {};

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
class CrShaderBytecode : public CrIntrusivePtrInterface
{
public:

	CrShaderBytecode() {}

	const CrVector<uint8_t>& GetBytecode() const
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

	CrVector<CrShaderBytecodeHandle> m_bytecodes;
};

class ICrShader : public CrIntrusivePtrInterface
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

	const CrVector<CrShaderBytecodeHandle>& GetBytecodes() const
	{
		return m_bytecodes;
	}

	const CrInputSignature& GetInputSignature() const
	{
		return m_inputSignature;
	}

protected:

	CrInputSignature m_inputSignature;

	CrVector<CrShaderBytecodeHandle> m_bytecodes;
};

struct CrComputeShaderDescriptor
{
	CrShaderDebugString m_debugName;

	CrShaderBytecodeHandle m_bytecode;
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

	const CrShaderBytecodeHandle& GetBytecode() const
	{
		return m_bytecode;
	}

	virtual ~ICrComputeShader() {}

private:

	CrShaderBytecodeHandle m_bytecode;
};

struct CrShaderBytecodeCompilationDescriptor
{
	CrShaderBytecodeCompilationDescriptor
	(const CrFixedPath& path, const CrFixedString128& entryPoint, cr3d::ShaderStage::T stage, cr3d::GraphicsApi::T graphicsApi, cr::Platform::T platform)
		: path(path), entryPoint(entryPoint), stage(stage), graphicsApi(graphicsApi), platform(platform) {}

	const CrFixedPath               path;
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
