#include "CrShaderCompiler.h"
#include "CrShaderMetadataBuilder.h"

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

	CrPath inputFilePath	= commandline("-input").str();
	CrPath outputFilePath	= commandline("-output").str();
	CrPath metadataFilePath	= commandline("-metadata").str();
	std::string entryPoint	= commandline("-entrypoint").str();

	//cr::Platform::T platform;
	//string platformString = commandline("-platform").str();
	//
	//if (platformString == "pcvulkan")
	//{
	//	platform = cr::Platform::PCVulkan;
	//}
	//else if (platformString =="pcdx12")
	//{
	//	platform = cr::Platform::PCDX12;
	//}
	//else if (platformString == "metalosx")
	//{
	//	platform = cr::Platform::MetalOSX;
	//}
	//else
	//{
	//	// No platform defined!
	//}

	CrShaderCompiler compiler;
	compiler.Initialize();

	if (!metadataFilePath.empty())
	{
		entryPoint = "metadata";
	}

	//if (commandline["-metadata"])
	{
		CrShaderMetadataBuilder::BuildMetadata(inputFilePath, metadataFilePath, entryPoint);
	}

	compiler.Finalize();

	return 0;
}