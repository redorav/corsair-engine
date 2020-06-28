#include "CrShaderMetadataBuilder.h"
#include "CrSPIRVCompiler.h"

#include "Core/FileSystem/CrFileSystem.h"

#include "Rendering/CrRendering.h"

#include <string>
#include <fstream>
#include <sstream>

#pragma warning (push, 0)
// Glslang
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>

// SPIR-V
#include <spirv_cross.hpp>
#include <spirv_cpp.hpp>
#pragma warning (pop)

static const std::string AutoGeneratedDisclaimer =
"//------------------------------------------------------------------------\n\
// This file was autogenerated by the Corsair Engine Shader Compiler.\n\
// Modifying it will have no effect.\n\
//------------------------------------------------------------------------\n\n";

static const std::string ConstantBufferSection = "\
//-----------------\n\
// Constant Buffers\n\
//-----------------\n\n";

static const std::string TextureSection = "\
//---------\n\
// Textures\n\
//---------\n\n";

static const std::string SamplerSection = "\
//---------\n\
// Samplers\n\
//---------\n\n";

static const std::string StorageImageSection = "\
//---------------\n\
// Storage Images\n\
//---------------\n\n";

static const std::string StorageBufferSection = "\
//----------------\n\
// Storage Buffers\n\
//----------------\n\n";

bool CrShaderMetadataBuilder::BuildMetadata(const CrPath& inputHLSL, const CrPath& outputFilename, const std::string& entryPoint)
{
	std::vector<uint32_t> spirvBytecode;
	bool compiled = CrSPIRVCompiler::HLSLtoSPIRV(inputHLSL.string(), entryPoint, cr3d::ShaderStage::Pixel, spirvBytecode);

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
	CrPath headerPath = outputFilename;
	WriteToFileIfChanged(headerPath.replace_extension("h").string(), metadataHeader);

	CrPath cppPath = outputFilename;
	WriteToFileIfChanged(cppPath.replace_extension("cpp").string(), metadataCpp);

	// Write dummy file that tells the build system dependency tracker that files are up to date
	CrPath uptodatePath = outputFilename;
	WriteToFile(uptodatePath.replace_extension("uptodate").string(), "");

	return true;
}

bool CrShaderMetadataBuilder::BuildSPIRVMetadata(const std::vector<uint32_t>& spirvBytecode, std::string& metadataHeader, std::string& metadataCpp)
{
	spirv_cross::Compiler reflection(spirvBytecode.data(), spirvBytecode.size());

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
	metadataHeader += "\n";

	metadataHeader += BuildConstantBufferMetadataHeader(reflection);

	metadataHeader += BuildTextureMetadataHeader(reflection);

	metadataHeader += BuildSamplerMetadataHeader(reflection);

	metadataHeader += BuildStorageImageMetadataHeader(reflection);

	metadataHeader += BuildStorageBufferMetadataHeader(reflection);

	//metadataHeader += "}\n";

	//---------------
	// Build cpp file
	//---------------

	metadataCpp = AutoGeneratedDisclaimer;
	
	// Maybe there is a better way of not writing these headers directly. They depend on the physical
	// structure of the code which can change
	metadataCpp += "#include \"CrRendering_pch.h\"\n";
	metadataCpp += "#include \"ShaderResources.h\"\n\n";
	metadataCpp += "#include \"Core/Containers/CrArray.h\"\n";
	metadataCpp += "#include \"Core/Containers/CrHashMap.h\"\n";
	metadataCpp += "#include \"Core/Containers/CrVector.h\"\n";
	metadataCpp += "#include \"Core/String/CrString.h\"\n";
	metadataCpp += "\n";

	metadataCpp += BuildConstantBufferMetadataCpp(reflection);

	metadataCpp += BuildTextureMetadataCpp(reflection);

	metadataCpp += BuildSamplerMetadataCpp(reflection);

	return true;
}

