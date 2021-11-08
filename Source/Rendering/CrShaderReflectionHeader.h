#pragma once

#include "Core/String/CrString.h"
#include "Core/Containers/CrVector.h"

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
	uint32_t bytecodeOffset; // Offset into the bytecode to mangle the bind point (needed for Vulkan)
	CrString name;
};

template<typename StreamT>
StreamT& operator << (StreamT& stream, CrShaderReflectionResource& resource)
{
	stream << resource.bindPoint;
	stream << resource.type;
	stream << resource.bytecodeOffset;
	stream << resource.name;
	return stream;
}

struct CrShaderInterfaceVariable
{
	uint8_t bindPoint;
	cr3d::ShaderInterfaceBuiltinType::T type;
	CrString name;
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
	CrShaderReflectionVersion::T version = CrShaderReflectionVersion::CurrentVersion;

	CrString entryPoint;
	cr3d::ShaderStage::T shaderStage;
	CrVector<CrShaderReflectionResource> resources;
	CrVector<CrShaderInterfaceVariable> stageInputs;
	CrVector<CrShaderInterfaceVariable> stageOutputs;

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

	stream << reflectionHeader.resources;
	stream << reflectionHeader.stageInputs;
	stream << reflectionHeader.stageOutputs;

	stream << reflectionHeader.threadGroupSizeX;
	stream << reflectionHeader.threadGroupSizeY;
	stream << reflectionHeader.threadGroupSizeZ;

	return stream;
}