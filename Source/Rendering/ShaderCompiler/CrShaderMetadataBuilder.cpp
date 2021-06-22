#include "CrShaderMetadataBuilder.h"

#include "CrShaderCompiler.h"
#include "CrCompilerGLSLANG.h"

#include "Core/FileSystem/CrFileSystem.h"

#include "Rendering/CrRendering.h"

#include <string>

// TODO Remove and use ICrFile
#include <fstream>
#include <sstream> 

#include "spirv_reflect.h"

static const std::string AutoGeneratedDisclaimer =
"//------------------------------------------------------------------------\n"
"// This file was autogenerated by the Corsair Engine Shader Compiler.\n"
"// Modifying it will have no effect after the file is regenerated.\n"
"//------------------------------------------------------------------------\n\n";

static const std::string ConstantBufferSection =
"//-----------------\n"
"// Constant Buffers\n"
"//-----------------\n\n";

static const std::string SamplerSection =
"//---------\n"
"// Samplers\n"
"//---------\n\n";

static const std::string TextureSection =
"//---------\n"
"// Textures\n"
"//---------\n\n";

static const std::string RWTextureSection =
"//------------\n"
"// RW Textures\n"
"//------------\n\n";

static const std::string StorageBufferSection =
"//----------------\n"
"// Storage Buffers\n"
"//----------------\n\n";

static const std::string RWStorageBufferSection =
"//-------------------\n"
"// RW Storage Buffers\n"
"//-------------------\n\n";

static const std::string DataBufferSection =
"//-------------\n"
"// Data Buffers\n"
"//-------------\n\n";

static const std::string RWDataBufferSection =
"//----------------\n"
"// RW Data Buffers\n"
"//----------------\n\n";

bool CrShaderMetadataBuilder::BuildMetadata(const CompilationDescriptor& compilationDescriptor)
{
	std::vector<uint32_t> spirvBytecode;
	bool compiled = CrCompilerGLSLANG::HLSLtoSPIRV(compilationDescriptor, spirvBytecode);

	if (!compiled)
	{
		return false;
	}

	std::string metadataHeader, metadataCpp;
	bool builtMetadata = BuildSPIRVMetadata(spirvBytecode, metadataHeader, metadataCpp);

	if (!builtMetadata)
	{
		return false;
	}

	// Create header and cpp filenames
	CrPath headerPath = compilationDescriptor.outputPath;
	WriteToFileIfChanged(headerPath.replace_extension("h").string(), metadataHeader);

	CrPath cppPath = compilationDescriptor.outputPath;
	WriteToFileIfChanged(cppPath.replace_extension("cpp").string(), metadataCpp);

	// Write dummy file that tells the build system dependency tracker that files are up to date
	CrPath uptodatePath = compilationDescriptor.outputPath;
	WriteToFile(uptodatePath.replace_extension("uptodate").string(), "");

	return true;
}

// This class translates GLSL resources to more HLSL-friendly resources. There are several
// odd things about the way GLSL treats buffers that doesn't map entirely to HLSL
struct HLSLResources
{
	HLSLResources(const SpvReflectShaderModule& reflection)
	{
		for (uint32_t i = 0; i < reflection.descriptor_binding_count; ++i)
		{
			const SpvReflectDescriptorBinding& binding = reflection.descriptor_bindings[i];

			switch(binding.descriptor_type)
			{
				case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				{
					constantBuffers.push_back(binding);
					break;
				}
				case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
				{
					textures.push_back(binding);
					break;
				}
				case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
				{
					samplers.push_back(binding);
					break;
				}
				case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
				{
					if (binding.resource_type == SPV_REFLECT_RESOURCE_FLAG_UAV)
					{
						rwStorageBuffers.push_back(binding);
					}
					else
					{
						storageBuffers.push_back(binding);
					}
					break;
				}
				case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
				{
					rwTextures.push_back(binding);
					break;
				}
				case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
				{
					dataBuffers.push_back(binding);
					break;
				}
				case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
				{
					rwDataBuffers.push_back(binding);
					break;
				}
				default:
				{
					printf("Unrecognized!");
				}
			}
		}
	}