std::string CrShaderMetadataBuilder::BuildConstantBufferMetadataHeader(spirv_cross::Compiler& reflection)
{
	const spirv_cross::ShaderResources& resources = reflection.get_shader_resources();
	std::string result;

	result += ConstantBufferSection;

	// 1. Print an enum with all the constant buffers, assigning them an index
	result += PrintResourceEnum(reflection, "ConstantBuffer", resources.uniform_buffers);

	// 2. For every constant buffer, print the data that defines it
	for (uint32_t uniformBufferIndex = 0; uniformBufferIndex < resources.uniform_buffers.size(); ++uniformBufferIndex)
	{
		const spirv_cross::Resource& uniformBuffer = resources.uniform_buffers[uniformBufferIndex];
		spirv_cross::SPIRType uniformBuffertype = reflection.get_type(uniformBuffer.base_type_id);

		// If the only member of the constant buffer is a struct, don't write it out as such, but flatten and merge it with the constant buffer. The way we do this
		// is we pass the SPIRType of the first struct instead of the SPIRType of the actual uniform buffer.
		bool flattenStruct = uniformBuffertype.member_types.size() == 1 && reflection.get_type(uniformBuffertype.member_types[0]).basetype == spirv_cross::SPIRType::Struct;
		const spirv_cross::SPIRType& structType = flattenStruct ? reflection.get_type(uniformBuffertype.member_types[0]) : uniformBuffertype;

		uint32_t indentationLevel = 0;
		result += PrintConstantBufferMemberStruct(reflection, structType, uniformBuffer.name + "Data", "", indentationLevel);

		result += PrintConstantBufferStructMetadata(uniformBuffer.name, uniformBufferIndex);
	}

	result += PrintConstantBufferMetadataStructDeclaration();

	result += PrintResourceMetadataInstanceDeclaration("ConstantBuffer", resources.uniform_buffers);

	//result += PrintConstantBufferGlobalGroupDeclaration(resources.uniform_buffers);

	result += "extern CrHashMap<CrString, ConstantBufferMetadata&> ConstantBufferTable;\n\n";

	result += "extern CrArray<ConstantBufferMetadata, " + std::to_string(resources.uniform_buffers.size()) + std::string("> ConstantBufferMetaTable;\n\n");

	return result;
}

std::string CrShaderMetadataBuilder::BuildConstantBufferMetadataCpp(spirv_cross::Compiler& reflection)
{
	const spirv_cross::ShaderResources& resources = reflection.get_shader_resources();

	std::string result;

	result += ConstantBufferSection;

	result += PrintConstantBufferMetadataInstanceDefinition(resources.uniform_buffers);

	//result += PrintConstantBufferGlobalGroupDefinition(resources.uniform_buffers);

	result += PrintResourceHashmap(reflection, "ConstantBuffer", resources.uniform_buffers);

	return result;
}

std::string CrShaderMetadataBuilder::PrintResourceEnum(spirv_cross::Compiler& reflection, const std::string& resourceType, const SPIRVCrossResourceVector& resources)
{
	std::string result;
	result += "namespace " + resourceType + "s\n{\n\tenum T : uint8_t\n\t{";

	for (uint32_t i = 0; i < resources.size(); ++i)
	{
		const spirv_cross::Resource& resource = resources[i];

		std::string resourceName = reflection.get_name(resource.id) == "" ? resource.name : reflection.get_name(resource.id);
		result += "\n\t\t" + resourceName + " = " + std::to_string(i) + ",";
	}

	result += "\n\t\tCount\n\t};\n};\n\n";
	return result;
}

std::string CrShaderMetadataBuilder::PrintResourceHashmap(spirv_cross::Compiler& reflection, const std::string& resourceType, const SPIRVCrossResourceVector& resources)
{
	std::string result;
	result += "CrHashMap<CrString, " + resourceType + "Metadata&> " + resourceType + "Table = { ";
	
	for (const auto& resource : resources)
	{
		std::string resourceName = reflection.get_name(resource.id) == "" ? resource.name : reflection.get_name(resource.id);
		result += "{\"" + resourceName + "\", " + resourceName + "MetaInstance }, ";
	}
	result += "};\n\n";

	result += "CrArray<" + resourceType + "Metadata, " + std::to_string(resources.size()) + "> " + resourceType + "MetaTable = { ";

	for (const auto& resource : resources)
	{
		std::string resourceName = reflection.get_name(resource.id) == "" ? resource.name : reflection.get_name(resource.id);
		result += resourceName + "MetaInstance, ";
	}
	result += "};\n\n";

	return result;
}

