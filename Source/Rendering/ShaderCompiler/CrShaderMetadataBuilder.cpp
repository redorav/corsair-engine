#include "Rendering/ShaderCompiler/CrShaderCompiler_pch.h"

#include "CrShaderMetadataBuilder.h"
#include "CrShaderCompilerUtilities.h"

#include "CrShaderCompiler.h"
#include "CrCompilerGLSLANG.h"

#include "Core/FileSystem/CrFixedPath.h"

#include "Rendering/CrRendering.h"

#include "spirv_reflect.h"

static const CrString AutoGeneratedDisclaimer =
"//------------------------------------------------------------------------\n"
"// This file was autogenerated by the Corsair Engine Shader Compiler.\n"
"// Modifying it will have no effect after the file is regenerated.\n"
"//------------------------------------------------------------------------\n\n";

static const CrString ConstantBufferSection =
"//-----------------\n"
"// Constant Buffers\n"
"//-----------------\n\n";

static const CrString SamplerSection =
"//---------\n"
"// Samplers\n"
"//---------\n\n";

static const CrString TextureSection =
"//---------\n"
"// Textures\n"
"//---------\n\n";

static const CrString RWTextureSection =
"//------------\n"
"// RW Textures\n"
"//------------\n\n";

static const CrString StorageBufferSection =
"//----------------\n"
"// Storage Buffers\n"
"//----------------\n\n";

static const CrString RWStorageBufferSection =
"//-------------------\n"
"// RW Storage Buffers\n"
"//-------------------\n\n";

static const CrString TypedBufferSection =
"//--------------\n"
"// Typed Buffers\n"
"//--------------\n\n";

static const CrString RWTypedBufferSection =
"//-----------------\n"
"// RW Typed Buffers\n"
"//-----------------\n\n";

bool CrShaderMetadataBuilder::BuildMetadata(const CompilationDescriptor& compilationDescriptor, CrString& compilationStatus)
{
	std::vector<uint32_t> spirvBytecode;
	bool compiled = CrCompilerGLSLANG::HLSLtoSPIRV(compilationDescriptor, spirvBytecode, compilationStatus);

	if (!compiled)
	{
		return false;
	}

	CrString metadataHeader, metadataCpp;
	bool builtMetadata = BuildSPIRVMetadata(spirvBytecode, metadataHeader, metadataCpp);

	if (!builtMetadata)
	{
		return false;
	}

	// Create header and cpp filenames
	CrFixedPath headerPath = compilationDescriptor.outputPath.c_str();
	CrShaderCompilerUtilities::WriteToFileIfChanged(headerPath.replace_extension("h").c_str(), metadataHeader);

	CrFixedPath cppPath = compilationDescriptor.outputPath.c_str();
	CrShaderCompilerUtilities::WriteToFileIfChanged(cppPath.replace_extension("cpp").c_str(), metadataCpp);

	// Write dummy file that tells the build system dependency tracker that files are up to date
	CrFixedPath uptodatePath = compilationDescriptor.outputPath.c_str();
	CrShaderCompilerUtilities::WriteToFile(uptodatePath.replace_extension("uptodate").c_str(), "");

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
					typedBuffers.push_back(binding);
					break;
				}
				case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
				{
					rwTypedBuffers.push_back(binding);
					break;
				}
				default:
				{
					printf("Unrecognized!");
				}
			}
		}
	}

	CrVector<SpvReflectDescriptorBinding> constantBuffers;
	CrVector<SpvReflectDescriptorBinding> samplers;
	CrVector<SpvReflectDescriptorBinding> textures;
	CrVector<SpvReflectDescriptorBinding> rwTextures;
	CrVector<SpvReflectDescriptorBinding> storageBuffers;
	CrVector<SpvReflectDescriptorBinding> rwStorageBuffers;
	CrVector<SpvReflectDescriptorBinding> typedBuffers;
	CrVector<SpvReflectDescriptorBinding> rwTypedBuffers;

	//std::vector<SpvReflectDescriptorBinding> stageInputs;
	//std::vector<SpvReflectDescriptorBinding> stageOutputs;
	//std::vector<SpvReflectDescriptorBinding> subpassInputs;
	//std::vector<SpvReflectDescriptorBinding> atomicCounters;
	//std::vector<SpvReflectDescriptorBinding> accelerationStructures;
	//std::vector<SpvReflectDescriptorBinding> pushConstantBuffers;
};