	std::vector<SpvReflectDescriptorBinding> constantBuffers;
	std::vector<SpvReflectDescriptorBinding> samplers;
	std::vector<SpvReflectDescriptorBinding> textures;
	std::vector<SpvReflectDescriptorBinding> rwTextures;
	std::vector<SpvReflectDescriptorBinding> storageBuffers;
	std::vector<SpvReflectDescriptorBinding> rwStorageBuffers;
	std::vector<SpvReflectDescriptorBinding> dataBuffers;
	std::vector<SpvReflectDescriptorBinding> rwDataBuffers;

	//std::vector<SpvReflectDescriptorBinding> stageInputs;
	//std::vector<SpvReflectDescriptorBinding> stageOutputs;
	//std::vector<SpvReflectDescriptorBinding> subpassInputs;
	//std::vector<SpvReflectDescriptorBinding> atomicCounters;
	//std::vector<SpvReflectDescriptorBinding> accelerationStructures;
	//std::vector<SpvReflectDescriptorBinding> pushConstantBuffers;
};

bool CrShaderMetadataBuilder::BuildSPIRVMetadata(
	const std::vector<uint32_t>& spirvBytecode, 
	std::string& metadataHeader, std::string& metadataCpp)
{
	// This needs to be kept alive
	spv_reflect::ShaderModule spvReflectModule(spirvBytecode.size() * 4, spirvBytecode.data());

	if (spvReflectModule.GetResult() != SPV_REFLECT_RESULT_SUCCESS)
	{
		// TODO Quit with message (message gets piped to VS)
	}

	HLSLResources resources(spvReflectModule.GetShaderModule());

	//------------------
	// Build header file
	//------------------

	metadataHeader = AutoGeneratedDisclaimer;

	//metadataHeader += "namespace crshader\n{\n";

	metadataHeader += "#pragma once\n\n";

	// Maybe there is a better way of not writing these headers directly. They depend on the physical
	// structure of the code which can change
	metadataHeader += "#include \"Core/Containers/CrArray.h\"\n";
	metadataHeader += "#include \"Core/Containers/CrHashMap.h\"\n";
	metadataHeader += "#include \"Core/Containers/CrVector.h\"\n";
	metadataHeader += "#include \"Core/String/CrString.h\"\n";
	metadataHeader += "#include \"Rendering/CrRenderingVector.h\"\n";
	metadataHeader += "\n";

	metadataHeader += BuildConstantBufferMetadataHeader(resources);

	metadataHeader += BuildSamplerMetadataHeader(resources);

	metadataHeader += BuildTextureMetadataHeader(resources);

	metadataHeader += BuildRWTextureMetadataHeader(resources);

	metadataHeader += BuildStorageBufferMetadataHeader(resources);

	metadataHeader += BuildRWStorageBufferMetadataHeader(resources);

	metadataHeader += BuildDataBufferMetadataHeader(resources);

	metadataHeader += BuildRWDataBufferMetadataHeader(resources);

	//metadataHeader += "}\n";

	//---------------
	// Build cpp file
	//---------------

	metadataCpp = AutoGeneratedDisclaimer;
	
	// Maybe there is a better way of not writing these headers directly. They depend on the physical
	// structure of the code which can change
	metadataCpp += "#include \"CrRendering_pch.h\"\n";
	metadataCpp += "#include \"ShaderMetadata.h\"\n\n";
	metadataCpp += "\n";

	metadataCpp += BuildConstantBufferMetadataCpp(resources);

	metadataCpp += BuildSamplerMetadataCpp(resources);

	metadataCpp += BuildTextureMetadataCpp(resources);

	metadataCpp += BuildRWTextureMetadataCpp(resources);

	metadataCpp += BuildStorageBufferMetadataCpp(resources);

	metadataCpp += BuildRWStorageBufferMetadataCpp(resources);

	metadataCpp += BuildDataBufferMetadataCpp(resources);

	metadataCpp += BuildRWDataBufferMetadataCpp(resources);

	return true;
}