std::string CrShaderMetadataBuilder::PrintConstantBufferMemberBuiltIn(spirv_cross::SPIRType builtinType, const std::string& memberName, const std::string& indentation)
{
	std::string result;
	switch (builtinType.basetype)
	{
		case spirv_cross::SPIRType::Struct:
			break;
		case spirv_cross::SPIRType::Float:		result += indentation + "float";	break;
		case spirv_cross::SPIRType::Int:		result += indentation + "int";		break;
		case spirv_cross::SPIRType::Double:		result += indentation + "double";	break;
		case spirv_cross::SPIRType::Boolean:	result += indentation + "bool";		break;
		case spirv_cross::SPIRType::UInt:		result += indentation + "uint32_t";	break;
		default: break;
	}

	result += (builtinType.vecsize > 1 ? std::to_string(builtinType.vecsize) : "") + (builtinType.columns > 1 ? ("x" + std::to_string(builtinType.columns)) : "");
	result += " " + memberName + ";\n";
	return result;
}

std::string CrShaderMetadataBuilder::PrintConstantBufferMemberStruct(const spirv_cross::Compiler& reflection, const spirv_cross::SPIRType& type, const std::string& structTypeName, const std::string& structName, uint32_t indentationLevel)
{
	std::string result;
	std::string structIndentation;
	for (uint32_t i = 0; i < indentationLevel; ++i)
	{
		structIndentation += "\t";
	}

	result += structIndentation + "struct " + structTypeName + "\n" + structIndentation + "{\n"; // Open the struct

	std::string memberIndentation = structIndentation + "\t";

	for (uint32_t i = 0; i < type.member_types.size(); ++i)
	{
		const spirv_cross::SPIRType& memberType = reflection.get_type(type.member_types[i]);
		const std::string& memberTypeName = reflection.get_name(memberType.self);
		const std::string& memberName = reflection.get_member_name(type.self, i);

		switch (memberType.basetype)
		{
			case spirv_cross::SPIRType::Struct:
				result += PrintConstantBufferMemberStruct(reflection, memberType, memberTypeName, memberName, indentationLevel + 1); break;
			default:
				result += PrintConstantBufferMemberBuiltIn(memberType, memberName, memberIndentation); break;
		}
	}

	result += structIndentation + "};\n\n"; // Close the struct

	if (structName != "")
	{
		result += structIndentation + structTypeName + " " + structName + ";\n"; // Declare a variable of the struct with a given name
	}

	return result;
}

std::string CrShaderMetadataBuilder::PrintResourceMetadataInstanceDeclaration(const std::string& resourceType, const SPIRVCrossResourceVector& resources)
{
	std::string result;
	for (const auto& uniformBuffer : resources) { result += "extern " + resourceType + "Metadata " + uniformBuffer.name + "MetaInstance;\n"; }
	result += "extern " + resourceType + "Metadata Invalid" + resourceType + "MetaInstance;\n";
	return result + "\n";
}