bool CrShaderMetadataBuilder::BuildSPIRVMetadata(
	const std::vector<uint32_t>& spirvBytecode, 
	CrString& metadataHeader, CrString& metadataCpp)
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

	metadataHeader += BuildTypedBufferMetadataHeader(resources);

	metadataHeader += BuildRWTypedBufferMetadataHeader(resources);

	//metadataHeader += "}\n";

	//---------------
	// Build cpp file
	//---------------

	metadataCpp = AutoGeneratedDisclaimer;
	
	// Maybe there is a better way of not writing these headers directly. They depend on the physical
	// structure of the code which can change
	metadataCpp += "#include \"Rendering/CrRendering_pch.h\"\n";
	metadataCpp += "#include \"ShaderMetadata.h\"\n\n";
	metadataCpp += "\n";

	metadataCpp += BuildConstantBufferMetadataCpp(resources);

	metadataCpp += BuildSamplerMetadataCpp(resources);

	metadataCpp += BuildTextureMetadataCpp(resources);

	metadataCpp += BuildRWTextureMetadataCpp(resources);

	metadataCpp += BuildStorageBufferMetadataCpp(resources);

	metadataCpp += BuildRWStorageBufferMetadataCpp(resources);

	metadataCpp += BuildTypedBufferMetadataCpp(resources);

	metadataCpp += BuildRWTypedBufferMetadataCpp(resources);

	return true;
}

