﻿#include "CrShaderCompiler.h"
#include "CrShaderMetadataBuilder.h"

#include "Core/CrPlatform.h"
#include "Core/FileSystem/ICrFile.h"
#include "Core/SmartPointers/CrSharedPtr.h"

#include "CrCompilerGLSLANG.h"
#include "CrCompilerDXC.h"

#include "GlobalVariables.h"

#include <argh.h> // TODO Move to core

#if defined(_WIN32)
#include <windows.h>
#include <shellapi.h>
#endif

#pragma warning (push, 0)
// Glslang
#include <glslang/Public/ShaderLang.h>
#pragma warning (pop)

#include "Core/FileSystem/CrFileSystem.h"

std::string CrShaderCompiler::ExecutablePath;

const std::string& CrShaderCompiler::GetExecutablePath()
{
	return ExecutablePath;
}

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
	// The graphics API will tell us which compilation pipeline we want to use. The compilationDescriptor
	// also includes which platform we want to be targeting so that we can make more informed decisions
	// within (such as what level of support to expect e.g. in Vulkan, etc)
	switch (compilationDescriptor.graphicsApi)
	{
		case cr3d::GraphicsApi::Vulkan:
		{
			CrCompilerDXC::HLSLtoSPIRV(compilationDescriptor);
			break;
		}
		case cr3d::GraphicsApi::D3D12:
		{
			//CrCompilerDXC::HLSLtoDXIL(compilationDescriptor);
			break;
		}
	}
}

static cr3d::ShaderStage::T ParseShaderStage(const std::string& stageString)
{
	if (stageString == "vertex")
	{
		return cr3d::ShaderStage::Vertex;
	}
	else if (stageString == "pixel")
	{
		return cr3d::ShaderStage::Pixel;
	}
	else if (stageString == "hull")
	{
		return cr3d::ShaderStage::Hull;
	}
	else if (stageString == "domain")
	{
		return cr3d::ShaderStage::Domain;
	}
	else if (stageString == "geometry")
	{
		return cr3d::ShaderStage::Geometry;
	}
	else if (stageString == "compute")
	{
		return cr3d::ShaderStage::Compute;
	}

	return cr3d::ShaderStage::Count;
}

static cr::Platform::T ParsePlatform(const std::string& platformString)
{
	if (platformString == "windows")
	{
		return cr::Platform::Windows;
	}
	else
	{
		return cr::Platform::Count;
	}
}

static cr3d::GraphicsApi::T ParseGraphicsApi(const std::string& graphicsApiString)
{
	if (graphicsApiString == "vulkan")
	{
		return cr3d::GraphicsApi::Vulkan;
	}
	else if (graphicsApiString == "d3d12")
	{
		return cr3d::GraphicsApi::D3D12;
	}
	else
	{
		return cr3d::GraphicsApi::Count;
	}
}

struct CommandLineArguments
{
	CommandLineArguments(int argcOriginal, char* argvOriginal[]) : argc(argcOriginal), argv(argvOriginal)
	{
		// Unix systems work in UTF-8 by default and have no problem inputting a properly encoded argv.
		// On Windows, however, directories do not come in as UTF-8 so we need to translate the wchar
		// version of the parameters to a Unicode (multibyte in Windows parlance) version. Because of
		// that, we also need to manage the memory for that fixed command line
		#if defined(_WIN32)

			argv = new char* [argcOriginal + 1];
			argv[argcOriginal] = nullptr;
			m_managedMemory = true;

			// Get the wide command line provided by Windows
			int wargc;
			wchar_t** wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);

			// Convert to UTF-8 and store locally
			for (int i = 0; i < wargc; ++i)
			{
				// Returns size required for the UTF-8 string
				int size = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, nullptr, 0, nullptr, nullptr);

				// Allocate new string (taking care to add the null terminator)
				argv[i] = new char[size];
				argv[i][size - 1] = 0;

