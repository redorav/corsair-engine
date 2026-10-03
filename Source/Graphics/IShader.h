#pragma once

#include "Core/FileSystem/CrFixedPath.h"

#include "Core/CrHash.h"
#include "Core/Streams/CrFileStream.h"

#include "Graphics/VertexDescriptor.h"
#include "Graphics/CrShaderReflectionHeader.h"

#include "Core/CrCoreForwardDeclarations.h"
#include "Graphics/CrGraphicsForwardDeclarations.h"

#include "crstl/fixed_string.h"
#include "crstl/fixed_vector.h"
#include "crstl/intrusive_ptr.h"
#include "crstl/unique_ptr.h"
#include "crstl/span.h"

namespace crgfx
{
	using bindpoint_t = uint8_t;

	struct ShaderBinding
	{
		ShaderBinding() {}

		ShaderBinding(bindpoint_t bindPoint, crgfx::ShaderStage::T stage, ConstantBuffers::T constantBufferID)
			: bindPoint(bindPoint), stage((uint8_t)stage), type(crgfx::ShaderResourceType::ConstantBuffer), constantBufferID(constantBufferID) {
		}

		ShaderBinding(bindpoint_t bindPoint, crgfx::ShaderStage::T stage, Samplers::T samplerID)
			: bindPoint(bindPoint), stage((uint8_t)stage), type(crgfx::ShaderResourceType::Sampler), samplerID(samplerID) {
		}

		ShaderBinding(bindpoint_t bindPoint, crgfx::ShaderStage::T stage, Textures::T textureID)
			: bindPoint(bindPoint), stage((uint8_t)stage), type(crgfx::ShaderResourceType::Texture), textureID(textureID) {
		}

		ShaderBinding(bindpoint_t bindPoint, crgfx::ShaderStage::T stage, RWTextures::T rwTextureID)
			: bindPoint(bindPoint), stage((uint8_t)stage), type(crgfx::ShaderResourceType::RWTexture), rwTextureID(rwTextureID) {
		}

		ShaderBinding(bindpoint_t bindPoint, crgfx::ShaderStage::T stage, StorageBuffers::T storageBufferID)
			: bindPoint(bindPoint), stage((uint8_t)stage), type(crgfx::ShaderResourceType::StorageBuffer), storageBufferID(storageBufferID) {
		}

		ShaderBinding(bindpoint_t bindPoint, crgfx::ShaderStage::T stage, RWStorageBuffers::T rwStorageBufferID)
			: bindPoint(bindPoint), stage((uint8_t)stage), type(crgfx::ShaderResourceType::RWStorageBuffer), rwStorageBufferID(rwStorageBufferID) {
		}

		ShaderBinding(bindpoint_t bindPoint, crgfx::ShaderStage::T stage, RWTypedBuffers::T rwTypedBufferID)
			: bindPoint(bindPoint), stage((uint8_t)stage), type(crgfx::ShaderResourceType::RWTypedBuffer), rwTypedBufferID(rwTypedBufferID) {
		}

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

	static_assert(crgfx::ShaderStage::Count < 16);
	static_assert(crgfx::ShaderResourceType::Count < 16);

	// We use this table to bucket resources before passing
	// them on (sorted) to the shader binding layout.
	struct ShaderBindingLayoutResources
	{
		crstl::fixed_vector<ShaderBinding, 64> constantBuffers;
		crstl::fixed_vector<ShaderBinding, 16> samplers;
		crstl::fixed_vector<ShaderBinding, 64> textures;
		crstl::fixed_vector<ShaderBinding, 32> rwTextures;

		crstl::fixed_vector<ShaderBinding, 32> storageBuffers;
		crstl::fixed_vector<ShaderBinding, 32> rwStorageBuffers;
		crstl::fixed_vector<ShaderBinding, 32> typedBuffers;
		crstl::fixed_vector<ShaderBinding, 32> rwTypedBuffers;
	};

	// Represents the resources needed by every stage. It is designed to be fast to iterate in order to rebuild tables quickly. Resources are sorted
	// according to type. This is not a resource table, just the binding points, owned by the shader. It also doesn't contain any names, but points to the 
	// builtins that the engine knows about
	class ShaderBindingLayout
	{
	public:

		ShaderBindingLayout() {}

		ShaderBindingLayout(const ShaderBindingLayoutResources& resources);

		struct ShaderResourceOffset
		{
			uint8_t offset;
			uint8_t count;
		};