std::string CrShaderMetadataBuilder::BuildConstantBufferMetadataHeader(const HLSLResources& resources)
{
	std::string result;

	result += ConstantBufferSection;

	// 1. Print an enum with all the constant buffers, assigning them an index
	result += PrintResourceEnum("ConstantBuffer", resources.constantBuffers);

	// 2. For every constant buffer, print the data that defines it
	for (uint32_t uniformBufferIndex = 0; uniformBufferIndex < resources.constantBuffers.size(); ++uniformBufferIndex)
	{
		const SpvReflectDescriptorBinding& uniformBuffer = resources.constantBuffers[uniformBufferIndex];
		
		uint32_t indentationLevel = 0;

		std::string constantBufferName = uniformBuffer.type_description->type_name;

		for (uint32_t memberIndex = 0; memberIndex < uniformBuffer.type_description->member_count; ++memberIndex)
		{
			const SpvReflectTypeDescription& member = uniformBuffer.type_description->members[memberIndex];
			result += PrintMemberStruct(member, constantBufferName + "Data", "", indentationLevel);
		}

		result += PrintConstantBufferStructMetadata(constantBufferName, uniformBufferIndex);
	}

	result += PrintConstantBufferMetadataStructDeclaration();

	result += PrintResourceMetadataInstanceDeclaration("ConstantBuffer", resources.constantBuffers);

	//result += PrintConstantBufferGlobalGroupDeclaration(resources.constantBuffers);

	result += "extern CrHashMap<CrString, ConstantBufferMetadata&> ConstantBufferTable;\n\n";

	result += "extern CrArray<ConstantBufferMetadata, " + std::to_string(resources.constantBuffers.size()) + std::string("> ConstantBufferMetaTable;\n\n");

	return result;
}

std::string CrShaderMetadataBuilder::BuildConstantBufferMetadataCpp(const HLSLResources& resources)
{
	std::string result;

	result += ConstantBufferSection;

	result += PrintConstantBufferMetadataInstanceDefinition(resources.constantBuffers);

	result += PrintResourceHashmap("ConstantBuffer", resources.constantBuffers);

	return result;
}

std::string CrShaderMetadataBuilder::PrintResourceEnum(const std::string& resourceTypeName, const ResourceVector& resources)
{
	std::string result;
	result += "namespace " + resourceTypeName + "s\n{\n\tenum T : uint8_t\n\t{";

	for (uint32_t i = 0; i < resources.size(); ++i)
	{
		const SpvReflectDescriptorBinding& resource = resources[i];
		std::string resourceName = std::string(resource.name) == "" ? resource.type_description->type_name : resource.name;
		result += "\n\t\t" + resourceName + " = " + std::to_string(i) + ",";
	}

	result += "\n\t\tCount\n\t};\n};\n\n";
	return result;
}

std::string CrShaderMetadataBuilder::PrintResourceHashmap(const std::string& resourceTypeName, const ResourceVector& resources)
{
	std::string result;
	result += "CrHashMap<CrString, " + resourceTypeName + "Metadata&> " + resourceTypeName + "Table =\n{";

	for (const auto& resource : resources)
	{
		std::string resourceName = std::string(resource.name) == "" ? resource.type_description->type_name : resource.name;
		result += "\n\t{ \"" + resourceName + "\", " + resourceName + "MetaInstance },";
	}
	result += "\n};\n\n";

	result += "CrArray<" + resourceTypeName + "Metadata, " + std::to_string(resources.size()) + "> " + resourceTypeName + "MetaTable =\n{";

	for (const auto& resource : resources)
	{
		std::string resourceName = std::string(resource.name) == "" ? resource.type_description->type_name : resource.name;
		result += "\n\t" + resourceName + "MetaInstance,";
	}
	result += "\n};\n\n";

	return result;
}

std::string GetBuiltinTypeString(const SpvReflectTypeDescription& type)
{
	std::string result;

	bool isComplexType = type.traits.numeric.matrix.row_count > 0 || type.traits.numeric.vector.component_count > 0;

	if (isComplexType)
	{
		result += "cr3d::";
	}

	if (type.type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT)
	{
		result += "float";
	}
	else if (type.type_flags & SPV_REFLECT_TYPE_FLAG_INT)
	{
		result += "int";
	}
	else if (type.type_flags & SPV_REFLECT_TYPE_FLAG_BOOL)
	{
		result += "bool";
	}

	if (type.traits.numeric.matrix.row_count > 0)
	{
		result += std::to_string(type.traits.numeric.matrix.row_count) + "x" + std::to_string(type.traits.numeric.matrix.column_count);
	}
	else if (type.traits.numeric.vector.component_count > 0)
	{
		result += std::to_string(type.traits.numeric.vector.component_count);
	}

	return result;
}