CrString CrShaderMetadataBuilder::BuildConstantBufferMetadataHeader(const HLSLResources& resources)
{
	CrString result;

	result += ConstantBufferSection;

	// 1. Print an enum with all the constant buffers, assigning them an index
	result += PrintResourceEnum("ConstantBuffer", resources.constantBuffers);

	// 2. For every constant buffer, print the data that defines it
	for (uint32_t uniformBufferIndex = 0; uniformBufferIndex < resources.constantBuffers.size(); ++uniformBufferIndex)
	{
		const SpvReflectDescriptorBinding& uniformBuffer = resources.constantBuffers[uniformBufferIndex];
		
		uint32_t indentationLevel = 0;

		CrString constantBufferName = uniformBuffer.type_description->type_name;

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

	result += "extern CrArray<ConstantBufferMetadata, " + CrString(resources.constantBuffers.size()) + CrString("> ConstantBufferMetaTable;\n\n");

	return result;
}

CrString CrShaderMetadataBuilder::BuildConstantBufferMetadataCpp(const HLSLResources& resources)
{
	CrString result;

	result += ConstantBufferSection;

	result += PrintConstantBufferMetadataInstanceDefinition(resources.constantBuffers);

	result += PrintResourceHashmap("ConstantBuffer", resources.constantBuffers);

	return result;
}

CrString CrShaderMetadataBuilder::PrintResourceEnum(const CrString& resourceTypeName, const ResourceVector& resources)
{
	CrString result;
	result += "namespace " + resourceTypeName + "s\n{\n\tenum T : uint8_t\n\t{";

	for (uint32_t i = 0; i < resources.size(); ++i)
	{
		const SpvReflectDescriptorBinding& resource = resources[i];
		CrString resourceName = CrString(resource.name) == "" ? resource.type_description->type_name : resource.name;
		result += "\n\t\t" + resourceName + " = " + CrString(i) + ",";
	}

	result += "\n\t\tCount\n\t};\n};\n\n";
	return result;
}

CrString CrShaderMetadataBuilder::PrintResourceHashmap(const CrString& resourceTypeName, const ResourceVector& resources)
{
	CrString result;
	result += "CrHashMap<CrString, " + resourceTypeName + "Metadata&> " + resourceTypeName + "Table =\n{";

	for (const auto& resource : resources)
	{
		CrString resourceName = CrString(resource.name) == "" ? resource.type_description->type_name : resource.name;
		result += "\n\t{ \"" + resourceName + "\", " + resourceName + "MetaInstance },";
	}
	result += "\n};\n\n";

	result += "CrArray<" + resourceTypeName + "Metadata, " + CrString(resources.size()) + "> " + resourceTypeName + "MetaTable =\n{";

	for (const auto& resource : resources)
	{
		CrString resourceName = CrString(resource.name) == "" ? resource.type_description->type_name : resource.name;
		result += "\n\t" + resourceName + "MetaInstance,";
	}
	result += "\n};\n\n";

	return result;
}

CrString GetBuiltinTypeString(const SpvReflectTypeDescription& type)
{
	CrString result;

	bool isComplexType = 
		(type.type_flags & SPV_REFLECT_TYPE_FLAG_MATRIX) ||
		(type.type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR);

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
		if (type.traits.numeric.scalar.signedness != 0)
		{
			result += "int";
		}
		else
		{
			if (type.traits.numeric.vector.component_count > 1)
			{
				result += "uint";
			}
			else
			{
				result += "uint32_t";
			}
		}
	}
	else if (type.type_flags & SPV_REFLECT_TYPE_FLAG_BOOL)
	{
		result += "bool";
	}

	if (type.type_flags & SPV_REFLECT_TYPE_FLAG_MATRIX)
	{
		// In SPIR-V, columns and rows are swapped with respect to HLSL.
		// There are good explanations here as to why this is the case
		// https://github.com/microsoft/DirectXShaderCompiler/blob/master/docs/SPIR-V.rst#vectors-and-matrices
		// https://github.com/microsoft/DirectXShaderCompiler/blob/master/docs/SPIR-V.rst#appendix-a-matrix-representation
		result += CrString(type.traits.numeric.matrix.column_count) + "x" + CrString(type.traits.numeric.matrix.row_count);
	}
	else if (type.type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR)
	{
		result += CrString(type.traits.numeric.vector.component_count);
	}

	return result;
}

CrString CrShaderMetadataBuilder::PrintMemberBuiltIn(const SpvReflectTypeDescription& type, const CrString& memberName, const CrString& indentation)
{
	CrString result;

	result += indentation;

	result += GetBuiltinTypeString(type);

	result += " " + memberName;

	// Add array dimensions
	for(uint32_t i = 0; i < type.traits.array.dims_count; ++i)
	{
		result += "[" + CrString(type.traits.array.dims[i]) + "]";
	}

	result += ";\n";
	return result;
}