		template<typename FunctionT, typename ResourceID>
		void ForEachResourceType(crgfx::ShaderResourceType::T resourceType, const FunctionT& function) const
		{
			const ShaderResourceOffset& resourceOffset = m_resourceOffsets[resourceType];
			for (uint8_t i = resourceOffset.offset; i < resourceOffset.offset + resourceOffset.count; ++i)
			{
				function((crgfx::ShaderStage::T)m_bindings[i].stage, (ResourceID)m_bindings[i].id, m_bindings[i].bindPoint);
			}
		}

		template<typename FunctionT, typename ResourceID>
		void ForEachResourceType(crgfx::ShaderStage::T shaderStage, crgfx::ShaderResourceType::T resourceType, const FunctionT& function) const
		{
			const ShaderResourceOffset& resourceOffset = m_stageResourceOffsets[resourceType][GetStageIndex(shaderStage)];
			for (uint8_t i = resourceOffset.offset; i < resourceOffset.offset + resourceOffset.count; ++i)
			{
				function((crgfx::ShaderStage::T)m_bindings[i].stage, (ResourceID)m_bindings[i].id, m_bindings[i].bindPoint);
			}
		}

		template<typename FunctionT>
		void ForEachConstantBuffer(const FunctionT& function) const
		{
			ForEachResourceType<FunctionT, ConstantBuffers::T>(crgfx::ShaderResourceType::ConstantBuffer, function);
		}

		template<typename FunctionT>
		void ForEachConstantBuffer(crgfx::ShaderStage::T shaderStage, const FunctionT& function) const
		{
			ForEachResourceType<FunctionT, ConstantBuffers::T>(shaderStage, crgfx::ShaderResourceType::ConstantBuffer, function);
		}

		template<typename FunctionT>
		void ForEachSampler(const FunctionT& function) const
		{
			ForEachResourceType<FunctionT, Samplers::T>(crgfx::ShaderResourceType::Sampler, function);
		}

		template<typename FunctionT>
		void ForEachSampler(crgfx::ShaderStage::T shaderStage, const FunctionT& function) const
		{
			ForEachResourceType<FunctionT, Samplers::T>(shaderStage, crgfx::ShaderResourceType::Sampler, function);
		}

		template<typename FunctionT>
		void ForEachTexture(const FunctionT& function) const
		{
			ForEachResourceType<FunctionT, Textures::T>(crgfx::ShaderResourceType::Texture, function);
		}

		template<typename FunctionT>
		void ForEachTexture(crgfx::ShaderStage::T shaderStage, const FunctionT& function) const
		{
			ForEachResourceType<FunctionT, Textures::T>(shaderStage, crgfx::ShaderResourceType::Texture, function);
		}

		template<typename FunctionT>
		void ForEachRWTexture(const FunctionT& function) const
		{
			ForEachResourceType<FunctionT, RWTextures::T>(crgfx::ShaderResourceType::RWTexture, function);
		}

		template<typename FunctionT>
		void ForEachRWTexture(crgfx::ShaderStage::T shaderStage, const FunctionT& function) const
		{
			ForEachResourceType<FunctionT, RWTextures::T>(shaderStage, crgfx::ShaderResourceType::RWTexture, function);
		}

		template<typename FunctionT>
		void ForEachStorageBuffer(const FunctionT& function) const
		{
			ForEachResourceType<FunctionT, StorageBuffers::T>(crgfx::ShaderResourceType::StorageBuffer, function);
		}

		template<typename FunctionT>
		void ForEachStorageBuffer(crgfx::ShaderStage::T shaderStage, const FunctionT& function) const
		{
			ForEachResourceType<FunctionT, StorageBuffers::T>(shaderStage, crgfx::ShaderResourceType::StorageBuffer, function);
		}

		template<typename FunctionT>
		void ForEachRWStorageBuffer(const FunctionT& function) const
		{
			ForEachResourceType<FunctionT, RWStorageBuffers::T>(crgfx::ShaderResourceType::RWStorageBuffer, function);
		}

		template<typename FunctionT>
		void ForEachRWStorageBuffer(crgfx::ShaderStage::T shaderStage, const FunctionT& function) const
		{
			ForEachResourceType<FunctionT, RWStorageBuffers::T>(shaderStage, crgfx::ShaderResourceType::RWStorageBuffer, function);
		}

		template<typename FunctionT>
		void ForEachRWTypedBuffer(const FunctionT& function) const
		{
			ForEachResourceType<FunctionT, RWTypedBuffers::T>(crgfx::ShaderResourceType::RWTypedBuffer, function);
		}

		template<typename FunctionT>
		void ForEachRWTypedBuffer(crgfx::ShaderStage::T shaderStage, const FunctionT& function) const
		{
			ForEachResourceType<FunctionT, RWTypedBuffers::T>(shaderStage, crgfx::ShaderResourceType::RWTypedBuffer, function);
		}

