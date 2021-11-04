#include <fstream>
#include <sstream>

#include "CrShaderCompiler.h"
#include "CrCompilerDXC.h"

#include "Rendering/CrRendering.h"

#include "Core/Process/CrProcess.h"
#include "Core/FileSystem/ICrFile.h"
#include "Core/Containers/CrArray.h"
#include "Core/CrGlobalPaths.h"

const char* GetDXCShaderProfile(cr3d::ShaderStage::T shaderStage)
{
	switch (shaderStage)
	{
		case cr3d::ShaderStage::Vertex:   return "vs_6_0";
		case cr3d::ShaderStage::Geometry: return "gs_6_0";
		case cr3d::ShaderStage::Hull:     return "hs_6_0";
		case cr3d::ShaderStage::Domain:   return "ds_6_0";
		case cr3d::ShaderStage::Pixel:    return "ps_6_0";
		case cr3d::ShaderStage::Compute:  return "cs_6_0";
		default: return "";
	}
}

static std::string FindDXCPath()
{
	CrArray<std::string, 2> candidatePaths = 
	{
		CrShaderCompiler::GetExecutableDirectory() + "DXC/DXC.exe",
		std::string(CrGlobalPaths::GetShaderCompilerDirectory().c_str()) + "DXC/DXC.exe"
	};

	for (const std::string& dxcPath : candidatePaths)
	{
		if (ICrFile::FileExists(dxcPath.c_str()))
		{
			return dxcPath;
		}
	}

	return "";
}

void CrCompilerDXC::CreateCommonCommandLine(const CompilationDescriptor& compilationDescriptor, CrFixedString512& commandLine)
{
	std::string dxcPath = FindDXCPath();

	CrProcessDescriptor processDescriptor;
	commandLine += dxcPath.c_str();
	commandLine += " -O3 ";

	commandLine += "-E ";
	commandLine += compilationDescriptor.entryPoint.c_str();
	commandLine += " ";

	commandLine += "-T ";
	commandLine += GetDXCShaderProfile(compilationDescriptor.shaderStage);
	commandLine += " ";

	commandLine += "-Fo \"";
	commandLine += compilationDescriptor.outputPath.c_str();
	commandLine += "\" ";

	commandLine += "\"";
	commandLine += compilationDescriptor.inputPath.c_str();
	commandLine += "\" ";

	for (uint32_t i = 0; i < compilationDescriptor.defines.size(); ++i)
	{
		const CrString& define = compilationDescriptor.defines[i];
		commandLine += "-D ";
		commandLine += define.c_str();
		commandLine += " ";
	}
}

bool CrCompilerDXC::HLSLtoSPIRV(const CompilationDescriptor& compilationDescriptor, std::string& compilationStatus)
{
	CrProcessDescriptor processDescriptor;
	CreateCommonCommandLine(compilationDescriptor, processDescriptor.commandLine);

	processDescriptor.commandLine += " -spirv ";

	CrProcess compilerProcess(processDescriptor);
	compilerProcess.Wait();

	if (compilerProcess.GetReturnValue() != 0)
	{
		CrArray<char, 2048> processOutput;
		compilerProcess.ReadStdOut(processOutput.data(), processOutput.size());
		compilationStatus += processOutput.data();
	}

	return compilerProcess.GetReturnValue() == 0;
}

bool CrCompilerDXC::HLSLtoDXIL(const CompilationDescriptor& compilationDescriptor, std::string& compilationStatus)
{
	CrProcessDescriptor processDescriptor;
	CreateCommonCommandLine(compilationDescriptor, processDescriptor.commandLine);

	CrProcess compilerProcess(processDescriptor);
	compilerProcess.Wait();

	if (compilerProcess.GetReturnValue() != 0)
	{
		CrArray<char, 2048> processOutput;
		compilerProcess.ReadStdOut(processOutput.data(), processOutput.size());
		compilationStatus += processOutput.data();
	}

	return compilerProcess.GetReturnValue() == 0;
}