std::string CrShaderMetadataBuilder::PrintMemberBuiltIn(const SpvReflectTypeDescription& type, const std::string& memberName, const std::string& indentation)
{
	std::string result;

	result += indentation;

	result += GetBuiltinTypeString(type);

	result += " " + memberName;

	// Add array dimensions
	for(uint32_t i = 0; i < type.traits.array.dims_count; ++i)
	{
		result += "[" + std::to_string(type.traits.array.stride) + "]";
	}

	result += ";\n";
	return result;
}

std::string CrShaderMetadataBuilder::PrintMemberStruct(const SpvReflectTypeDescription& type, const std::string& structTypeName, const std::string& structName, uint32_t indentationLevel)
{
	std::string result;
	std::string structIndentation;

	for (uint32_t i = 0; i < indentationLevel; ++i)
	{
		structIndentation += "\t";
	}

	result += structIndentation + "struct " + structTypeName + "\n" + structIndentation + "{\n"; // Open the struct

	std::string memberIndentation = structIndentation + "\t";

	for (uint32_t i = 0; i < type.member_count; ++i)
	{
		const SpvReflectTypeDescription& member = type.members[i];

		switch (member.op)
		{
			case SpvOpTypeStruct:
			{
				result += PrintMemberStruct(member, member.type_name, member.struct_member_name, indentationLevel + 1);
				break;
			}
			default:
			{
				result += PrintMemberBuiltIn(member, member.struct_member_name, memberIndentation);
				break;
			}
		}
	}

	result += structIndentation + "};\n\n"; // Close the struct

	if (structName != "")
	{
		result += structIndentation + structTypeName + " " + structName + ";\n"; // Declare a variable of the struct with a given name
	}

	return result;
}

std::string CrShaderMetadataBuilder::PrintResourceMetadataInstanceDeclaration(const std::string& resourceType, const ResourceVector& resources)
{
	std::string result;
	for (const SpvReflectDescriptorBinding& resource : resources)
	{
		std::string resourceName = std::string(resource.name) == "" ? resource.type_description->type_name : resource.name;
		result += "extern " + resourceType + "Metadata " + resourceName + "MetaInstance;\n";
	}

	result += "extern " + resourceType + "Metadata Invalid" + resourceType + "MetaInstance;\n";
	return result + "\n";
}

std::string CrShaderMetadataBuilder::PrintConstantBufferStructMetadata(const std::string& name, int index)
{
	std::string result;
	result += "struct " + name + " : public " + name + "Data\n{\n";
	result += "\tusing Data = " + name + "Data;\n";
	result += "\tenum { size = sizeof(" + name + "Data) };\n";
	result += "\tenum { index = " + std::to_string(index) + " };\n";
	result += "};\n\n";
	return result;
}

std::string CrShaderMetadataBuilder::PrintConstantBufferMetadataStructDeclaration()
{
	std::string result = "struct ConstantBufferMetadata" \
		"\n{\n" \
		"\tConstantBufferMetadata(uint32_t id, uint32_t size) : id(static_cast<ConstantBuffers::T>(id)), size(size) {}\n" \
		"\tconst ConstantBuffers::T id;\n" \
		"\tconst uint32_t size;\n" \
		"};\n\n";
	return result;
}

std::string CrShaderMetadataBuilder::PrintConstantBufferMetadataInstanceDefinition(const ResourceVector& uniformBuffers)
{
	std::string result;
	for (const SpvReflectDescriptorBinding& uniformBuffer : uniformBuffers)
	{
		std::string name = std::string(uniformBuffer.name) == "" ? uniformBuffer.type_description->type_name : uniformBuffer.name;
		auto scopedMeta = [&](const std::string& member) { return name + "::" + member; }; // Convenience lambda
		result += "ConstantBufferMetadata " + name + "MetaInstance(" + scopedMeta("index, ") + scopedMeta("size") + ");\n";
	}
	result += "ConstantBufferMetadata InvalidConstantBufferMetaInstance(UINT32_MAX, 0);\n";
	return result + "\n";
}

std::string CrShaderMetadataBuilder::BuildTextureMetadataHeader(const HLSLResources& resources)
{
	std::string result;

	result += TextureSection;

	result += PrintResourceEnum("Texture", resources.textures);

	result += PrintTextureMetadataStructDeclaration();

	result += PrintResourceMetadataInstanceDeclaration("Texture", resources.textures);

	result += "extern CrHashMap<CrString, TextureMetadata&> TextureTable;\n\n";

	result += "extern CrArray<TextureMetadata, " + std::to_string(resources.textures.size()) + "> TextureMetaTable;\n\n";

	return result;
}