		template<typename FunctionT>
		static void AddResources(const CrShaderReflectionHeader& reflectionHeader, ShaderBindingLayoutResources& resources, const FunctionT& function);

		uint8_t GetTotalResourceCount() const { return m_totalResourceCount; }

		uint8_t GetConstantBufferCount()  const { return m_resourceOffsets[crgfx::ShaderResourceType::ConstantBuffer].count; }
		uint8_t GetSamplerCount()         const { return m_resourceOffsets[crgfx::ShaderResourceType::Sampler].count; }
		uint8_t GetTextureCount()         const { return m_resourceOffsets[crgfx::ShaderResourceType::Texture].count; }
		uint8_t GetRWTextureCount()       const { return m_resourceOffsets[crgfx::ShaderResourceType::RWTexture].count; }
		uint8_t GetStorageBufferCount()   const { return m_resourceOffsets[crgfx::ShaderResourceType::StorageBuffer].count; }
		uint8_t GetRWStorageBufferCount() const { return m_resourceOffsets[crgfx::ShaderResourceType::RWStorageBuffer].count; }
		uint8_t GetRWTypedBufferCount()   const { return m_resourceOffsets[crgfx::ShaderResourceType::RWTypedBuffer].count; }

		uint8_t GetConstantBufferCount(crgfx::ShaderStage::T shaderStage)  const { return m_stageResourceOffsets[crgfx::ShaderResourceType::ConstantBuffer][GetStageIndex(shaderStage)].count; }
		uint8_t GetSamplerCount(crgfx::ShaderStage::T shaderStage)         const { return m_stageResourceOffsets[crgfx::ShaderResourceType::Sampler][GetStageIndex(shaderStage)].count; }
		uint8_t GetTextureCount(crgfx::ShaderStage::T shaderStage)         const { return m_stageResourceOffsets[crgfx::ShaderResourceType::Texture][GetStageIndex(shaderStage)].count; }
		uint8_t GetRWTextureCount(crgfx::ShaderStage::T shaderStage)       const { return m_stageResourceOffsets[crgfx::ShaderResourceType::RWTexture][GetStageIndex(shaderStage)].count; }
		uint8_t GetStorageBufferCount(crgfx::ShaderStage::T shaderStage)   const { return m_stageResourceOffsets[crgfx::ShaderResourceType::StorageBuffer][GetStageIndex(shaderStage)].count; }
		uint8_t GetRWStorageBufferCount(crgfx::ShaderStage::T shaderStage) const { return m_stageResourceOffsets[crgfx::ShaderResourceType::RWStorageBuffer][GetStageIndex(shaderStage)].count; }
		uint8_t GetRWTypedBufferCount(crgfx::ShaderStage::T shaderStage)   const { return m_stageResourceOffsets[crgfx::ShaderResourceType::RWTypedBuffer][GetStageIndex(shaderStage)].count; }

	private:

		template<typename ResourcesT>
		void ProcessResourceArray(crgfx::ShaderResourceType::T resourceType, const ResourcesT& resources);

		static uint32_t GetStageIndex(crgfx::ShaderStage::T stage)
		{
			switch (stage)
			{
				case crgfx::ShaderStage::Compute: return 0;
				default: return (uint32_t)stage;
			}
		}

		// Total number of resources
		uint8_t m_totalResourceCount = 0;

		// Number of resources and offset per resource type
		crstl::array<ShaderResourceOffset, crgfx::ShaderResourceType::Count> m_resourceOffsets = {};

		// This might seem a little wasteful but it can address the array below at a much smaller memory cost
		// than trying to pack resources indexed by stage count directly. There is a size and an offset for 
		// every resource type, for every stage. Some resource stages overlap, such as compute and graphics, 
		// so we don't take up unnecessary space

		// TODO GraphicsStageCount means too much these days and this array grows if we add more stages. How to improve this
		crstl::array<crstl::array<ShaderResourceOffset, crgfx::ShaderStage::GraphicsStageCount>, crgfx::ShaderResourceType::Count> m_stageResourceOffsets = {};

		crstl::fixed_vector<ShaderBinding, 64> m_bindings;
	};

	struct VertexInput
	{
		uint16_t semantic : 5; // CrVertexSemantic::T
		uint16_t components : 3;
	};

	static_assert(sizeof(VertexInput) == 2, "CrVertexInput size mismatch");

	struct InputSignature
	{
		crstl::fixed_vector<VertexInput, crgfx::MaxVertexStreams> inputs;
	};

