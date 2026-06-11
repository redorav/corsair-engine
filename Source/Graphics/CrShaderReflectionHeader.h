#pragma once

#include "crstl/string.h"
#include "crstl/vector.h"

namespace CrShaderReflectionVersion
{
	enum T
	{
		InitialVersion,
		CurrentVersion = InitialVersion
	};
};

struct CrShaderReflectionResource
{
	uint8_t bindPoint;
	cr3d::ShaderResourceType::T type;
	crstl::string name;
};

template<typename StreamT>
StreamT& operator << (StreamT& stream, CrShaderReflectionResource& resource)
{
	stream << resource.bindPoint;
	stream << resource.type;
	stream << resource.name;
	return stream;
}

struct CrShaderInterfaceVariable
{
	uint8_t bindPoint;
	cr3d::ShaderInterfaceBuiltinType::T type;
	crstl::string name;
};

template<typename StreamT>
StreamT& operator << (StreamT& stream, CrShaderInterfaceVariable& resource)
{
	stream << resource.bindPoint;
	stream << resource.type;
	stream << resource.name;
	return stream;
}

struct CrShaderReflectionHeader
{
	template<typename FunctionT>
	void ForEachResource(const FunctionT& function)
	{
		for (CrShaderReflectionResource& resource : constantBuffers)
		{
			function(resource);
		}

		for (CrShaderReflectionResource& resource : samplers)
		{
			function(resource);
		}

		for (CrShaderReflectionResource& resource : textures)
		{
			function(resource);
		}

		for (CrShaderReflectionResource& resource : rwTextures)
		{
			function(resource);
		}

		for (CrShaderReflectionResource& resource : storageBuffers)
		{
			function(resource);
		}

		for (CrShaderReflectionResource& resource : rwStorageBuffers)
		{
			function(resource);
		}

		for (CrShaderReflectionResource& resource : rwTypedBuffers)
		{
			function(resource);
		}
	}

	CrShaderReflectionVersion::T version = CrShaderReflectionVersion::CurrentVersion;

	crstl::string entryPoint;
	cr3d::ShaderStage::T shaderStage = cr3d::ShaderStage::Count;
	uint64_t bytecodeHash = (uint64_t)-1;
	
	crstl::vector<CrShaderReflectionResource> constantBuffers;
	crstl::vector<CrShaderReflectionResource> samplers;
	crstl::vector<CrShaderReflectionResource> textures;
	crstl::vector<CrShaderReflectionResource> rwTextures;
	crstl::vector<CrShaderReflectionResource> storageBuffers;
	crstl::vector<CrShaderReflectionResource> rwStorageBuffers;
	crstl::vector<CrShaderReflectionResource> rwTypedBuffers;

	crstl::vector<CrShaderInterfaceVariable> stageInputs;
	crstl::vector<CrShaderInterfaceVariable> stageOutputs;

	uint32_t threadGroupSizeX = 0;
	uint32_t threadGroupSizeY = 0;
	uint32_t threadGroupSizeZ = 0;
};

template<typename StreamT>
StreamT& operator << (StreamT& stream, CrShaderReflectionHeader& reflectionHeader)
{
	stream << reflectionHeader.version;
	stream << reflectionHeader.entryPoint;
	stream << reflectionHeader.shaderStage;
	stream << reflectionHeader.bytecodeHash;

	stream << reflectionHeader.constantBuffers;
	stream << reflectionHeader.samplers;
	stream << reflectionHeader.textures;
	stream << reflectionHeader.rwTextures;
	stream << reflectionHeader.storageBuffers;
	stream << reflectionHeader.rwStorageBuffers;
	stream << reflectionHeader.rwTypedBuffers;

	stream << reflectionHeader.stageInputs;
	stream << reflectionHeader.stageOutputs;

	stream << reflectionHeader.threadGroupSizeX;
	stream << reflectionHeader.threadGroupSizeY;
	stream << reflectionHeader.threadGroupSizeZ;

	return stream;
}