CrString CrShaderMetadataBuilder::PrintMemberStruct(const SpvReflectTypeDescription& type, const CrString& structTypeName, const CrString& structName, uint32_t indentationLevel)
{
	CrString result;
	CrString structIndentation;

	for (uint32_t i = 0; i < indentationLevel; ++i)
	{
		structIndentation += "\t";
	}

	result += structIndentation + "struct " + structTypeName + "\n" + structIndentation + "{\n"; // Open the struct

	CrString memberIndentation = structIndentation + "\t";

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

CrString CrShaderMetadataBuilder::PrintResourceMetadataInstanceDeclaration(const CrString& resourceType, const ResourceVector& resources)
{
	CrString result;
	for (const SpvReflectDescriptorBinding& resource : resources)
	{
		CrString resourceName = CrString(resource.name) == "" ? resource.type_description->type_name : resource.name;
		result += "extern " + resourceType + "Metadata " + resourceName + "MetaInstance;\n";
	}

	result += "extern " + resourceType + "Metadata Invalid" + resourceType + "MetaInstance;\n";
	return result + "\n";
}

CrString CrShaderMetadataBuilder::PrintConstantBufferStructMetadata(const CrString& name, int index)
{
	CrString result;
	result += "struct " + name + " : public " + name + "Data\n{\n";
	result += "\tusing Data = " + name + "Data;\n";
	result += "\tenum { size = sizeof(" + name + "Data) };\n";
	result += "\tenum { index = " + CrString(index) + " };\n";
	result += "};\n\n";
	return result;
}

CrString CrShaderMetadataBuilder::PrintConstantBufferMetadataStructDeclaration()
{
	CrString result = "struct ConstantBufferMetadata" \
		"\n{\n" \
		"\tConstantBufferMetadata(uint32_t id, uint32_t size) : id(static_cast<ConstantBuffers::T>(id)), size(size) {}\n" \
		"\tconst ConstantBuffers::T id;\n" \
		"\tconst uint32_t size;\n" \
		"};\n\n";
	return result;
}

CrString CrShaderMetadataBuilder::PrintConstantBufferMetadataInstanceDefinition(const ResourceVector& uniformBuffers)
{
	CrString result;
	for (const SpvReflectDescriptorBinding& uniformBuffer : uniformBuffers)
	{
		CrString name = CrString(uniformBuffer.name) == "" ? uniformBuffer.type_description->type_name : uniformBuffer.name;
		auto scopedMeta = [&](const CrString& member) { return name + "::" + member; }; // Convenience lambda
		result += "ConstantBufferMetadata " + name + "MetaInstance(" + scopedMeta("index, ") + scopedMeta("size") + ");\n";
	}
	result += "ConstantBufferMetadata InvalidConstantBufferMetaInstance(UINT32_MAX, 0);\n";
	return result + "\n";
}

CrString CrShaderMetadataBuilder::BuildTextureMetadataHeader(const HLSLResources& resources)
{
	CrString result;

	result += TextureSection;

	result += PrintResourceEnum("Texture", resources.textures);

	result += PrintTextureMetadataStructDeclaration();

	result += PrintResourceMetadataInstanceDeclaration("Texture", resources.textures);

	result += "extern CrHashMap<CrString, TextureMetadata&> TextureTable;\n\n";

	result += "extern CrArray<TextureMetadata, " + CrString(resources.textures.size()) + "> TextureMetaTable;\n\n";

	return result;
}

CrString CrShaderMetadataBuilder::BuildTextureMetadataCpp(const HLSLResources& resources)
{
	CrString result;

	result += TextureSection;

	result += PrintTextureMetadataInstanceDefinition(resources.textures);

	result += PrintResourceHashmap("Texture", resources.textures);

	return result;
}

CrString CrShaderMetadataBuilder::PrintTextureMetadataStructDeclaration()
{
	CrString result = "struct TextureMetadata" \
		"\n{\n" \
		"\tTextureMetadata(uint32_t id) : id(static_cast<Textures::T>(id)) {}\n" \
		"\tconst Textures::T id;\n" \
		"};\n\n";
	return result;
}

CrString CrShaderMetadataBuilder::PrintTextureMetadataInstanceDefinition(const ResourceVector& textures)
{
	CrString result;
	for (const auto& texture : textures)
	{
		result += "TextureMetadata " + CrString(texture.name) + "MetaInstance(" + "Textures::" + CrString(texture.name) + ");\n";
	}
	result += "TextureMetadata InvalidTextureMetaInstance(UINT32_MAX);\n";
	return result + "\n";
}

CrString CrShaderMetadataBuilder::BuildSamplerMetadataHeader(const HLSLResources& resources)
{
	CrString result;

	result += SamplerSection;

	result += PrintResourceEnum("Sampler", resources.samplers);

	result += PrintSamplerMetadataStructDeclaration();

	result += PrintResourceMetadataInstanceDeclaration("Sampler", resources.samplers);

	result += "extern CrHashMap<CrString, SamplerMetadata&> SamplerTable;\n\n";

	result += "extern CrArray<SamplerMetadata, " + CrString(resources.samplers.size()) + "> SamplerMetaTable;\n\n";

	return result;
}

CrString CrShaderMetadataBuilder::BuildSamplerMetadataCpp(const HLSLResources& resources)
{
	CrString result;

	result += SamplerSection;

	result += PrintSamplerMetadataInstanceDefinition(resources.samplers);

	result += PrintResourceHashmap("Sampler", resources.samplers);

	return result;
}

CrString CrShaderMetadataBuilder::PrintSamplerMetadataStructDeclaration()
{
	CrString result = "struct SamplerMetadata" \
		"\n{\n" \
		"\tSamplerMetadata(uint32_t id) : id(static_cast<Samplers::T>(id)) {}\n" \
		"\tconst Samplers::T id;\n" \
		"};\n\n";
	return result;
}

CrString CrShaderMetadataBuilder::PrintSamplerMetadataInstanceDefinition(const ResourceVector& samplers)
{
	CrString result;
	for (const auto& sampler : samplers) { result += "SamplerMetadata " + CrString(sampler.name) + "MetaInstance(" + "Samplers::" + CrString(sampler.name) + ");\n"; }
	result += "SamplerMetadata InvalidSamplerMetaInstance(UINT32_MAX);\n";
	return result + "\n";
}

CrString CrShaderMetadataBuilder::BuildRWTextureMetadataHeader(const HLSLResources& resources)
{
	CrString result;

	result += RWTextureSection;

	result += PrintResourceEnum("RWTexture", resources.rwTextures);

	result += PrintRWTextureMetadataStructDeclaration();

	result += PrintResourceMetadataInstanceDeclaration("RWTexture", resources.rwTextures);

	result += "extern CrHashMap<CrString, RWTextureMetadata&> RWTextureTable;\n\n";

	result += "extern CrArray<RWTextureMetadata, " + CrString(resources.rwTextures.size()) + "> RWTextureMetaTable;\n\n";

	return result;
}

CrString CrShaderMetadataBuilder::BuildRWTextureMetadataCpp(const HLSLResources& resources)
{
	CrString result;

	result += RWTextureSection;

	result += PrintRWTextureMetadataInstanceDefinition(resources.rwTextures);

	result += PrintResourceHashmap("RWTexture", resources.rwTextures);

	return result;
}

CrString CrShaderMetadataBuilder::PrintRWTextureMetadataInstanceDefinition(const ResourceVector& rwTextures)
{
	CrString result;
	for (const auto& rwTexture : rwTextures)
	{
		result += "RWTextureMetadata " + CrString(rwTexture.name) + "MetaInstance(" + "RWTextures::" + CrString(rwTexture.name) + ");\n";
	}
	result += "RWTextureMetadata InvalidRWTextureMetaInstance(UINT32_MAX);\n";
	return result + "\n";
}

CrString CrShaderMetadataBuilder::PrintRWTextureMetadataStructDeclaration()
{
	CrString result = "struct RWTextureMetadata" \
		"\n{\n" \
		"\tRWTextureMetadata(uint32_t id) : id(static_cast<RWTextures::T>(id)) {}\n" \
		"\tconst RWTextures::T id;\n" \
		"};\n\n";
	return result;
}

CrString CrShaderMetadataBuilder::BuildStorageBufferMetadataStruct(const CrString bufferName, uint32_t index, const SpvReflectTypeDescription& member, bool rw)
{
	CrString result;

	const CrString rwString = rw ? "RW" : "";

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
		CrString typeString = GetBuiltinTypeString(member);
		result += "\n{\n";
		result += "\tusing Data = " + typeString + ";\n";
		result += "\tenum { stride = sizeof(" + typeString + ") };\n";
	}

	result += "\tenum { index = " + CrString(index) + " };\n";
	result += "}; typedef " + rwString + "StorageBufferDataStruct<" + rwString + "StorageBuffers::" + bufferName + "> " + bufferName + ";\n\n";

	return result;
}

