#pragma once

#include <string>
#include <vector>
#include <filesystem>

using CrPath = std::filesystem::path;

namespace spirv_cross
{
	class Compiler;
	struct Resource;
	struct SPIRType;

	template <typename T, size_t N = 8> class SmallVector;
}

typedef spirv_cross::SmallVector<spirv_cross::Resource> SPIRVCrossResourceVector;

class CrShaderMetadataBuilder
{
public:

	static bool BuildMetadata(const CrPath& inputHLSL, const CrPath& outputFilename, const std::string& entryPoint);

private:

	// Takes SPIRV as input and creates two text files, one for the header and one for the cpp
	static bool BuildSPIRVMetadata(const std::vector<uint32_t>& spirvBytecode, std::string& metadataHeader, std::string& metadataCpp);

	static std::string PrintResourceMetadataInstanceDeclaration(const std::string& resourceType, const SPIRVCrossResourceVector& uniformBuffers);

	// Builds the enum with all the resources
	static std::string PrintResourceEnum(spirv_cross::Compiler& reflection, const std::string& resourceType, const SPIRVCrossResourceVector& resources);

	static std::string PrintResourceHashmap(spirv_cross::Compiler& reflection, const std::string& resourceType, const SPIRVCrossResourceVector& resources);

	//-----------------
	// Constant Buffers
	//-----------------

	static std::string BuildConstantBufferMetadataHeader(spirv_cross::Compiler& reflection);
	static std::string BuildConstantBufferMetadataCpp(spirv_cross::Compiler& reflection);

	// Prints out the struct or built-in as it comes from the reflection information
	static std::string PrintConstantBufferMemberBuiltIn(spirv_cross::SPIRType builtinType, const std::string& memberName, const std::string& indentation);
	static std::string PrintConstantBufferMemberStruct(const spirv_cross::Compiler& reflection, const spirv_cross::SPIRType& type, const std::string& structTypeName, const std::string& structName, uint32_t indentationLevel);

	static std::string PrintConstantBufferStructMetadata(const std::string& name, int index);
	static std::string PrintConstantBufferMetadataStructDeclaration();
	
	static std::string PrintConstantBufferMetadataInstanceDefinition(const SPIRVCrossResourceVector& uniformBuffers);

	static std::string PrintConstantBufferGlobalGroupDeclaration(const SPIRVCrossResourceVector& uniformBuffers);
	static std::string PrintConstantBufferGlobalGroupDefinition(const SPIRVCrossResourceVector& uniformBuffers);

	//---------
	// Textures
	//---------

	static std::string BuildTextureMetadataHeader(spirv_cross::Compiler& reflection);
	static std::string BuildTextureMetadataCpp(spirv_cross::Compiler& reflection);

	static std::string PrintTextureMetadataStructDeclaration();
	static std::string PrintTextureMetadataInstanceDefinition(const SPIRVCrossResourceVector& textures);

	//---------
	// Samplers
	//---------

	static std::string BuildSamplerMetadataHeader(spirv_cross::Compiler& reflection);
	static std::string BuildSamplerMetadataCpp(spirv_cross::Compiler& reflection);

	static std::string PrintSamplerMetadataStructDeclaration();
	static std::string PrintSamplerMetadataInstanceDefinition(const SPIRVCrossResourceVector& samplers);

	//---------------
	// Storage Images
	//---------------

	static std::string BuildStorageImageMetadataHeader(spirv_cross::Compiler& reflection);

	//----------------
	// Storage Buffers
	//----------------

	static std::string BuildStorageBufferMetadataHeader(spirv_cross::Compiler& reflection);

	static void WriteToFile(const std::string& filename, const std::string& text);

	static void WriteToFileIfChanged(const std::string& filename, const std::string& text);
};