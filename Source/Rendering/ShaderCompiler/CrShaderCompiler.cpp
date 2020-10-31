#include "CrShaderCompiler.h"
#include "CrShaderMetadataBuilder.h"

#include "Core/CrPlatform.h"
#include "Core/FileSystem/ICrFile.h"
#include "Core/SmartPointers/CrSharedPtr.h"

#include "CrSPIRVCompiler.h"

#include <argh.h> // TODO Move to core

#pragma warning (push, 0)
// Glslang
#include <glslang/Public/ShaderLang.h>

// SPIR-V
#include <spirv_cross.hpp>
#include <spirv_cpp.hpp>
#pragma warning (pop)

#include "Core/FileSystem/CrFileSystem.h"

void CrShaderCompiler::Initialize()
{
	glslang::InitializeProcess();
}

void CrShaderCompiler::Finalize()
{
	glslang::FinalizeProcess();
}

void CrShaderCompiler::Compile(const CompilationDescriptor& compilationDescriptor)
{
	switch (compilationDescriptor.platform)
	{
		case cr::Platform::PCVulkan:
			std::vector<uint32_t> spirvBytecode;
			CrSPIRVCompiler::HLSLtoSPIRV(compilationDescriptor, spirvBytecode);

			const char* outputPath = compilationDescriptor.outputPath.c_str();

			// TODO Write to file
			auto file = ICrFile::CreateUnique(outputPath, FileOpenFlags::Write | FileOpenFlags::ForceCreate);
			file->Write(spirvBytecode.data(), spirvBytecode.size() * sizeof(uint32_t));
			break;
	}
}

static cr3d::ShaderStage::T ParseShaderStage(const std::string& stageString)
{
	if (stageString == "pixel")
	{
		return cr3d::ShaderStage::Pixel;
	}
	else if (stageString == "vertex")
	{
		return cr3d::ShaderStage::Vertex;
	}

	return cr3d::ShaderStage::Count;
}

static cr::Platform::T ParsePlatform(const std::string& platformString)
{
	if (platformString == "pcvulkan")
	{
		return cr::Platform::PCVulkan;
	}
	else if (platformString == "pcdx12")
	{
		return cr::Platform::PCDX12;
	}
	else
	{
		return cr::Platform::Count;
	}
}

// Usage:
// -input sourceFile.hlsl	: Source file to be compiled
// -output outputFile.bin	: Destination the compiled shader is written to
// -entryPoint main_px		: Entry point for the compiled shader
// -stage pixel				: Shader stage this entry point runs in
// -metadata metadataPath	: Where metadata is stored
// -platform vulkan_pc		: Platform to compile this shader for
// -D DEFINE1				: Add defines for the compilation
// 
// Example: crshadercompiler.exe -input sourceFile.hlsl -output outputFile.bin -entryPoint main_px -metadata metadataPath -platform vulkan_pc -D DEFINE1

int main(int argc, char** argv)
{
	argh::parser commandline(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);

	CrPath inputFilePath          = commandline("-input").str();
	CrPath outputFilePath         = commandline("-output").str();
	bool buildMetadata            = commandline["-metadata"];
	std::string entryPoint        = commandline("-entrypoint").str();
	std::string shaderStageString = commandline("-stage").str();
	std::string platformString    = commandline("-platform").str();

	CrShaderCompiler compiler;
	compiler.Initialize();

	// If we've been asked to create metadata
	if (buildMetadata)
	{
		CompilationDescriptor compilationDescriptor;
		compilationDescriptor.inputPath = inputFilePath.string();
		compilationDescriptor.outputPath = outputFilePath.string();
		compilationDescriptor.entryPoint = entryPoint;
		compilationDescriptor.shaderStage = cr3d::ShaderStage::Pixel;
		CrShaderMetadataBuilder::BuildMetadata(compilationDescriptor);
	}
	else
	{
		cr::Platform::T platform = ParsePlatform(platformString);

		if (platform == cr::Platform::Count)
		{
			// No platform defined!
			// TODO print an error message
			return 1;
		}

		cr3d::ShaderStage::T shaderStage = ParseShaderStage(shaderStageString);

		if (shaderStage == cr3d::ShaderStage::Count)
		{
			// No stage defined!
			// TODO print an error message
			return 1;
		}

		CompilationDescriptor compilationDescriptor;
		compilationDescriptor.entryPoint = entryPoint;
		compilationDescriptor.inputPath = inputFilePath.string();
		compilationDescriptor.outputPath = outputFilePath.string();
		compilationDescriptor.shaderStage = shaderStage;
		compilationDescriptor.platform = platform;

		compiler.Compile(compilationDescriptor);
	}

	compiler.Finalize();

	return 0;
}