CrString CrShaderMetadataBuilder::BuildStorageBufferMetadataHeader(const HLSLResources& resources)
{
	CrString result;

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

	result += "extern CrArray<StorageBufferMetadata, " + CrString(resources.storageBuffers.size()) + "> StorageBufferMetaTable;\n\n";

	return result;
}

CrString CrShaderMetadataBuilder::BuildStorageBufferMetadataCpp(const HLSLResources& resources)
{
	CrString result;

	result += StorageBufferSection;

	result += PrintStorageBufferMetadataInstanceDefinition(resources.storageBuffers);

	result += PrintResourceHashmap("StorageBuffer", resources.storageBuffers);

	return result;
}

CrString CrShaderMetadataBuilder::PrintStorageBufferMetadataInstanceDefinition(const ResourceVector& storageBuffers)
{
	CrString result;
	for (const auto& storageBuffer : storageBuffers)
	{
		CrString name = CrString(storageBuffer.name);
		auto scopedMeta = [&](const CrString& member) { return name + "::" + member; }; // Convenience lambda
		result += "StorageBufferMetadata " + name + "MetaInstance(" + scopedMeta("index, ") + scopedMeta("stride") + ");\n";
	}
	result += "StorageBufferMetadata InvalidStorageBufferMetaInstance(UINT32_MAX, 0);\n";
	return result + "\n";
}

