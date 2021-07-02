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

bool CrCompilerDXC::HLSLtoSPIRV(const CompilationDescriptor& compilationDescriptor, std::string& compilationStatus)
{
	std::string dxcPath = FindDXCPath();

	CrProcessDescriptor processDescriptor;
	processDescriptor.commandLine += dxcPath.c_str();
	processDescriptor.commandLine += " -spirv -O3 ";

	processDescriptor.commandLine += "-E ";
	processDescriptor.commandLine += compilationDescriptor.entryPoint.c_str();
	processDescriptor.commandLine += " ";

	processDescriptor.commandLine += "-T ";
	processDescriptor.commandLine += GetDXCShaderProfile(compilationDescriptor.shaderStage);
	processDescriptor.commandLine += " ";

	processDescriptor.commandLine += "-Fo \"";
	processDescriptor.commandLine += compilationDescriptor.outputPath.c_str();
	processDescriptor.commandLine += "\" ";

	processDescriptor.commandLine += "\"";
	processDescriptor.commandLine += compilationDescriptor.inputPath.c_str();
	processDescriptor.commandLine += "\" ";

	for (uint32_t i = 0; i < compilationDescriptor.defines.size(); ++i)
	{
		const CrString& define = compilationDescriptor.defines[i];
		processDescriptor.commandLine += "-D ";
		processDescriptor.commandLine += define.c_str();
		processDescriptor.commandLine += " ";
	}

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