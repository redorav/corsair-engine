#include "Rendering/CrRendering_pch.h"

#include "Rendering/ICrRenderDevice.h"
#include "Rendering/CrShaderManager.h"
#include "Rendering/ICrShaderReflection.h"
#include "Rendering/ICrShader.h"
#include "Rendering/CrShaderReflectionHeader.h"
#include "Resource/CrResourceManager.h"

#include "Core/CrMacros.h"
#include "Core/Logging/ICrDebug.h"
#include "Core/CrPlatform.h"
#include "Core/Time/CrTimer.h"
#include "Core/Containers/CrArray.h"
#include "Core/CrGlobalPaths.h"
#include "Core/Streams/CrFileStream.h"

#include "crstl/process.h"

CrShaderManager* ShaderManager;

const char* CrShaderManager::GetShaderBytecodeExtension(cr3d::GraphicsApi::T graphicsApi)
{
	switch (graphicsApi)
	{
		case cr3d::GraphicsApi::Vulkan: return ".spv";
		case cr3d::GraphicsApi::D3D12: return ".bin";
		default: return "";
	}
}

CrGraphicsShaderHandle CrShaderManager::CompileGraphicsShader(const CrShaderCompilationDescriptor& shaderCompilationDescriptor) const
{
	// Create the graphics shader descriptor
	CrGraphicsShaderDescriptor graphicsShaderDescriptor;
	graphicsShaderDescriptor.m_bytecodes.reserve(shaderCompilationDescriptor.GetBytecodeDescriptors().size());

	// Load all the relevant shader bytecodes
	for (const CrShaderBytecodeCompilationDescriptor& bytecodeDescriptor : shaderCompilationDescriptor.GetBytecodeDescriptors())
	{
		CrShaderBytecodeHandle bytecode = CompileShaderBytecode(bytecodeDescriptor, shaderCompilationDescriptor.GetDefines());

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

CrFixedPath CrShaderManager::GetCompiledShadersPath(cr::Platform::T platform, cr3d::GraphicsApi::T graphicsApi) const
{
	CrFixedPath shaderCachePath = CrGlobalPaths::GetTempEngineDirectory() + "Compiled Shaders/";
	CrString folderName = CrString(cr::Platform::ToString(platform)) + "_" + cr3d::GraphicsApi::ToString(graphicsApi) + "/";
	shaderCachePath /= folderName.c_str();
	return shaderCachePath;
}

void CrShaderManager::Initialize(ICrRenderDevice* renderDevice)
{
	CrAssert(renderDevice != nullptr);
	CrAssert(ShaderManager == nullptr);
	ShaderManager = new CrShaderManager(renderDevice);
}

void CrShaderManager::Deinitialize()
{
	CrAssert(ShaderManager != nullptr);
	delete ShaderManager;
}

CrShaderBytecodeHandle CrShaderManager::CompileShaderBytecode(const CrShaderBytecodeCompilationDescriptor& bytecodeDescriptor) const
{
	return CompileShaderBytecode(bytecodeDescriptor, CrShaderCompilerDefines::Dummy);
}

CrShaderBytecodeHandle CrShaderManager::CompileShaderBytecode
(
	const CrShaderBytecodeCompilationDescriptor& bytecodeDescriptor,
	const CrShaderCompilerDefines& defines
) const
{
	CrFixedPath ShaderCacheDirectory = GetCompiledShadersPath(bytecodeDescriptor.platform, bytecodeDescriptor.graphicsApi);

	crstl::create_directories(ShaderCacheDirectory.c_str());

	CrFixedString512 commandLine;

	commandLine += " -input \"";
	commandLine += bytecodeDescriptor.path.c_str();
	commandLine += "\" ";
	
	commandLine += "-entrypoint ";
	commandLine += bytecodeDescriptor.entryPoint.c_str();
	commandLine += " ";

	commandLine += "-stage ";
	commandLine += cr3d::ShaderStage::ToString(bytecodeDescriptor.stage, true);
	commandLine += " ";

	commandLine += "-reflection ";

	CrFixedPath filename = bytecodeDescriptor.path.filename();
	size_t extensionDotPosition = filename.find_last_of(".");
	if (extensionDotPosition != filename.npos)
	{
		filename.resize(extensionDotPosition);
	}

	filename += "_";
	filename += bytecodeDescriptor.entryPoint.c_str();
	filename += GetShaderBytecodeExtension(bytecodeDescriptor.graphicsApi);

	CrFixedString512 outputPath(ShaderCacheDirectory.c_str());
	outputPath += filename.c_str();

	commandLine += "-output \"";
	commandLine += outputPath.c_str();
	commandLine += "\" ";

	commandLine += "-platform ";
	commandLine += cr::Platform::ToString(bytecodeDescriptor.platform, true);
	commandLine += " ";

	commandLine += "-graphicsapi ";
	commandLine += cr3d::GraphicsApi::ToString(bytecodeDescriptor.graphicsApi, true);
	commandLine += " ";

	for (uint32_t i = 0; i < defines.GetDefines().size(); ++i)
	{
		commandLine += " -D ";
		commandLine += defines.GetDefines()[i].c_str();
	}

	CrTimer compilationTime;

	// TODO We need a searching policy here. If we were to distribute this as a build we'd
	// want the shader compiler in a known directory, or several directories that we search
	// The platform-specific compilers also need to be in directories relative to the main one
	crstl::process process(CrGlobalPaths::GetShaderCompilerPath().c_str(), commandLine.c_str());

	if (process.is_launched())
	{
		crstl::process_exit_status exitStatus = process.wait();

		if (exitStatus.get_exit_code() >= 0)
		{
			// Serialize in bytecode
			CrReadFileStream compilationOutput(outputPath.c_str());
			CrShaderBytecodeHandle bytecode = CrShaderBytecodeHandle(new CrShaderBytecode());
			compilationOutput << *bytecode.get();

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
			CrString processOutput;
			processOutput.resize_uninitialized(2048);
			process.read_stdout(processOutput.data(), processOutput.size());
			CrAssertMsg(false, "%s", processOutput.data());
		}
	}
	else
	{
		CrLog("Failed to launch shader compiler process");
	}

	return nullptr;
}

CrShaderManager::CrShaderManager(ICrRenderDevice* renderDevice)
{
	m_renderDevice = renderDevice;
}