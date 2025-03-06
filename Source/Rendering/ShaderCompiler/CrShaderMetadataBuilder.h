#pragma once

#include "Core/CrCoreForwardDeclarations.h"

struct SpvReflectDescriptorBinding;
struct SpvReflectTypeDescription;

typedef CrVector<SpvReflectDescriptorBinding> ResourceVector;

struct CompilationDescriptor;
struct HLSLResources;

class CrShaderMetadataBuilder
{
public:

	static bool BuildMetadata(const CompilationDescriptor& compilationDescriptor, crstl::string& compilationStatus);

private:

	// Takes SPIRV as input and creates two text files, one for the header and one for the cpp
	static bool BuildSPIRVMetadata(const CrVector<uint32_t>& spirvBytecode, crstl::string& metadataHeader, crstl::string& metadataCpp, crstl::string& compilationStatus);

	static crstl::string PrintResourceMetadataInstanceDeclaration(const crstl::string& resourceType, const ResourceVector& uniformBuffers);

	// Builds the enum with all the resources
	static crstl::string PrintResourceEnum(const crstl::string& resourceTypeName, const ResourceVector& resources);
	static crstl::string PrintResourceHashmap(const crstl::string& resourceTypeName, const ResourceVector& resources);

	// Prints out the struct or built-in as it comes from the reflection information
	static crstl::string PrintMemberBuiltIn(const SpvReflectTypeDescription& type, const crstl::string& memberName, const crstl::string& indentation);
	static crstl::string PrintMemberStruct(const SpvReflectTypeDescription& type, const crstl::string& structTypeName, const crstl::string& structName, uint32_t indentationLevel);

	//-----------------
	// Constant Buffers
	//-----------------

	static crstl::string BuildConstantBufferMetadataHeader(const HLSLResources& resources);
	static crstl::string BuildConstantBufferMetadataCpp(const HLSLResources& resources);
	static crstl::string PrintConstantBufferMetadataStructDeclaration();	
	static crstl::string PrintConstantBufferMetadataInstanceDefinition(const ResourceVector& uniformBuffers);
	static crstl::string PrintConstantBufferStructMetadata(const crstl::string& name, int index);

	//---------
	// Samplers
	//---------

	static crstl::string BuildSamplerMetadataHeader(const HLSLResources& resources);
	static crstl::string BuildSamplerMetadataCpp(const HLSLResources& resources);
	static crstl::string PrintSamplerMetadataInstanceDefinition(const ResourceVector& samplers);
	static crstl::string PrintSamplerMetadataStructDeclaration();

	//---------
	// Textures
	//---------

	static crstl::string BuildTextureMetadataHeader(const HLSLResources& resources);
	static crstl::string BuildTextureMetadataCpp(const HLSLResources& resources);
	static crstl::string PrintTextureMetadataInstanceDefinition(const ResourceVector& textures);
	static crstl::string PrintTextureMetadataStructDeclaration();

	//------------
	// RW Textures
	//------------

	static crstl::string BuildRWTextureMetadataHeader(const HLSLResources& resources);
	static crstl::string BuildRWTextureMetadataCpp(const HLSLResources& resources);
	static crstl::string PrintRWTextureMetadataInstanceDefinition(const ResourceVector& rwTextures);
	static crstl::string PrintRWTextureMetadataStructDeclaration();

	//--------
	// Buffers
	//--------

	static crstl::string BuildStorageBufferMetadataStruct(const crstl::string bufferName, uint32_t index, const SpvReflectTypeDescription& member, bool rw);

	static crstl::string BuildStorageBufferMetadataHeader(const HLSLResources& resources);
	static crstl::string BuildStorageBufferMetadataCpp(const HLSLResources& resources);
	static crstl::string PrintStorageBufferMetadataInstanceDefinition(const ResourceVector& buffers);
	static crstl::string PrintStorageBufferMetadataStructDeclaration();

	//-----------
	// RW Buffers
	//-----------

	static crstl::string BuildRWStorageBufferMetadataHeader(const HLSLResources& resources);
	static crstl::string BuildRWStorageBufferMetadataCpp(const HLSLResources& resources);
	static crstl::string PrintRWStorageBufferMetadataInstanceDefinition(const ResourceVector& rwBuffers);
	static crstl::string PrintRWStorageBufferMetadataStructDeclaration();

	//-------------
	// Data Buffers
	//-------------

	static crstl::string BuildTypedBufferMetadataHeader(const HLSLResources& resources);
	static crstl::string BuildTypedBufferMetadataCpp(const HLSLResources& resources);
	static crstl::string PrintTypedBufferMetadataInstanceDefinition(const ResourceVector& typedBuffers);
	static crstl::string PrintTypedBufferMetadataStructDeclaration();

	//----------------
	// RW Data Buffers
	//----------------

	static crstl::string BuildRWTypedBufferMetadataHeader(const HLSLResources& resources);
	static crstl::string BuildRWTypedBufferMetadataCpp(const HLSLResources& resources);
	static crstl::string PrintRWTypedBufferMetadataInstanceDefinition(const ResourceVector& rwTypedBuffers);
	static crstl::string PrintRWTypedBufferMetadataStructDeclaration();
};