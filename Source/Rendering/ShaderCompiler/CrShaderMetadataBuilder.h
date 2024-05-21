#pragma once

#include "Core/CrCoreForwardDeclarations.h"

#include <vector>

struct SpvReflectDescriptorBinding;
struct SpvReflectTypeDescription;

typedef CrVector<SpvReflectDescriptorBinding> ResourceVector;

struct CompilationDescriptor;
struct HLSLResources;

class CrShaderMetadataBuilder
{
public:

	static bool BuildMetadata(const CompilationDescriptor& compilationDescriptor, CrString& compilationStatus);

private:

	// Takes SPIRV as input and creates two text files, one for the header and one for the cpp
	static bool BuildSPIRVMetadata(const std::vector<uint32_t>& spirvBytecode, CrString& metadataHeader, CrString& metadataCpp);

	static CrString PrintResourceMetadataInstanceDeclaration(const CrString& resourceType, const ResourceVector& uniformBuffers);

	// Builds the enum with all the resources
	static CrString PrintResourceEnum(const CrString& resourceTypeName, const ResourceVector& resources);
	static CrString PrintResourceHashmap(const CrString& resourceTypeName, const ResourceVector& resources);

	// Prints out the struct or built-in as it comes from the reflection information
	static CrString PrintMemberBuiltIn(const SpvReflectTypeDescription& type, const CrString& memberName, const CrString& indentation);
	static CrString PrintMemberStruct(const SpvReflectTypeDescription& type, const CrString& structTypeName, const CrString& structName, uint32_t indentationLevel);

	//-----------------
	// Constant Buffers
	//-----------------

	static CrString BuildConstantBufferMetadataHeader(const HLSLResources& resources);
	static CrString BuildConstantBufferMetadataCpp(const HLSLResources& resources);
	static CrString PrintConstantBufferMetadataStructDeclaration();	
	static CrString PrintConstantBufferMetadataInstanceDefinition(const ResourceVector& uniformBuffers);
	static CrString PrintConstantBufferStructMetadata(const CrString& name, int index);

	//---------
	// Samplers
	//---------

	static CrString BuildSamplerMetadataHeader(const HLSLResources& resources);
	static CrString BuildSamplerMetadataCpp(const HLSLResources& resources);
	static CrString PrintSamplerMetadataInstanceDefinition(const ResourceVector& samplers);
	static CrString PrintSamplerMetadataStructDeclaration();

	//---------
	// Textures
	//---------

	static CrString BuildTextureMetadataHeader(const HLSLResources& resources);
	static CrString BuildTextureMetadataCpp(const HLSLResources& resources);
	static CrString PrintTextureMetadataInstanceDefinition(const ResourceVector& textures);
	static CrString PrintTextureMetadataStructDeclaration();

	//------------
	// RW Textures
	//------------

	static CrString BuildRWTextureMetadataHeader(const HLSLResources& resources);
	static CrString BuildRWTextureMetadataCpp(const HLSLResources& resources);
	static CrString PrintRWTextureMetadataInstanceDefinition(const ResourceVector& rwTextures);
	static CrString PrintRWTextureMetadataStructDeclaration();

	//--------
	// Buffers
	//--------

	static CrString BuildStorageBufferMetadataStruct(const CrString bufferName, uint32_t index, const SpvReflectTypeDescription& member, bool rw);

	static CrString BuildStorageBufferMetadataHeader(const HLSLResources& resources);
	static CrString BuildStorageBufferMetadataCpp(const HLSLResources& resources);
	static CrString PrintStorageBufferMetadataInstanceDefinition(const ResourceVector& buffers);
	static CrString PrintStorageBufferMetadataStructDeclaration();

	//-----------
	// RW Buffers
	//-----------

	static CrString BuildRWStorageBufferMetadataHeader(const HLSLResources& resources);
	static CrString BuildRWStorageBufferMetadataCpp(const HLSLResources& resources);
	static CrString PrintRWStorageBufferMetadataInstanceDefinition(const ResourceVector& rwBuffers);
	static CrString PrintRWStorageBufferMetadataStructDeclaration();

	//-------------
	// Data Buffers
	//-------------

	static CrString BuildDataBufferMetadataHeader(const HLSLResources& resources);
	static CrString BuildDataBufferMetadataCpp(const HLSLResources& resources);
	static CrString PrintDataBufferMetadataInstanceDefinition(const ResourceVector& dataBuffers);
	static CrString PrintDataBufferMetadataStructDeclaration();

	//----------------
	// RW Data Buffers
	//----------------

	static CrString BuildRWTypedBufferMetadataHeader(const HLSLResources& resources);
	static CrString BuildRWTypedBufferMetadataCpp(const HLSLResources& resources);
	static CrString PrintRWTypedBufferMetadataInstanceDefinition(const ResourceVector& rwTypedBuffers);
	static CrString PrintRWTypedBufferMetadataStructDeclaration();
};