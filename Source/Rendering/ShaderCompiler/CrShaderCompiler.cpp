#include "Rendering/ShaderCompiler/CrShaderCompiler_pch.h"

#include "CrShaderCompiler.h"
#include "CrShaderMetadataBuilder.h"
#include "CrBuiltinShaderBuilder.h"
#include "CrShaderCompilerUtilities.h"

#include "Core/CrPlatform.h"
#include "Core/Containers/CrHashMap.h"
#include "Core/CrCommandLine.h"
#include "Core/FileSystem/CrFixedPath.h"
#include "Core/Logging/ICrDebug.h"
#include "Core/Function/CrFixedFunction.h"
#include "Core/String/CrStringUtilities.h"
#include "Core/CrMacros.h"
#include "Core/CrGlobalPaths.h"

#include "crstl/filesystem.h"

#include "CrCompilerDXC.h"

#if defined(_WIN32)
#include <windows.h>
#include <shellapi.h>
#endif

void CompilationDescriptor::Process() const
{
	if (!processed)
	{
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

		switch (platform)
		{
			case cr::Platform::Windows: defines.push_back("WINDOWS_TARGET"); break;
			default: break;
		}

		switch (graphicsApi)
		{
			case cr3d::GraphicsApi::Vulkan: defines.push_back("VULKAN_API"); break;
			case cr3d::GraphicsApi::D3D12: defines.push_back("D3D12_API"); break;
			default: break;
		}

		processed = true;
	}
}

crstl::string CrShaderCompiler::ExecutableDirectory;

CrFixedPath CrShaderCompiler::PDBDirectory;

CrFixedPath CrShaderCompiler::PDBDirectories[cr::Platform::Count][cr3d::GraphicsApi::Count];

const crstl::string& CrShaderCompiler::GetExecutableDirectory()
{
	return ExecutableDirectory;
}

const CrFixedPath& CrShaderCompiler::GetPDBDirectory(cr::Platform::T platform, cr3d::GraphicsApi::T graphicsApi)
{
	return PDBDirectories[platform][graphicsApi];
}

void CrShaderCompiler::Initialize()
{
	
}

void CrShaderCompiler::Finalize()
{
	
}

