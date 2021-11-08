﻿#include "CrShaderCompiler.h"
#include "CrShaderMetadataBuilder.h"

#include "Core/CrPlatform.h"
#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/Containers/CrPair.h"
#include "Core/Containers/CrHashMap.h"
#include "Core/CrCommandLine.h"
#include "Core/FileSystem/ICrFile.h"
#include "Core/FileSystem/CrPath.h"

#include "CrCompilerGLSLANG.h"
#include "CrCompilerDXC.h"

#if defined(_WIN32)
#include <windows.h>
#include <shellapi.h>
#endif

#pragma warning (push, 0)
// Glslang
#include <glslang/Public/ShaderLang.h>
#pragma warning (pop)

CrString CrShaderCompiler::ExecutableDirectory;

const CrString& CrShaderCompiler::GetExecutableDirectory()
{
	return ExecutableDirectory;
}

void CrShaderCompiler::Initialize()
{
	glslang::InitializeProcess();
}

void CrShaderCompiler::Finalize()
{
	glslang::FinalizeProcess();
}

bool CrShaderCompiler::Compile(const CompilationDescriptor& compilationDescriptor, CrString& compilationStatus)
{
	// The graphics API will tell us which compilation pipeline we want to use. The compilationDescriptor
	// also includes which platform we want to be targeting so that we can make more informed decisions
	// within (such as what level of support to expect e.g. in Vulkan, etc)
	switch (compilationDescriptor.graphicsApi)
	{
		case cr3d::GraphicsApi::Vulkan:
		{
			return CrCompilerDXC::HLSLtoSPIRV(compilationDescriptor, compilationStatus);
		}
		case cr3d::GraphicsApi::D3D12:
		{
			return CrCompilerDXC::HLSLtoDXIL(compilationDescriptor, compilationStatus);
		}
		default:
			break;
	}

	return false;
}

static cr3d::ShaderStage::T ParseShaderStage(const CrString& stageString)
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

static cr::Platform::T ParsePlatform(const CrString& platformString)
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

static cr3d::GraphicsApi::T ParseGraphicsApi(const CrString& graphicsApiString)
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
	printf("%s", errorMessage.c_str());
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
// -D DEFINE1 -D DEFINE2  : Add defines for the compilation
// -reflection            : Append custom reflection to the shader. This makes it unreadable to compilers (e.g. fxc/dxc)
//                          and is for use in the engine. It strips out the built-in reflection data
// 
// -metadata metadataPath : Where C++ metadata is stored

int main(int argc, char* argv[])
{
	CommandLineArguments commandLineArguments(argc, argv);

	CrCommandLineParser commandLine(argc, argv);
	
	CrPath executablePath = argv[0];
	executablePath.remove_filename();

	CrShaderCompiler::ExecutableDirectory = executablePath.c_str();

	CrPath inputFilePath              = commandLine("-input").c_str();
	CrPath outputFilePath             = commandLine("-output").c_str();
	bool buildMetadata                = commandLine["-metadata"];
	bool buildReflection              = commandLine["-reflection"];
	const CrString& entryPoint        = commandLine("-entrypoint").c_str();
	const CrString& shaderStageString = commandLine("-stage").c_str();
	const CrString& platformString    = commandLine("-platform").c_str();
	const CrString& graphicsApiString = commandLine("-graphicsapi").c_str();

	CrVector<CrString> defines;
	commandLine.for_each("-D",[&defines](const CrString& value)
	{
		defines.push_back(value);
	});

	CrString inputPath = inputFilePath.c_str();
	CrString outputPath = outputFilePath.c_str();

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

		switch (shaderStage)
		{
			case cr3d::ShaderStage::Vertex:   defines.push_back("VERTEX_SHADER"); break;
			case cr3d::ShaderStage::Pixel:    defines.push_back("PIXEL_SHADER"); break;
			case cr3d::ShaderStage::Hull:     defines.push_back("HULL_SHADER"); break;
			case cr3d::ShaderStage::Domain:   defines.push_back("DOMAIN_SHADER"); break;
			case cr3d::ShaderStage::Geometry: defines.push_back("GEOMETRY_SHADER"); break;
			case cr3d::ShaderStage::Compute:  defines.push_back("COMPUTE_SHADER"); break;
			default: break;
		}

		CrPath tempPath = outputFilePath;
		tempPath.replace_extension(".temp");

		CompilationDescriptor compilationDescriptor;
		compilationDescriptor.entryPoint      = entryPoint;
		compilationDescriptor.inputPath       = inputFilePath.c_str();
		compilationDescriptor.outputPath      = outputFilePath.c_str();
		compilationDescriptor.tempPath        = tempPath.c_str();
		compilationDescriptor.shaderStage     = shaderStage;
		compilationDescriptor.platform        = platform;
		compilationDescriptor.graphicsApi     = graphicsApi;
		compilationDescriptor.defines         = defines;
		compilationDescriptor.buildReflection = buildReflection;

		CrString compilationStatus;
		bool success = compiler.Compile(compilationDescriptor, compilationStatus);

		if (!success)
		{
			QuitWithMessage(compilationStatus.c_str());
		}
	}

	compiler.Finalize();

	return 0;
}