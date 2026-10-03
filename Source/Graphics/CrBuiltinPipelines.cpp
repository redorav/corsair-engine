#include "Graphics/CrRendering_pch.h"

#include "Graphics/CrBuiltinPipelines.h"
#include "Graphics/IGraphicsSystem.h"
#include "Graphics/IDevice.h"
#include "Graphics/IShader.h"
#include "Graphics/CrRendererConfig.h"
#include "Graphics/CrCommonVertexLayouts.h"

#include "Core/CrGlobalPaths.h"
#include "crstl/process.h"

#include "GeneratedShaders/BuiltinShaders.h"

CrBuiltinPipelines* BuiltinPipelines;

void CrBuiltinPipelines::Initialize()
{
	CrAssert(BuiltinPipelines == nullptr);
	BuiltinPipelines = new CrBuiltinPipelines();
}

void CrBuiltinPipelines::Deinitialize()
{
	CrAssert(BuiltinPipelines != nullptr);
	delete BuiltinPipelines;
}

CrBuiltinPipelines::CrBuiltinPipelines()
{
	// Builtin ubershaders
	{
		BasicUbershaderForwardShader = CrBuiltinPipelines::GetGraphicsShaderBytecode(CrBuiltinShaders::BasicUbershaderVS, CrBuiltinShaders::BasicForwardUbershaderPS);

		BasicUbershaderGBufferShader = CrBuiltinPipelines::GetGraphicsShaderBytecode(CrBuiltinShaders::BasicUbershaderVS, CrBuiltinShaders::BasicGBufferUbershaderPS);

		BasicUbershaderDebugShader = CrBuiltinPipelines::GetGraphicsShaderBytecode(CrBuiltinShaders::BasicUbershaderVS, CrBuiltinShaders::BasicDebugUbershaderPS);
	}
}

crgfx::GraphicsShaderBytecode CrBuiltinPipelines::GetGraphicsShaderBytecode(CrBuiltinShaders::T vertexShaderIndex, CrBuiltinShaders::T pixelShaderIndex)
{
	crgfx::IDevice* device = crgfx::GetDevice().get();

	const crgfx::DeviceProperties& properties = device->GetProperties();

	const crgfx::ShaderBytecodeHandle& vertexShaderBytecode = crgfx::GetBuiltinShaderBytecode(vertexShaderIndex);
	const crgfx::ShaderBytecodeHandle& pixelShaderBytecode = crgfx::GetBuiltinShaderBytecode(pixelShaderIndex);

	crgfx::ShaderDebugString debugName;
	debugName += CrBuiltinShaders::GetMetadata(vertexShaderIndex, properties.graphicsApi).name.c_str();
	debugName += "_";
	debugName += CrBuiltinShaders::GetMetadata(pixelShaderIndex, properties.graphicsApi).name.c_str();

	crgfx::GraphicsShaderBytecode bytecode(debugName);
	bytecode.AddBytecode(vertexShaderBytecode);
	bytecode.AddBytecode(pixelShaderBytecode);
	return bytecode;
}

crgfx::GraphicsPipelineHandle CrBuiltinPipelines::GetGraphicsPipeline
(
	const crgfx::GraphicsPipelineDescriptor& graphicsPipelineDescriptor,
	const crgfx::VertexDescriptor& vertexDescriptor,
	CrBuiltinShaders::T vertexShaderIndex,
	CrBuiltinShaders::T pixelShaderIndex
)
{
	crgfx::IDevice* device = crgfx::GetDevice().get();

	CrHash pipelineHash = graphicsPipelineDescriptor.ComputeHash();
	CrHash vertexDescriptorHash = vertexDescriptor.ComputeHash();
	CrHash builtinVertexShaderHash = CrHash(vertexShaderIndex);
	CrHash builtinPixelShaderHash = CrHash(pixelShaderIndex);

	CrHash finalHash = pipelineHash << vertexDescriptorHash << builtinVertexShaderHash << builtinPixelShaderHash;

	const auto pipelineIter = m_builtinGraphicsPipelines.find(finalHash.GetHash());
	if (pipelineIter != m_builtinGraphicsPipelines.end())
	{
		return pipelineIter->second;
	}
	else
	{

		crgfx::GraphicsShaderBytecode shader = GetGraphicsShaderBytecode(vertexShaderIndex, pixelShaderIndex);

		crgfx::GraphicsPipelineHandle graphicsPipeline = device->CreateGraphicsPipeline(graphicsPipelineDescriptor, shader, vertexDescriptor);
		graphicsPipeline->SetShaderIndices(vertexShaderIndex, pixelShaderIndex);

		m_builtinGraphicsPipelines.insert(finalHash.GetHash(), graphicsPipeline);
		return graphicsPipeline;
	}
}

crgfx::ComputePipelineHandle CrBuiltinPipelines::GetComputePipeline(CrBuiltinCompute::T computeShaderIndex)
{
	CrHash hash(computeShaderIndex);

	const auto pipelineIter = m_builtinComputePipelines.find(hash.GetHash());

	if (pipelineIter != m_builtinComputePipelines.end())
	{
		return pipelineIter->second;
	}
	else
	{
		crgfx::IDevice* renderDevice = crgfx::GetDevice().get();

		const crgfx::DeviceProperties& properties = renderDevice->GetProperties();

		crgfx::ComputeShaderBytecode shader = crgfx::ComputeShaderBytecode(CrBuiltinCompute::GetMetadata(computeShaderIndex, properties.graphicsApi).name.c_str(), crgfx::GetBuiltinComputeBytecode(computeShaderIndex));

		crgfx::ComputePipelineHandle computePipeline = renderDevice->CreateComputePipeline(shader);
		computePipeline->SetComputeShaderIndex(computeShaderIndex);

		m_builtinComputePipelines.insert(hash.GetHash(), computePipeline);

		return computePipeline;
	}
}