bool CrShaderCompiler::Compile(const CompilationDescriptor& compilationDescriptor, crstl::string& compilationStatus)
{
	// Patches the compilation descriptor with information derived from the current state (such as defines that identify
	// the shader stage, the platform, etc)
	compilationDescriptor.Process();

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

static cr3d::ShaderStage::T ParseShaderStage(const crstl::string& stageString)
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

static cr::Platform::T ParsePlatform(const crstl::string& platformString)
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

static cr3d::GraphicsApi::T ParseGraphicsApi(const crstl::string& graphicsApiString)
{
	if (graphicsApiString.comparei("vulkan") == 0)
	{
		return cr3d::GraphicsApi::Vulkan;
	}
	else if (graphicsApiString.comparei("d3d12") == 0)
	{
		return cr3d::GraphicsApi::D3D12;
	}
	else
	{
		return cr3d::GraphicsApi::Count;
	}
}

// Usage:
// -input sourceFile.hlsl : Source file to be compiled
// -output outputFile.bin : Destination the compiled shader is written to
// -entryPoint MainPS     : Entry point for the compiled shader
// -stage pixel           : Shader stage this entry point runs in
// -platform windows      : Platform to compile this shader for
// -graphicsapi vulkan    : Graphics API for this platform
// -D DEFINE1 -D DEFINE2  : Add defines for the compilation
// -reflection            : Append custom reflection to the shader. This makes it unreadable to compilers (e.g. fxc/dxc)
//                          and is for use in the engine. It strips out the built-in reflection data
// 
// -metadata metadataPath : Where C++ metadata is stored
// -builtin               : Build builtin shaders
// -builtin-headers       : Build metadata headers for the build to consume. Don't pass in when live recompiling
// -pdb pdbPath           : Create a PDB for this shader and store in pdbPath
//                          The PDB is uniquely identified with a hash stored in the reflection data

int main(int argc, char* argv[])
{
	CrCommandLineParser commandLine(argc, argv);
	
	CrFixedPath executablePath = argv[0];
	executablePath.remove_filename();

	CrShaderCompiler::ExecutableDirectory = executablePath.c_str();

	CrFixedPath inputFilePath              = commandLine("-input").c_str();
	CrFixedPath outputFilePath             = commandLine("-output").c_str();
	bool buildMetadata                     = commandLine["-metadata"];
	bool buildBuiltinShaders               = commandLine["-builtin"];
	bool buildBuiltinHeaders               = commandLine["-builtin-headers"];
	const crstl::string& entryPoint        = commandLine("-entrypoint");
	const crstl::string& shaderStageString = commandLine("-stage");
	const crstl::string& platformString    = commandLine("-platform");

	CrShaderCompiler::PDBDirectory    = commandLine("-pdb").c_str();

	crstl::vector<crstl::string> graphicsApiStrings;
	commandLine.for_each("-graphicsapi", [&graphicsApiStrings](const crstl::string& value)
	{
		graphicsApiStrings.push_back(value);
	});

	crstl::vector<crstl::string> defines;
	commandLine.for_each("-D",[&defines](const crstl::string& value)
	{
		defines.push_back(value);
	});

	// If no PDB directory was supplied, use the temp folder
	if (CrShaderCompiler::PDBDirectory.empty())
	{
		CrShaderCompiler::PDBDirectory = CrGlobalPaths::GetTempEngineDirectory() + "Shader PDBs";
	}

	crstl::string inputPath = inputFilePath.c_str();
	crstl::string outputPath = outputFilePath.c_str();

	if (inputPath.empty())
	{
		CrShaderCompilerUtilities::QuitWithMessage("Error: no input specified\n");
	}

	if (outputPath.empty())
	{
		CrShaderCompilerUtilities::QuitWithMessage("Error: no output specified\n");
	}

	cr::Platform::T platform = ParsePlatform(platformString);

	if (!CrShaderCompiler::PDBDirectory.empty())
	{
		for (const crstl::string& graphicsApiString : graphicsApiStrings)
		{
			cr3d::GraphicsApi::T graphicsApi = ParseGraphicsApi(graphicsApiString);
			CrFixedPath& pdbPath = CrShaderCompiler::PDBDirectories[platform][graphicsApi];
			pdbPath = CrShaderCompiler::PDBDirectory;
			pdbPath /= cr::Platform::ToString(platform);
			pdbPath += "_";
			pdbPath += cr3d::GraphicsApi::ToString(graphicsApi);

			if (!crstl::exists(pdbPath.c_str()))
			{
				crstl::create_directories(pdbPath.c_str());
			}
		}
	}

	CrShaderCompiler::Initialize();

	// If we've been asked to create metadata
	if (buildMetadata)
	{
		if (entryPoint.empty())
		{
			CrShaderCompilerUtilities::QuitWithMessage("No entry point specified\n");
		}

		CompilationDescriptor compilationDescriptor;
		compilationDescriptor.inputPath = inputPath;
		compilationDescriptor.outputPath = outputPath;
		compilationDescriptor.entryPoint = entryPoint;
		compilationDescriptor.shaderStage = cr3d::ShaderStage::Pixel;

		// Make sure resources aren't stripped out
		compilationDescriptor.optimization = OptimizationLevel::None;

		crstl::string compilationStatus;
		bool success = CrShaderMetadataBuilder::BuildMetadata(compilationDescriptor, compilationStatus);

		if (!success)
		{
			CrShaderCompilerUtilities::QuitWithMessage(compilationStatus);
		}
	}
	else if (buildBuiltinShaders)
	{
		if (platform == cr::Platform::Count)
		{
			CrShaderCompilerUtilities::QuitWithMessage("No platform specified\n");
		}

		CrBuiltinShadersDescriptor builtinShadersDescriptor;
		builtinShadersDescriptor.inputPath = inputFilePath;
		builtinShadersDescriptor.outputPath = outputFilePath;
		builtinShadersDescriptor.buildBuiltinHeaders = buildBuiltinHeaders;

		crstl::create_directories(outputFilePath.c_str());

		for (const crstl::string& graphicsApiString : graphicsApiStrings)
		{
			cr3d::GraphicsApi::T graphicsApi = ParseGraphicsApi(graphicsApiString);

			if (graphicsApi == cr3d::GraphicsApi::Count)
			{
				CrShaderCompilerUtilities::QuitWithMessage("No graphics API specified\n");
			}

			builtinShadersDescriptor.graphicsApis.push_back(graphicsApi);
		}

		builtinShadersDescriptor.platform = platform;

		CrBuiltinShaderBuilder::ProcessBuiltinShaders(builtinShadersDescriptor);
	}
	else
	{
		if (platform == cr::Platform::Count)
		{
			CrShaderCompilerUtilities::QuitWithMessage("No platform specified\n");
		}

		cr3d::GraphicsApi::T graphicsApi = ParseGraphicsApi(graphicsApiStrings[0]);

		if (graphicsApi == cr3d::GraphicsApi::Count)
		{
			CrShaderCompilerUtilities::QuitWithMessage("No graphics API specified\n");
		}

		if (entryPoint.empty())
		{
			CrShaderCompilerUtilities::QuitWithMessage("No entry point specified\n");
		}

		cr3d::ShaderStage::T shaderStage = ParseShaderStage(shaderStageString);

		if (shaderStage == cr3d::ShaderStage::Count)
		{
			CrShaderCompilerUtilities::QuitWithMessage("No shader stage specified\n");
		}

		CrFixedPath tempPath = outputFilePath;
		tempPath.replace_extension(".temp");

		CompilationDescriptor compilationDescriptor;
		compilationDescriptor.inputPath       = inputFilePath;
		compilationDescriptor.outputPath      = outputFilePath;
		compilationDescriptor.tempPath        = tempPath;
		compilationDescriptor.entryPoint      = entryPoint;
		compilationDescriptor.defines         = defines;
		compilationDescriptor.platform        = platform;
		compilationDescriptor.graphicsApi     = graphicsApi;
		compilationDescriptor.shaderStage     = shaderStage;

		crstl::string compilationStatus;
		bool success = CrShaderCompiler::Compile(compilationDescriptor, compilationStatus);

		if (!success)
		{
			CrShaderCompilerUtilities::QuitWithMessage(compilationStatus.c_str());
		}
	}

	CrShaderCompiler::Finalize();

	return 0;
}
