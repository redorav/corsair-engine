#pragma once

#include <string>
#include <vector>
#include <filesystem>

using CrPath = std::filesystem::path;

struct SpvReflectDescriptorBinding;
struct SpvReflectTypeDescription;

typedef std::vector<SpvReflectDescriptorBinding> ResourceVector;

struct CompilationDescriptor;
struct HLSLResources;

class CrShaderMetadataBuilder
{
public:

	static bool BuildMetadata(const CompilationDescriptor& compilationDescriptor);

private:

	// Takes SPIRV as input and creates two text files, one for the header and one for the cpp
	static bool BuildSPIRVMetadata(const std::vector<uint32_t>& spirvBytecode, std::string& metadataHeader, std::string& metadataCpp);

	static std::string PrintResourceMetadataInstanceDeclaration(const std::string& resourceType, const ResourceVector& uniformBuffers);

	// Builds the enum with all the resources
	static std::string PrintResourceEnum(const std::string& resourceTypeName, const ResourceVector& resources);
	static std::string PrintResourceHashmap(const std::string& resourceTypeName, const ResourceVector& resources);

	// Prints out the struct or built-in as it comes from the reflection information
	static std::string PrintMemberBuiltIn(const SpvReflectTypeDescription& type, const std::string& memberName, const std::string& indentation);
	static std::string PrintMemberStruct(const SpvReflectTypeDescription& type, const std::string& structTypeName, const std::string& structName, uint32_t indentationLevel);

	//-----------------
	// Constant Buffers
	//-----------------

	static std::string BuildConstantBufferMetadataHeader(const HLSLResources& resources);
	static std::string BuildConstantBufferMetadataCpp(const HLSLResources& resources);
	static std::string PrintConstantBufferMetadataStructDeclaration();	
	static std::string PrintConstantBufferMetadataInstanceDefinition(const ResourceVector& uniformBuffers);
	static std::string PrintConstantBufferStructMetadata(const std::string& name, int index);

	//---------
	// Samplers
	//---------

	static std::string BuildSamplerMetadataHeader(const HLSLResources& resources);
	static std::string BuildSamplerMetadataCpp(const HLSLResources& resources);
	static std::string PrintSamplerMetadataInstanceDefinition(const ResourceVector& samplers);
	static std::string PrintSamplerMetadataStructDeclaration();

	//---------
	// Textures
	//---------

	static std::string BuildTextureMetadataHeader(const HLSLResources& resources);
	static std::string BuildTextureMetadataCpp(const HLSLResources& resources);
	static std::string PrintTextureMetadataInstanceDefinition(const ResourceVector& textures);
	static std::string PrintTextureMetadataStructDeclaration();

	//------------
	// RW Textures
	//------------

	static std::string BuildRWTextureMetadataHeader(const HLSLResources& resources);
	static std::string BuildRWTextureMetadataCpp(const HLSLResources& resources);
	static std::string PrintRWTextureMetadataInstanceDefinition(const ResourceVector& rwTextures);
	static std::string PrintRWTextureMetadataStructDeclaration();

	//--------
	// Buffers
	//--------

	static std::string BuildStorageBufferMetadataStruct(const std::string bufferName, uint32_t index, const SpvReflectTypeDescription& member, bool rw);

	static std::string BuildStorageBufferMetadataHeader(const HLSLResources& resources);
	static std::string BuildStorageBufferMetadataCpp(const HLSLResources& resources);
	static std::string PrintStorageBufferMetadataInstanceDefinition(const ResourceVector& buffers);
	static std::string PrintStorageBufferMetadataStructDeclaration();

	//-----------
	// RW Buffers
	//-----------

	static std::string BuildRWStorageBufferMetadataHeader(const HLSLResources& resources);
	static std::string BuildRWStorageBufferMetadataCpp(const HLSLResources& resources);
	static std::string PrintRWStorageBufferMetadataInstanceDefinition(const ResourceVector& rwBuffers);
	static std::string PrintRWStorageBufferMetadataStructDeclaration();

	//-------------
	// Data Buffers
	//-------------

	static std::string BuildDataBufferMetadataHeader(const HLSLResources& resources);
	static std::string BuildDataBufferMetadataCpp(const HLSLResources& resources);
	static std::string PrintDataBufferMetadataInstanceDefinition(const ResourceVector& dataBuffers);
	static std::string PrintDataBufferMetadataStructDeclaration();

	//----------------
	// RW Data Buffers
	//----------------

	static std::string BuildRWDataBufferMetadataHeader(const HLSLResources& resources);
	static std::string BuildRWDataBufferMetadataCpp(const HLSLResources& resources);
	static std::string PrintRWDataBufferMetadataInstanceDefinition(const ResourceVector& rwDataBuffers);
	static std::string PrintRWDataBufferMetadataStructDeclaration();

	static void WriteToFile(const std::string& filename, const std::string& text);

	static void WriteToFileIfChanged(const std::string& filename, const std::string& text);
};