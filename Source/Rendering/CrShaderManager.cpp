#include "CrRendering_pch.h"

#include "Rendering/ICrRenderDevice.h"
#include "Rendering/CrShaderManager.h"
#include "Rendering/ICrShaderReflection.h"
#include "Rendering/ICrShader.h"
#include "CrResourceManager.h"

#include "Core/CrMacros.h"
#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/FileSystem/ICrFile.h"
#include "Core/Logging/ICrDebug.h"
#include "Core/Process/CrProcess.h"
#include "Core/CrPlatform.h"
#include "Core/Time/CrTimer.h"
#include "Core/Containers/CrArray.h"
#include "Core/CrGlobalPaths.h"
#include "Core/CrPlatform.h"

static CrShaderManager g_shaderManager;

CrShaderManager& CrShaderManager::Get()
{
	return g_shaderManager;
}

CrGraphicsShaderHandle CrShaderManager::CompileGraphicsShader(const CrShaderCompilationDescriptor& shaderCompilationDescriptor) const
{
	// Create the graphics shader descriptor
	CrGraphicsShaderDescriptor graphicsShaderDescriptor;
	graphicsShaderDescriptor.m_bytecodes.reserve(shaderCompilationDescriptor.GetBytecodeDescriptors().size());

	// Load all the relevant shader bytecodes
	for (const CrShaderBytecodeCompilationDescriptor& bytecodeDescriptor : shaderCompilationDescriptor.GetBytecodeDescriptors())
	{
		CrShaderBytecodeSharedHandle bytecode = CompileShaderBytecode(bytecodeDescriptor, shaderCompilationDescriptor.GetDefines());

		graphicsShaderDescriptor.m_bytecodes.push_back(bytecode);
	}

	CrGraphicsShaderHandle graphicsShader = m_renderDevice->CreateGraphicsShader(graphicsShaderDescriptor);

	return graphicsShader;
}

CrComputeShaderHandle CrShaderManager::CompileComputeShader(const CrShaderCompilationDescriptor& shaderCompilationDescriptor) const
{
	const CrShaderBytecodeCompilationDescriptor& bytecodeDescriptor = shaderCompilationDescriptor.GetBytecodeDescriptors()[0];

	CrComputeShaderDescriptor computeShaderDescriptor;
	computeShaderDescriptor.m_bytecode = CompileShaderBytecode(bytecodeDescriptor, shaderCompilationDescriptor.GetDefines());

	CrComputeShaderHandle computeShader = m_renderDevice->CreateComputeShader(computeShaderDescriptor);

	return computeShader;
}

CrPath CrShaderManager::GetShaderCachePath(cr::Platform::T platform, cr3d::GraphicsApi::T graphicsApi) const
{
	CrPath shaderCachePath = CrGlobalPaths::GetTempEngineDirectory() + "ShaderCache/";
	CrString folderName = CrString(cr::Platform::ToString(platform)) + "_" + cr3d::GraphicsApi::ToString(graphicsApi) + "/";
	shaderCachePath /= folderName.c_str();
	return shaderCachePath;
}

void CrShaderManager::Initialize(const ICrRenderDevice* renderDevice)
{
	m_renderDevice = renderDevice;
}

CrShaderBytecodeSharedHandle CrShaderManager::CompileShaderBytecode(const CrShaderBytecodeCompilationDescriptor& bytecodeDescriptor) const
{
	return CompileShaderBytecode(bytecodeDescriptor, CrShaderDefines::Dummy);
}

CrShaderBytecodeSharedHandle CrShaderManager::CompileShaderBytecode
(
	const CrShaderBytecodeCompilationDescriptor& bytecodeDescriptor,
	const CrShaderDefines& defines
) const
{
	CrPath ShaderCacheDirectory = GetShaderCachePath(bytecodeDescriptor.platform, bytecodeDescriptor.graphicsApi);

	ICrFile::CreateDirectories(ShaderCacheDirectory.c_str());

	CrFixedString512 outputPath = ShaderCacheDirectory.c_str();

	CrProcessDescriptor processDescriptor;

	// TODO We need a searching policy here. If we were to distribute this as a build we'd
	// want the shader compiler in a known directory, or several directories that we search
	// The platform-specific compilers also need to be in directories relative to the main one
	processDescriptor.commandLine += CrGlobalPaths::GetShaderCompilerPath().c_str();

	processDescriptor.commandLine += " -input \"";
	processDescriptor.commandLine += bytecodeDescriptor.path.c_str();
	processDescriptor.commandLine += "\" ";
		
	processDescriptor.commandLine += "-entrypoint ";
	processDescriptor.commandLine += bytecodeDescriptor.entryPoint.c_str();
	processDescriptor.commandLine += " ";

	processDescriptor.commandLine += "-stage ";
	processDescriptor.commandLine += cr3d::ShaderStage::ToString(bytecodeDescriptor.stage);
	processDescriptor.commandLine += " ";

	// TODO Replace with CrPath
	CrFixedString128 filename = bytecodeDescriptor.path.filename().c_str();
	size_t extensionDotPosition = filename.find_last_of(".");
	if (extensionDotPosition != filename.npos)
	{
		filename.resize(extensionDotPosition);
	}

	filename += "_";
	filename += bytecodeDescriptor.entryPoint.c_str();
	filename += ".spv"; // TODO Fix extension based on API/platform

	outputPath += filename.c_str();

	processDescriptor.commandLine += "-output \"";
	processDescriptor.commandLine += outputPath.c_str();
	processDescriptor.commandLine += "\" ";

	processDescriptor.commandLine += "-platform ";
	processDescriptor.commandLine += cr::Platform::ToString(bytecodeDescriptor.platform);
	processDescriptor.commandLine += " ";

	processDescriptor.commandLine += "-graphicsapi ";
	processDescriptor.commandLine += cr3d::GraphicsApi::ToString(bytecodeDescriptor.graphicsApi);

	for (uint32_t i = 0; i < defines.defines.size(); ++i)
	{
		processDescriptor.commandLine += " -D ";
		processDescriptor.commandLine += defines.defines[i].c_str();
	}

	CrTimer compilationTime;

	CrProcess process(processDescriptor);
	process.Wait();

	if (process.GetReturnValue() >= 0)
	{
		CrFileSharedHandle compilationOutput = ICrFile::OpenFile(outputPath.c_str(), FileOpenFlags::Read);

		// Generate the SPIR-V bytecode
		CrVector<unsigned char> spirvBytecodeBytes;
		spirvBytecodeBytes.resize(compilationOutput->GetSize());

		compilationOutput->Read(spirvBytecodeBytes.data(), spirvBytecodeBytes.size());

		CrShaderBytecodeSharedHandle bytecode = CrShaderBytecodeSharedHandle(new CrShaderBytecode
		(
			std::move(spirvBytecodeBytes),
			bytecodeDescriptor.entryPoint,
			bytecodeDescriptor.stage
		));

		CrLog("Compiled %s [%s] for %s %s (%f ms)",
			bytecodeDescriptor.entryPoint.c_str(),
			bytecodeDescriptor.path.c_str(),
			cr::Platform::ToString(bytecodeDescriptor.platform),
			cr3d::GraphicsApi::ToString(bytecodeDescriptor.graphicsApi),
			compilationTime.GetCurrent().AsMilliseconds());

		return bytecode;
	}
	else
	{
		CrArray<char, 2048> processOutput;
		process.ReadStdOut(processOutput.data(), processOutput.size());
		CrLog(processOutput.data());

		return nullptr;
	}
}