std::string CrShaderMetadataBuilder::BuildTextureMetadataCpp(const HLSLResources& resources)
{
	std::string result;

	result += TextureSection;

	result += PrintTextureMetadataInstanceDefinition(resources.textures);

	result += PrintResourceHashmap("Texture", resources.textures);

	return result;
}

std::string CrShaderMetadataBuilder::PrintTextureMetadataStructDeclaration()
{
	std::string result = "struct TextureMetadata" \
		"\n{\n" \
		"\tTextureMetadata(uint32_t id) : id(static_cast<Textures::T>(id)) {}\n" \
		"\tconst Textures::T id;\n" \
		"};\n\n";
	return result;
}

std::string CrShaderMetadataBuilder::PrintTextureMetadataInstanceDefinition(const ResourceVector& textures)
{
	std::string result;
	for (const auto& texture : textures)
	{
		result += "TextureMetadata " + std::string(texture.name) + "MetaInstance(" + "Textures::" + std::string(texture.name) + ");\n";
	}
	result += "TextureMetadata InvalidTextureMetaInstance(UINT32_MAX);\n";
	return result + "\n";
}

std::string CrShaderMetadataBuilder::BuildSamplerMetadataHeader(const HLSLResources& resources)
{
	std::string result;

	result += SamplerSection;

	result += PrintResourceEnum("Sampler", resources.samplers);

	result += PrintSamplerMetadataStructDeclaration();

	result += PrintResourceMetadataInstanceDeclaration("Sampler", resources.samplers);

	result += "extern CrHashMap<CrString, SamplerMetadata&> SamplerTable;\n\n";

	result += "extern CrArray<SamplerMetadata, " + std::to_string(resources.samplers.size()) + "> SamplerMetaTable;\n\n";

	return result;
}

std::string CrShaderMetadataBuilder::BuildSamplerMetadataCpp(const HLSLResources& resources)
{
	std::string result;

	result += SamplerSection;

	result += PrintSamplerMetadataInstanceDefinition(resources.samplers);

	result += PrintResourceHashmap("Sampler", resources.samplers);

	return result;
}

std::string CrShaderMetadataBuilder::PrintSamplerMetadataStructDeclaration()
{
	std::string result = "struct SamplerMetadata" \
		"\n{\n" \
		"\tSamplerMetadata(uint32_t id) : id(static_cast<Samplers::T>(id)) {}\n" \
		"\tconst Samplers::T id;\n" \
		"};\n\n";
	return result;
}

std::string CrShaderMetadataBuilder::PrintSamplerMetadataInstanceDefinition(const ResourceVector& samplers)
{
	std::string result;
	for (const auto& sampler : samplers) { result += "SamplerMetadata " + std::string(sampler.name) + "MetaInstance(" + "Samplers::" + std::string(sampler.name) + ");\n"; }
	result += "SamplerMetadata InvalidSamplerMetaInstance(UINT32_MAX);\n";
	return result + "\n";
}

std::string CrShaderMetadataBuilder::BuildRWTextureMetadataHeader(const HLSLResources& resources)
{
	std::string result;

	result += RWTextureSection;

	result += PrintResourceEnum("RWTexture", resources.rwTextures);

	result += PrintRWTextureMetadataStructDeclaration();

	result += PrintResourceMetadataInstanceDeclaration("RWTexture", resources.rwTextures);

	result += "extern CrHashMap<CrString, RWTextureMetadata&> RWTextureTable;\n\n";

	result += "extern CrArray<RWTextureMetadata, " + std::to_string(resources.rwTextures.size()) + "> RWTextureMetaTable;\n\n";

	return result;
}

std::string CrShaderMetadataBuilder::BuildRWTextureMetadataCpp(const HLSLResources& resources)
{
	std::string result;

	result += RWTextureSection;

	result += PrintRWTextureMetadataInstanceDefinition(resources.rwTextures);

	result += PrintResourceHashmap("RWTexture", resources.rwTextures);

	return result;
}