				// Convert wide char to multibyte
				WideCharToMultiByte(CP_UTF8, 0, wargv[i], (int)wcslen(wargv[i]), argv[i], size, nullptr, nullptr);
			}

			LocalFree(wargv);

		#endif
	}

	~CommandLineArguments()
	{
		if (m_managedMemory)
		{
			// Delete each string first
			for (int i = 0; argv[i] != nullptr; ++i)
			{
				delete argv[i];
			}

			// Delete the array
			delete[] argv;
		}
	}

	int argc = 0;
	char** argv = nullptr;

private:

	bool m_managedMemory = false;
};

void QuitWithMessage(const std::string& errorMessage)
{
	printf(errorMessage.c_str());
	fflush(stdout);
	exit(-1);
}

// Usage:
// -input sourceFile.hlsl : Source file to be compiled
// -output outputFile.bin : Destination the compiled shader is written to
// -entryPoint main_px    : Entry point for the compiled shader
// -stage pixel           : Shader stage this entry point runs in
// -platform windows      : Platform to compile this shader for
// -graphicsapi vulkan    : Graphics API for this platform
// -D DEFINE1             : Add defines for the compilation
// 
// -metadata metadataPath : Where metadata is stored

int main(int argc, char* argv[])
{
	CommandLineArguments commandLineArguments(argc, argv);
	
	CrPath executablePath = argv[0];
	executablePath.remove_filename();

	CrShaderCompiler::ExecutablePath = executablePath.string();

	argh::parser commandline(commandLineArguments.argc, commandLineArguments.argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);

	CrPath inputFilePath          = commandline("-input").str();
	CrPath outputFilePath         = commandline("-output").str();
	bool buildMetadata            = commandline["-metadata"];
	std::string entryPoint        = commandline("-entrypoint").str();
	std::string shaderStageString = commandline("-stage").str();
	std::string platformString    = commandline("-platform").str();
	std::string graphicsApiString = commandline("-graphicsapi").str();

	std::string inputPath = inputFilePath.string();
	std::string outputPath = outputFilePath.string();

	CrShaderCompiler compiler;
	compiler.Initialize();

	if (entryPoint.empty())
	{
		QuitWithMessage("Error: no entry point specified");
	}

	if (inputPath.empty())
	{
		QuitWithMessage("Error: no input file specified");
	}

	if (outputPath.empty())
	{
		QuitWithMessage("Error: no output file specified");
	}

	// If we've been asked to create metadata
	if (buildMetadata)
	{
		CompilationDescriptor compilationDescriptor;
		compilationDescriptor.inputPath = inputPath;
		compilationDescriptor.outputPath = outputPath;
		compilationDescriptor.entryPoint = entryPoint;
		compilationDescriptor.shaderStage = cr3d::ShaderStage::Pixel;

		CrShaderMetadataBuilder::BuildMetadata(compilationDescriptor);
	}
	else
	{
		cr::Platform::T platform = ParsePlatform(platformString);

		if (platform == cr::Platform::Count)
		{
			QuitWithMessage("No platform specified");
		}

		cr3d::GraphicsApi::T graphicsApi = ParseGraphicsApi(graphicsApiString);

		if (graphicsApi == cr3d::GraphicsApi::Count)
		{
			QuitWithMessage("No graphics API specified");
		}

		cr3d::ShaderStage::T shaderStage = ParseShaderStage(shaderStageString);

		if (shaderStage == cr3d::ShaderStage::Count)
		{
			QuitWithMessage("No shader stage specified");
		}

		CompilationDescriptor compilationDescriptor;
		compilationDescriptor.entryPoint  = entryPoint;
		compilationDescriptor.inputPath   = inputFilePath.string();
		compilationDescriptor.outputPath  = outputFilePath.string();
		compilationDescriptor.shaderStage = shaderStage;
		compilationDescriptor.platform    = platform;
		compilationDescriptor.graphicsApi = graphicsApi;

		compiler.Compile(compilationDescriptor);
	}

	compiler.Finalize();

	return 0;
}