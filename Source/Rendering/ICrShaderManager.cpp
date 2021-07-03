#include "CrRendering_pch.h"

#include "Rendering/ShaderCompiler/CrShaderCompilerCommandLine.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/ICrShaderManager.h"
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

// TODO Delete
static ICrShaderManager g_shaderManager;

ICrShaderManager* ICrShaderManager::Get()
{
	return &g_shaderManager;
}

CrShaderBytecodeSharedHandle ICrShaderManager::LoadShaderBytecode(const CrPath& path, const CrShaderBytecodeDescriptor& bytecodeDescriptor) const
{
	CrFileSharedHandle file = ICrFile::OpenFile(path.c_str(), FileOpenFlags::Read);
	return LoadShaderBytecode(file, bytecodeDescriptor);
}

CrShaderBytecodeSharedHandle ICrShaderManager::LoadShaderBytecode(const CrFileSharedHandle& file, const CrShaderBytecodeDescriptor& bytecodeDescriptor) const
{
	switch (bytecodeDescriptor.format)
	{
		case cr3d::ShaderCodeFormat::Binary:
		{
			CrShaderBytecodeSharedHandle bytecode;
			{
				CrVector<unsigned char> bytecodeData;
				bytecodeData.resize(file->GetSize());
				file->Read(bytecodeData.data(), bytecodeData.size());
				bytecode = CrShaderBytecodeSharedHandle(new CrShaderBytecode(std::move(bytecodeData), bytecodeDescriptor.entryPoint, bytecodeDescriptor.stage));
			}
			return bytecode;
		}
		case cr3d::ShaderCodeFormat::SourceHLSL:
		{
			return CompileShaderBytecode(bytecodeDescriptor);
		}
	}

	return nullptr;
}

CrGraphicsShaderHandle ICrShaderManager::LoadGraphicsShader(const CrBytecodeLoadDescriptor& bytecodeLoadDescriptor) const
{
	// Create the graphics shader descriptor
	CrGraphicsShaderDescriptor graphicsShaderDescriptor;
	graphicsShaderDescriptor.m_bytecodes.reserve(bytecodeLoadDescriptor.GetBytecodeDescriptors().size());

	// Load all the relevant shader bytecodes
	for (const CrShaderBytecodeDescriptor& bytecodeDescriptor : bytecodeLoadDescriptor.GetBytecodeDescriptors())
	{
		CrShaderBytecodeSharedHandle bytecode = LoadShaderBytecode(bytecodeDescriptor.path, bytecodeDescriptor);

		graphicsShaderDescriptor.m_bytecodes.push_back(bytecode);
	}

	CrGraphicsShaderHandle graphicsShader = m_renderDevice->CreateGraphicsShader(graphicsShaderDescriptor);

	return graphicsShader;
}

CrComputeShaderHandle ICrShaderManager::LoadComputeShader(const CrBytecodeLoadDescriptor& bytecodeLoadDescriptor) const
{
	const CrShaderBytecodeDescriptor& bytecodeDescriptor = bytecodeLoadDescriptor.GetBytecodeDescriptors()[0];

	CrComputeShaderDescriptor computeShaderDescriptor;
	computeShaderDescriptor.m_bytecode = LoadShaderBytecode(bytecodeDescriptor.path, bytecodeDescriptor);

	CrComputeShaderHandle computeShader = m_renderDevice->CreateComputeShader(computeShaderDescriptor);

	return computeShader;
}

void ICrShaderManager::Init(const ICrRenderDevice* renderDevice)
{
	m_renderDevice = renderDevice;
}

CrShaderBytecodeSharedHandle ICrShaderManager::CompileShaderBytecode(const CrShaderBytecodeDescriptor& bytecodeDescriptor) const
{
	CrProcessDescriptor processDescriptor;

	// TODO We need a searching policy here. If we were to distribute this as a build we'd
	// want the shader compiler in a known directory, or several directories that we search
	// The platform-specific compilers also need to be in directories relative to the main one
	processDescriptor.commandLine += CrGlobalPaths::GetShaderCompilerPath().c_str();

	processDescriptor.commandLine += " -input ";
	processDescriptor.commandLine += bytecodeDescriptor.path.c_str();
	processDescriptor.commandLine += " ";
		
	processDescriptor.commandLine += "-entrypoint ";
	processDescriptor.commandLine += bytecodeDescriptor.entryPoint.c_str();
	processDescriptor.commandLine += " ";

	processDescriptor.commandLine += "-stage ";
	processDescriptor.commandLine += CrShaderCompilerCommandLine::GetShaderStage(bytecodeDescriptor.stage);
	processDescriptor.commandLine += " ";

	const CrString& ShaderCacheDirectory = CrGlobalPaths::GetTempEngineDirectory() + "ShaderCache/";

	ICrFile::CreateDirectories(ShaderCacheDirectory.c_str());

	CrFixedString512 outputPath = ShaderCacheDirectory.c_str();

	CrFixedString128 filename = bytecodeDescriptor.path.filename().c_str();
	size_t extensionDotPosition = filename.find_last_of(".");
	if (extensionDotPosition != filename.npos)
	{
		filename.resize(extensionDotPosition);
	}

	filename += "_";
	filename += bytecodeDescriptor.entryPoint.c_str();
	filename += ".spv";

	outputPath += filename.c_str();

	processDescriptor.commandLine += "-output \"";
	processDescriptor.commandLine += outputPath.c_str();
	processDescriptor.commandLine += "\" ";

	processDescriptor.commandLine += "-platform ";
	processDescriptor.commandLine += CrShaderCompilerCommandLine::GetPlatform(bytecodeDescriptor.platform);
	processDescriptor.commandLine += " ";

	processDescriptor.commandLine += "-graphicsapi ";
	processDescriptor.commandLine += cr3d::GraphicsApi::ToString(bytecodeDescriptor.graphicsApi);

	for (uint32_t i = 0; i < bytecodeDescriptor.defines.size(); ++i)
	{
		processDescriptor.commandLine += " -D ";
		processDescriptor.commandLine += bytecodeDescriptor.defines[i].c_str();
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

		CrLog("Compiled %s[%s] for %s %s (%f ms)",
			bytecodeDescriptor.entryPoint.c_str(),
			bytecodeDescriptor.path.c_str(),
			CrShaderCompilerCommandLine::GetPlatform(bytecodeDescriptor.platform),
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