std::string CrShaderMetadataBuilder::PrintRWTextureMetadataInstanceDefinition(const ResourceVector& rwTextures)
{
	std::string result;
	for (const auto& rwTexture : rwTextures)
	{
		result += "RWTextureMetadata " + std::string(rwTexture.name) + "MetaInstance(" + "RWTextures::" + std::string(rwTexture.name) + ");\n";
	}
	result += "RWTextureMetadata InvalidRWTextureMetaInstance(UINT32_MAX);\n";
	return result + "\n";
}

std::string CrShaderMetadataBuilder::PrintRWTextureMetadataStructDeclaration()
{
	std::string result = "struct RWTextureMetadata" \
		"\n{\n" \
		"\tRWTextureMetadata(uint32_t id) : id(static_cast<RWTextures::T>(id)) {}\n" \
		"\tconst RWTextures::T id;\n" \
		"};\n\n";
	return result;
}

std::string CrShaderMetadataBuilder::BuildStorageBufferMetadataStruct(const std::string bufferName, uint32_t index, const SpvReflectTypeDescription& member, bool rw)
{
	std::string result;

	const std::string rwString = rw ? "RW" : "";

	bool isMemberStruct = member.type_flags & SPV_REFLECT_TYPE_FLAG_STRUCT;

	// Print the struct we are going to derive from that contains the relevant members
	if (isMemberStruct)
	{
		uint32_t indentationLevel = 0;
		result += PrintMemberStruct(member, bufferName + "Data", "", indentationLevel);
	}

	// Print the struct that contains the metadata and is actually used in engine
	result += "template<>\n";
	result += "struct " + rwString + "StorageBufferDataStruct<" + rwString + "StorageBuffers::" + bufferName + ">";

	if (isMemberStruct)
	{
		result += " : public " + bufferName + "Data\n{\n";
		result += "\tusing Data = " + bufferName + "Data;\n";
		result += "\tenum { stride = sizeof(" + bufferName + "Data) };\n";
	}
	else
	{
		std::string typeString = GetBuiltinTypeString(member);
		result += "\n{\n";
		result += "\tusing Data = " + typeString + ";\n";
		result += "\tenum { stride = sizeof(" + typeString + ") };\n";
	}

	result += "\tenum { index = " + std::to_string(index) + " };\n";
	result += "}; typedef " + rwString + "StorageBufferDataStruct<" + rwString + "StorageBuffers::" + bufferName + "> " + bufferName + ";\n\n";

	return result;
}

std::string CrShaderMetadataBuilder::BuildStorageBufferMetadataHeader(const HLSLResources& resources)
{
	std::string result;

	result += StorageBufferSection;

	result += PrintResourceEnum("StorageBuffer", resources.storageBuffers);

	// Print the template
	result += "template<enum StorageBuffers::T index>\nstruct StorageBufferDataStruct {};\n\n";

	for (uint32_t storageBufferIndex = 0; storageBufferIndex < resources.storageBuffers.size(); ++storageBufferIndex)
	{
		const SpvReflectDescriptorBinding& storageBuffer = resources.storageBuffers[storageBufferIndex];
		result += BuildStorageBufferMetadataStruct(storageBuffer.name, storageBufferIndex, storageBuffer.type_description->members[0], false);
	}

	result += PrintStorageBufferMetadataStructDeclaration();

	result += PrintResourceMetadataInstanceDeclaration("StorageBuffer", resources.storageBuffers);

	result += "extern CrHashMap<CrString, StorageBufferMetadata&> StorageBufferTable;\n\n";

	result += "extern CrArray<StorageBufferMetadata, " + std::to_string(resources.storageBuffers.size()) + "> StorageBufferMetaTable;\n\n";

	return result;
}

std::string CrShaderMetadataBuilder::BuildStorageBufferMetadataCpp(const HLSLResources& resources)
{
	std::string result;

	result += StorageBufferSection;

	result += PrintStorageBufferMetadataInstanceDefinition(resources.storageBuffers);

	result += PrintResourceHashmap("StorageBuffer", resources.storageBuffers);

	return result;
}

std::string CrShaderMetadataBuilder::PrintStorageBufferMetadataInstanceDefinition(const ResourceVector& storageBuffers)
{
	std::string result;
	for (const auto& storageBuffer : storageBuffers)
	{
		std::string name = std::string(storageBuffer.name);
		auto scopedMeta = [&](const std::string& member) { return name + "::" + member; }; // Convenience lambda
		result += "StorageBufferMetadata " + name + "MetaInstance(" + scopedMeta("index, ") + scopedMeta("stride") + ");\n";
	}
	result += "StorageBufferMetadata InvalidStorageBufferMetaInstance(UINT32_MAX, 0);\n";
	return result + "\n";
}