void CrBuiltinPipelines::RecompileBuiltinPipelines()
{
#if !defined(CR_CONFIG_FINAL)

	crgfx::IDevice* device = crgfx::GetDevice().get();
	
	const crgfx::DeviceProperties& deviceProperties = device->GetProperties();
	
	CrFixedPath outputPath = CrFixedPath(CrGlobalPaths::GetTempEngineDirectory()) / "Bultin Shaders Runtime";
	
	crstl::fixed_string2048 commandLine;
	
	commandLine += " -builtin";
	
	commandLine += " -input \"";
	commandLine += CrGlobalPaths::GetShaderSourceDirectory().c_str();
	commandLine += "\"";
	
	commandLine += " -output \"";
	commandLine += outputPath.c_str();
	commandLine += "\"";
	
	commandLine += " -platform windows";
	
	commandLine += " -graphicsapi ";
	commandLine += crgfx::GraphicsApi::ToString(deviceProperties.graphicsApi);
	
	crstl::process process
	(
		CrGlobalPaths::GetShaderCompilerPath().c_str(),
		commandLine.c_str()
	);

	if (process.is_launched())
	{
		crstl::process_exit_status exitStatus = process.wait();

		if (exitStatus.get_exit_code() == 0)
		{
			// Load the new pipelines after compilation
			// Make sure we idle the device before attempting recompilation as this will destroy pipelines that could be in flight
			device->WaitIdle();

			for (auto iter = m_builtinComputePipelines.begin(); iter != m_builtinComputePipelines.end(); ++iter)
			{
				crgfx::ComputePipelineHandle& computePipeline = iter->second;

				const CrBuiltinComputeMetadata& builtinShaderMetadata = CrBuiltinCompute::GetMetadata(computePipeline->GetComputeShaderIndex(), deviceProperties.graphicsApi);

				CrFixedPath binaryPath = outputPath;
				binaryPath /= builtinShaderMetadata.uniqueBinaryName;

				CrReadFileStream shaderBytecodeStream(binaryPath.c_str());

				if (shaderBytecodeStream.GetFile())
				{
					const crgfx::ShaderBytecodeHandle& bytecode = crgfx::ShaderBytecodeHandle(new crgfx::ShaderBytecode());
					shaderBytecodeStream << *bytecode.get();

					crgfx::ComputeShaderBytecode computeShaderBytecode = crgfx::ComputeShaderBytecode(builtinShaderMetadata.name.c_str(), bytecode);

					computePipeline->Recompile(device, computeShaderBytecode);
				}
			}

			for (auto iter = m_builtinGraphicsPipelines.begin(); iter != m_builtinGraphicsPipelines.end(); ++iter)
			{
				crgfx::GraphicsPipelineHandle& graphicsPipeline = iter->second;

				const CrBuiltinShaderMetadata& vertexShaderMetadata = CrBuiltinShaders::GetMetadata(graphicsPipeline->GetVertexShaderIndex(), deviceProperties.graphicsApi);
				const CrBuiltinShaderMetadata& pixelShaderMetadata = CrBuiltinShaders::GetMetadata(graphicsPipeline->GetPixelShaderIndex(), deviceProperties.graphicsApi);

				crgfx::ShaderDebugString debugName;
				debugName += vertexShaderMetadata.name.c_str();
				debugName += "_";
				debugName += pixelShaderMetadata.name.c_str();

				crgfx::GraphicsShaderBytecode shaderBytecode(debugName);

				CrFixedPath vertexBinaryPath = outputPath;
				vertexBinaryPath /= vertexShaderMetadata.uniqueBinaryName;

				CrReadFileStream vertexShaderBytecodeStream(vertexBinaryPath.c_str());

				if (vertexShaderBytecodeStream.GetFile())
				{
					const crgfx::ShaderBytecodeHandle& bytecode = crgfx::ShaderBytecodeHandle(new crgfx::ShaderBytecode());
					vertexShaderBytecodeStream << *bytecode.get();
					shaderBytecode.AddBytecode(bytecode);
				}

				CrFixedPath pixelBinaryPath = outputPath;
				pixelBinaryPath /= pixelShaderMetadata.uniqueBinaryName;

				CrReadFileStream pixelShaderBytecodeStream(pixelBinaryPath.c_str());

				if (pixelShaderBytecodeStream.GetFile())
				{
					const crgfx::ShaderBytecodeHandle& bytecode = crgfx::ShaderBytecodeHandle(new crgfx::ShaderBytecode());
					pixelShaderBytecodeStream << *bytecode.get();
					shaderBytecode.AddBytecode(bytecode);
				}

				graphicsPipeline->Recompile(device, shaderBytecode);
			}
		}
		else
		{
			crstl::string processOutput;
			processOutput.resize_uninitialized(2048);
			process.read_stdout(processOutput.data(), processOutput.size());
			CrLog("%s", processOutput.c_str());
		}
	}
	else
	{
		CrLog("Compilation process failed to launch");
	}

#endif
}