std::string CrShaderMetadataBuilder::PrintConstantBufferStructMetadata(const std::string& name, int index)
{
	std::string result;
	result += "struct " + name + " : public " + name + "Data\n{\n";
	result += "\tenum { index = " + std::to_string(index) + " };\n";
	result += "\tenum { size = sizeof(" + name + "Data) };\n";
	result += "\tusing Data = " + name + "Data;\n";
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

std::string CrShaderMetadataBuilder::PrintConstantBufferMetadataInstanceDefinition(const SPIRVCrossResourceVector& uniformBuffers)
{
	std::string result;
	for (const auto& uniformBuffer : uniformBuffers)
	{
		auto scopedMeta = [&](const std::string& member) { return uniformBuffer.name + "::" + member; }; // Convenience lambda
		result += "ConstantBufferMetadata " + uniformBuffer.name + "MetaInstance(" + scopedMeta("index, ") + scopedMeta("size") + ");\n";
	}
	result += "ConstantBufferMetadata InvalidConstantBufferMetaInstance(UINT32_MAX, 0);\n";
	return result + "\n";
}

std::string CrShaderMetadataBuilder::PrintConstantBufferGlobalGroupDeclaration(const SPIRVCrossResourceVector& uniformBuffers)
{
	std::string result;
	for (const auto& uniformBuffer : uniformBuffers) { result += "extern CrConstantBuffer<" + uniformBuffer.name + "> cb_" + uniformBuffer.name + ";\n"; }
	return result + "\n";
}

std::string CrShaderMetadataBuilder::PrintConstantBufferGlobalGroupDefinition(const SPIRVCrossResourceVector& uniformBuffers)
{
	std::string result;
	for (const auto& uniformBuffer : uniformBuffers) { result += "CrConstantBuffer<" + uniformBuffer.name + "> cb_" + uniformBuffer.name + ";\n"; }
	return result + "\n";
}

std::string CrShaderMetadataBuilder::BuildTextureMetadataHeader(spirv_cross::Compiler& reflection)
{
	const spirv_cross::ShaderResources& resources = reflection.get_shader_resources();

	std::string result;
	result += TextureSection;

	result += PrintResourceEnum(reflection, "Texture", resources.separate_images);

	result += PrintTextureMetadataStructDeclaration();

	result += PrintResourceMetadataInstanceDeclaration("Texture", resources.separate_images);

	result += "extern CrHashMap<CrString, TextureMetadata&> TextureTable;\n\n";

	result += "extern CrArray<TextureMetadata, " + std::to_string(resources.separate_images.size()) + "> TextureMetaTable;\n\n";

	return result;
}

std::string CrShaderMetadataBuilder::BuildTextureMetadataCpp(spirv_cross::Compiler& reflection)
{
	const spirv_cross::ShaderResources& resources = reflection.get_shader_resources();

	std::string result;

	result += TextureSection;

	result += PrintTextureMetadataInstanceDefinition(resources.separate_images);

	result += PrintResourceHashmap(reflection, "Texture", resources.separate_images);

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

std::string CrShaderMetadataBuilder::PrintTextureMetadataInstanceDefinition(const SPIRVCrossResourceVector& textures)
{
	std::string result;
	for (const auto& texture : textures) { result += "TextureMetadata " + texture.name + "MetaInstance(" + "Textures::" + texture.name + ");\n"; }
	result += "TextureMetadata InvalidTextureMetaInstance(UINT32_MAX);\n";
	return result + "\n";
}

std::string CrShaderMetadataBuilder::BuildSamplerMetadataHeader(spirv_cross::Compiler& reflection)
{
	const spirv_cross::ShaderResources& resources = reflection.get_shader_resources();

	std::string result;

	result += SamplerSection;

	result += PrintResourceEnum(reflection, "Sampler", resources.separate_samplers);

	result += PrintSamplerMetadataStructDeclaration();

	result += PrintResourceMetadataInstanceDeclaration("Sampler", resources.separate_samplers);
	
	result += "extern CrHashMap<CrString, SamplerMetadata&> SamplerTable;\n\n";

	result += "extern CrArray<SamplerMetadata, " + std::to_string(resources.separate_samplers.size()) + "> SamplerMetaTable;\n\n";

	return result;
}

std::string CrShaderMetadataBuilder::BuildSamplerMetadataCpp(spirv_cross::Compiler& reflection)
{
	const spirv_cross::ShaderResources& resources = reflection.get_shader_resources();

	std::string result;

	result += SamplerSection;

	result += PrintSamplerMetadataInstanceDefinition(resources.separate_samplers);

	result += PrintResourceHashmap(reflection, "Sampler", resources.separate_samplers);

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

std::string CrShaderMetadataBuilder::PrintSamplerMetadataInstanceDefinition(const SPIRVCrossResourceVector& samplers)
{
	std::string result;
	for (const auto& sampler : samplers) { result += "SamplerMetadata " + sampler.name + "MetaInstance(" + "Samplers::" + sampler.name + ");\n"; }
	result += "SamplerMetadata InvalidSamplerMetaInstance(UINT32_MAX);\n";
	return result + "\n";
}

std::string CrShaderMetadataBuilder::BuildStorageImageMetadataHeader(spirv_cross::Compiler& reflection)
{
	const spirv_cross::ShaderResources& resources = reflection.get_shader_resources();

	std::string result;

	result += StorageImageSection;
	
	result += PrintResourceEnum(reflection, "StorageImage", resources.storage_images);
	
	//result += PrintSamplerMetadataStructDeclaration();

	//result += PrintResourceMetadataInstanceDeclaration("Sampler", resources.separate_samplers);

	//result += "extern CrHashMap<CrString, SamplerMetadata&> SamplerTable;\n\n";

	return result;
}

std::string CrShaderMetadataBuilder::BuildStorageBufferMetadataHeader(spirv_cross::Compiler& reflection)
{
	const spirv_cross::ShaderResources& resources = reflection.get_shader_resources();

	std::string result;

	result += StorageBufferSection;

	result += PrintResourceEnum(reflection, "StorageBuffer", resources.storage_buffers);

	//result += PrintSamplerMetadataStructDeclaration();

	//result += PrintResourceMetadataInstanceDeclaration("Sampler", resources.separate_samplers);

	//result += "extern CrHashMap<CrString, SamplerMetadata&> SamplerTable;\n\n";

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