std::string CrShaderMetadataBuilder::PrintStorageBufferMetadataStructDeclaration()
{
	std::string result = "struct StorageBufferMetadata"
		"\n{\n"
		"\tStorageBufferMetadata(uint32_t id, uint32_t stride) : id(static_cast<StorageBuffers::T>(id)), stride(stride) {}\n"
		"\tconst StorageBuffers::T id;\n"
		"\tconst uint32_t stride;\n"
		"};\n\n";
	return result;
}

std::string CrShaderMetadataBuilder::BuildRWStorageBufferMetadataHeader(const HLSLResources& resources)
{
	std::string result;

	result += RWStorageBufferSection;

	result += PrintResourceEnum("RWStorageBuffer", resources.rwStorageBuffers);

	// Print the template
	result += "template<enum RWStorageBuffers::T index>\nstruct RWStorageBufferDataStruct {};\n\n";

	for (uint32_t rwStorageBufferIndex = 0; rwStorageBufferIndex < resources.rwStorageBuffers.size(); ++rwStorageBufferIndex)
	{
		const SpvReflectDescriptorBinding& rwStorageBuffer = resources.rwStorageBuffers[rwStorageBufferIndex];
		result += BuildStorageBufferMetadataStruct(rwStorageBuffer.name, rwStorageBufferIndex, rwStorageBuffer.type_description->members[0], true);
	}

	result += PrintRWStorageBufferMetadataStructDeclaration();

	result += PrintResourceMetadataInstanceDeclaration("RWStorageBuffer", resources.rwStorageBuffers);

	result += "extern CrHashMap<CrString, RWStorageBufferMetadata&> RWStorageBufferTable;\n\n";

	result += "extern CrArray<RWStorageBufferMetadata, " + std::to_string(resources.rwStorageBuffers.size()) + "> RWStorageBufferMetaTable;\n\n";

	return result;
}

std::string CrShaderMetadataBuilder::BuildRWStorageBufferMetadataCpp(const HLSLResources& resources)
{
	std::string result;

	result += RWStorageBufferSection;

	result += PrintRWStorageBufferMetadataInstanceDefinition(resources.rwStorageBuffers);

	result += PrintResourceHashmap("RWStorageBuffer", resources.rwStorageBuffers);

	return result;
}

std::string CrShaderMetadataBuilder::PrintRWStorageBufferMetadataInstanceDefinition(const ResourceVector& rwStorageBuffers)
{
	std::string result;
	for (const auto& rwStorageBuffer : rwStorageBuffers)
	{
		std::string name = std::string(rwStorageBuffer.name);
		auto scopedMeta = [&](const std::string& member) { return name + "::" + member; }; // Convenience lambda
		result += "RWStorageBufferMetadata " + name + "MetaInstance(" + scopedMeta("index, ") + scopedMeta("stride") + ");\n";
	}
	result += "RWStorageBufferMetadata InvalidRWStorageBufferMetaInstance(UINT32_MAX, 0);\n";
	return result + "\n";
}

std::string CrShaderMetadataBuilder::PrintRWStorageBufferMetadataStructDeclaration()
{
	std::string result = "struct RWStorageBufferMetadata" \
		"\n{\n" \
		"\tRWStorageBufferMetadata(uint32_t id, uint32_t stride) : id(static_cast<RWStorageBuffers::T>(id)), stride(stride) {}\n" \
		"\tconst RWStorageBuffers::T id;\n" \
		"\tconst uint32_t stride;\n" \
		"};\n\n";
	return result;
}

std::string CrShaderMetadataBuilder::BuildDataBufferMetadataHeader(const HLSLResources& resources)
{
	std::string result;

	result += DataBufferSection;

	result += PrintResourceEnum("DataBuffer", resources.dataBuffers);

	result += PrintDataBufferMetadataStructDeclaration();

	result += PrintResourceMetadataInstanceDeclaration("DataBuffer", resources.dataBuffers);

	result += "extern CrHashMap<CrString, DataBufferMetadata&> DataBufferTable;\n\n";

	result += "extern CrArray<DataBufferMetadata, " + std::to_string(resources.dataBuffers.size()) + "> DataBufferMetaTable;\n\n";

	return result;
}