	// Bytecode represents shader code from a particular stage, e.g. vertex, pixel, etc
	// TODO This duplicates the bytecode contained in the executable. We need a way to make this class
	// both own bytecode and point to existing bytecode to avoid duplication
	class ShaderBytecode : public crstl::intrusive_ptr_interface_delete
	{
	public:

		ShaderBytecode() {}

		const crstl::span<uint8_t> GetBytecode() const
		{
			return m_bytecode;
		}

		const crstl::string& GetEntryPoint() const
		{
			return m_reflection.entryPoint;
		}

		crgfx::ShaderStage::T GetShaderStage() const
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
		friend StreamT& operator << (StreamT& stream, ShaderBytecode& bytecode);

	private:

		crstl::vector<uint8_t> m_bytecode;
		CrShaderReflectionHeader m_reflection;
		CrHash m_hash; // A hash of the entire bytecode
	};

	template<typename StreamT>
	StreamT& operator << (StreamT& stream, ShaderBytecode& bytecode)
	{
		stream << bytecode.m_reflection;
		stream << bytecode.m_bytecode;
		if (stream.IsReading())
		{
			bytecode.ComputeHash();
		}
		return stream;
	}

	typedef crstl::fixed_string64 ShaderDebugString;

	class GraphicsShaderBytecode
	{
	public:

		GraphicsShaderBytecode() {}

		GraphicsShaderBytecode(const ShaderDebugString& debugName)
		{
			m_debugName = debugName;
		}

		const crstl::span<const ShaderBytecodeHandle> GetBytecodes() const
		{
			return crstl::span<const ShaderBytecodeHandle>(m_bytecodes.data(), m_bytecodes.size());
		}

		void AddBytecode(const ShaderBytecodeHandle& bytecode)
		{
			m_bytecodes.push_back(bytecode);
			m_hash << bytecode->GetHash();
		}

		CrHash GetHash() const
		{
			return m_hash;
		}

		const char* GetDebugName() const
		{
			return m_debugName.c_str();
		}

	protected:

		ShaderDebugString m_debugName;

		crstl::fixed_vector<ShaderBytecodeHandle, crgfx::ShaderStage::GraphicsStageCount> m_bytecodes;

		// Hash produced from the bytecodes belonging to this shader
		CrHash m_hash;
	};

	class ComputeShaderBytecode
	{
	public:

		ComputeShaderBytecode(const ShaderDebugString& debugName, const ShaderBytecodeHandle& bytecode)
		{
			m_debugName = debugName;
			m_bytecode = bytecode;
		}

		CrHash GetHash() const
		{
			return m_bytecode->GetHash();
		}

		const char* GetDebugName() const
		{
			return m_debugName.c_str();
		}

		const ShaderBytecodeHandle& GetBytecode() const
		{
			return m_bytecode;
		}

	private:

		ShaderDebugString m_debugName;

		ShaderBytecodeHandle m_bytecode;
	};
};

// TODO Move out of IShader

struct CrShaderBytecodeCompilationDescriptor
{
	CrShaderBytecodeCompilationDescriptor
	(const CrFixedPath& path, const crstl::fixed_string128& entryPoint, crgfx::ShaderStage::T stage, crgfx::GraphicsApi::T graphicsApi, cr::Platform::T platform)
		: path(path), entryPoint(entryPoint), stage(stage), graphicsApi(graphicsApi), platform(platform) {
	}

	const CrFixedPath               path;
	const crstl::fixed_string128    entryPoint;
	const crgfx::ShaderStage::T     stage;
	const crgfx::GraphicsApi::T     graphicsApi;
	const cr::Platform::T           platform;
};

// Add defines to a shader compilation
struct CrShaderCompilerDefines
{
	void AddDefine(const crstl::string& define)
	{
		defines.push_back(define);
	}

	const crstl::vector<crstl::string>& GetDefines() const
	{
		return defines;
	}

	static CrShaderCompilerDefines Dummy;

private:

	crstl::vector<crstl::string> defines;
};

struct CrShaderCompilationDescriptor
{
	void AddBytecodeDescriptor(const CrShaderBytecodeCompilationDescriptor& bytecodeDescriptor)
	{
		m_stageBytecodeDescriptors.push_back(bytecodeDescriptor);
	}

	void AddDefine(const crstl::string& define)
	{
		m_defines.AddDefine(define);
	}

	const crstl::vector<CrShaderBytecodeCompilationDescriptor>& GetBytecodeDescriptors() const
	{
		return m_stageBytecodeDescriptors;
	}

	const CrShaderCompilerDefines& GetDefines() const
	{
		return m_defines;
	}

private:

	crstl::vector<CrShaderBytecodeCompilationDescriptor> m_stageBytecodeDescriptors;
	CrShaderCompilerDefines m_defines;
};