CrString CrShaderMetadataBuilder::PrintStorageBufferMetadataStructDeclaration()
{
	CrString result = "struct StorageBufferMetadata"
		"\n{\n"
		"\tStorageBufferMetadata(uint32_t id, uint32_t stride) : id(static_cast<StorageBuffers::T>(id)), stride(stride) {}\n"
		"\tconst StorageBuffers::T id;\n"
		"\tconst uint32_t stride;\n"
		"};\n\n";
	return result;
}

CrString CrShaderMetadataBuilder::BuildRWStorageBufferMetadataHeader(const HLSLResources& resources)
{
	CrString result;

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

	result += "extern CrArray<RWStorageBufferMetadata, " + CrString(resources.rwStorageBuffers.size()) + "> RWStorageBufferMetaTable;\n\n";

	return result;
}

CrString CrShaderMetadataBuilder::BuildRWStorageBufferMetadataCpp(const HLSLResources& resources)
{
	CrString result;

	result += RWStorageBufferSection;

	result += PrintRWStorageBufferMetadataInstanceDefinition(resources.rwStorageBuffers);

	result += PrintResourceHashmap("RWStorageBuffer", resources.rwStorageBuffers);

	return result;
}

CrString CrShaderMetadataBuilder::PrintRWStorageBufferMetadataInstanceDefinition(const ResourceVector& rwStorageBuffers)
{
	CrString result;
	for (const auto& rwStorageBuffer : rwStorageBuffers)
	{
		CrString name = CrString(rwStorageBuffer.name);
		auto scopedMeta = [&](const CrString& member) { return name + "::" + member; }; // Convenience lambda
		result += "RWStorageBufferMetadata " + name + "MetaInstance(" + scopedMeta("index, ") + scopedMeta("stride") + ");\n";
	}
	result += "RWStorageBufferMetadata InvalidRWStorageBufferMetaInstance(UINT32_MAX, 0);\n";
	return result + "\n";
}

CrString CrShaderMetadataBuilder::PrintRWStorageBufferMetadataStructDeclaration()
{
	CrString result = "struct RWStorageBufferMetadata" \
		"\n{\n" \
		"\tRWStorageBufferMetadata(uint32_t id, uint32_t stride) : id(static_cast<RWStorageBuffers::T>(id)), stride(stride) {}\n" \
		"\tconst RWStorageBuffers::T id;\n" \
		"\tconst uint32_t stride;\n" \
		"};\n\n";
	return result;
}