std::string CrShaderMetadataBuilder::BuildDataBufferMetadataCpp(const HLSLResources& resources)
{
	std::string result;

	result += DataBufferSection;

	result += PrintDataBufferMetadataInstanceDefinition(resources.dataBuffers);

	result += PrintResourceHashmap("DataBuffer", resources.dataBuffers);

	return result;
}

std::string CrShaderMetadataBuilder::PrintDataBufferMetadataInstanceDefinition(const ResourceVector& dataBuffers)
{
	std::string result;
	for (const auto& dataBuffer : dataBuffers)
	{
		result += "DataBufferMetadata " + std::string(dataBuffer.name) + "MetaInstance(" + "DataBuffers::" + std::string(dataBuffer.name) + ");\n";
	}
	result += "DataBufferMetadata InvalidDataBufferMetaInstance(UINT32_MAX);\n";
	return result + "\n";
}

std::string CrShaderMetadataBuilder::PrintDataBufferMetadataStructDeclaration()
{
	std::string result = "struct DataBufferMetadata" \
		"\n{\n" \
		"\tDataBufferMetadata(uint32_t id) : id(static_cast<DataBuffers::T>(id)) {}\n" \
		"\tconst DataBuffers::T id;\n" \
		"};\n\n";
	return result;
}

std::string CrShaderMetadataBuilder::BuildRWDataBufferMetadataHeader(const HLSLResources& resources)
{
	std::string result;

	result += RWDataBufferSection;

	result += PrintResourceEnum("RWDataBuffer", resources.rwDataBuffers);

	result += PrintRWDataBufferMetadataStructDeclaration();

	result += PrintResourceMetadataInstanceDeclaration("RWDataBuffer", resources.rwDataBuffers);

	result += "extern CrHashMap<CrString, RWDataBufferMetadata&> RWDataBufferTable;\n\n";

	result += "extern CrArray<RWDataBufferMetadata, " + std::to_string(resources.rwDataBuffers.size()) + "> RWDataBufferMetaTable;\n\n";

	return result;
}

std::string CrShaderMetadataBuilder::BuildRWDataBufferMetadataCpp(const HLSLResources& resources)
{
	std::string result;

	result += RWDataBufferSection;

	result += PrintRWDataBufferMetadataInstanceDefinition(resources.rwDataBuffers);

	result += PrintResourceHashmap("RWDataBuffer", resources.rwDataBuffers);

	return result;
}

std::string CrShaderMetadataBuilder::PrintRWDataBufferMetadataInstanceDefinition(const ResourceVector& rwDataBuffers)
{
	std::string result;
	for (const auto& rwDataBuffer : rwDataBuffers)
	{
		result += "RWDataBufferMetadata " + std::string(rwDataBuffer.name) + "MetaInstance(" + "RWDataBuffers::" + std::string(rwDataBuffer.name) + ");\n";
	}
	result += "RWDataBufferMetadata InvalidRWDataBufferMetaInstance(UINT32_MAX);\n";
	return result + "\n";
}

std::string CrShaderMetadataBuilder::PrintRWDataBufferMetadataStructDeclaration()
{
	std::string result = "struct RWDataBufferMetadata" \
		"\n{\n" \
		"\tRWDataBufferMetadata(uint32_t id) : id(static_cast<RWDataBuffers::T>(id)) {}\n" \
		"\tconst RWDataBuffers::T id;\n" \
		"};\n\n";
	return result;
}

void CrShaderMetadataBuilder::WriteToFile(const std::string& filename, const std::string& text)
{
	std::ofstream fileStream;
	fileStream.open(filename.c_str(), std::ios::out);
	fileStream.write(text.c_str(), text.size());
	fileStream.close();
	printf("Wrote contents of file to %s\n", filename.c_str());
}

void CrShaderMetadataBuilder::WriteToFileIfChanged(const std::string& filename, const std::string& text)
{
	std::ifstream originalFile(filename);
	std::stringstream originalFileStream;
	originalFileStream << originalFile.rdbuf();
	originalFile.close();

	const std::string& originalContents = originalFileStream.str();

	if (originalContents != text)
	{
		WriteToFile(filename, text);
	}
}