CrString CrShaderMetadataBuilder::BuildTypedBufferMetadataHeader(const HLSLResources& resources)
{
	CrString result;

	result += TypedBufferSection;

	result += PrintResourceEnum("TypedBuffer", resources.typedBuffers);

	result += PrintTypedBufferMetadataStructDeclaration();

	result += PrintResourceMetadataInstanceDeclaration("TypedBuffer", resources.typedBuffers);

	result += "extern CrHashMap<CrString, TypedBufferMetadata&> TypedBufferTable;\n\n";

	result += "extern CrArray<TypedBufferMetadata, " + CrString(resources.typedBuffers.size()) + "> TypedBufferMetaTable;\n\n";

	return result;
}

CrString CrShaderMetadataBuilder::BuildTypedBufferMetadataCpp(const HLSLResources& resources)
{
	CrString result;

	result += TypedBufferSection;

	result += PrintTypedBufferMetadataInstanceDefinition(resources.typedBuffers);

	result += PrintResourceHashmap("TypedBuffer", resources.typedBuffers);

	return result;
}

CrString CrShaderMetadataBuilder::PrintTypedBufferMetadataInstanceDefinition(const ResourceVector& typedBuffers)
{
	CrString result;
	for (const auto& typedBuffer : typedBuffers)
	{
		result += "TypedBufferMetadata " + CrString(typedBuffer.name) + "MetaInstance(" + "TypedBuffers::" + CrString(typedBuffer.name) + ");\n";
	}
	result += "TypedBufferMetadata InvalidTypedBufferMetaInstance(UINT32_MAX);\n";
	return result + "\n";
}

CrString CrShaderMetadataBuilder::PrintTypedBufferMetadataStructDeclaration()
{
	CrString result = "struct TypedBufferMetadata" \
		"\n{\n" \
		"\tTypedBufferMetadata(uint32_t id) : id(static_cast<TypedBuffers::T>(id)) {}\n" \
		"\tconst TypedBuffers::T id;\n" \
		"};\n\n";
	return result;
}

CrString CrShaderMetadataBuilder::BuildRWTypedBufferMetadataHeader(const HLSLResources& resources)
{
	CrString result;

	result += RWTypedBufferSection;

	result += PrintResourceEnum("RWTypedBuffer", resources.rwTypedBuffers);

	result += PrintRWTypedBufferMetadataStructDeclaration();

	result += PrintResourceMetadataInstanceDeclaration("RWTypedBuffer", resources.rwTypedBuffers);

	result += "extern CrHashMap<CrString, RWTypedBufferMetadata&> RWTypedBufferTable;\n\n";

	result += "extern CrArray<RWTypedBufferMetadata, " + CrString(resources.rwTypedBuffers.size()) + "> RWTypedBufferMetaTable;\n\n";

	return result;
}

CrString CrShaderMetadataBuilder::BuildRWTypedBufferMetadataCpp(const HLSLResources& resources)
{
	CrString result;

	result += RWTypedBufferSection;

	result += PrintRWTypedBufferMetadataInstanceDefinition(resources.rwTypedBuffers);

	result += PrintResourceHashmap("RWTypedBuffer", resources.rwTypedBuffers);

	return result;
}

CrString CrShaderMetadataBuilder::PrintRWTypedBufferMetadataInstanceDefinition(const ResourceVector& rwTypedBuffers)
{
	CrString result;
	for (const auto& rwTypedBuffer : rwTypedBuffers)
	{
		result += "RWTypedBufferMetadata " + CrString(rwTypedBuffer.name) + "MetaInstance(" + "RWTypedBuffers::" + CrString(rwTypedBuffer.name) + ");\n";
	}
	result += "RWTypedBufferMetadata InvalidRWTypedBufferMetaInstance(UINT32_MAX);\n";
	return result + "\n";
}

CrString CrShaderMetadataBuilder::PrintRWTypedBufferMetadataStructDeclaration()
{
	CrString result = "struct RWTypedBufferMetadata" \
		"\n{\n" \
		"\tRWTypedBufferMetadata(uint32_t id) : id(static_cast<RWTypedBuffers::T>(id)) {}\n" \
		"\tconst RWTypedBuffers::T id;\n" \
		"};\n\n";
	return result;
}