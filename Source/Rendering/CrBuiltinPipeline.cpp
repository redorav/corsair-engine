#include "Rendering/CrRendering_pch.h"

#include "Rendering/CrBuiltinPipeline.h"
#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/ICrShader.h"
#include "Rendering/CrRendererConfig.h"
#include "Rendering/CrCommonVertexLayouts.h"

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
		CrGraphicsPipelineDescriptor forwardUbershaderDescriptor;
		forwardUbershaderDescriptor.renderTargets.colorFormats[0] = CrRendererConfig::SwapchainFormat;
		forwardUbershaderDescriptor.renderTargets.depthFormat = CrRendererConfig::DepthBufferFormat;
		BasicUbershaderForward = CrBuiltinPipelines::GetGraphicsPipeline(forwardUbershaderDescriptor, ComplexVertexDescriptor, CrBuiltinShaders::BasicUbershaderVS, CrBuiltinShaders::BasicForwardUbershaderPS);

		CrGraphicsPipelineDescriptor gbufferUbershaderDescriptor;
		gbufferUbershaderDescriptor.renderTargets.colorFormats[0] = CrRendererConfig::GBufferAlbedoAOFormat;
		gbufferUbershaderDescriptor.renderTargets.colorFormats[1] = CrRendererConfig::GBufferNormalsFormat;
		gbufferUbershaderDescriptor.renderTargets.colorFormats[2] = CrRendererConfig::GBufferMaterialFormat;
		gbufferUbershaderDescriptor.renderTargets.depthFormat = CrRendererConfig::DepthBufferFormat;
		BasicUbershaderGBuffer = CrBuiltinPipelines::GetGraphicsPipeline(gbufferUbershaderDescriptor, ComplexVertexDescriptor, CrBuiltinShaders::BasicUbershaderVS, CrBuiltinShaders::BasicGBufferUbershaderPS);

		CrGraphicsPipelineDescriptor debugUbershaderDescriptor;
		debugUbershaderDescriptor.renderTargets.colorFormats[0] = CrRendererConfig::DebugShaderFormat;
		debugUbershaderDescriptor.renderTargets.depthFormat = CrRendererConfig::DepthBufferFormat;
		BasicUbershaderDebug = CrBuiltinPipelines::GetGraphicsPipeline(debugUbershaderDescriptor, ComplexVertexDescriptor, CrBuiltinShaders::BasicUbershaderVS, CrBuiltinShaders::BasicDebugUbershaderPS);
	}
}

CrGraphicsPipelineHandle CrBuiltinPipelines::GetGraphicsPipeline
(
	const CrGraphicsPipelineDescriptor& graphicsPipelineDescriptor,
	const CrVertexDescriptor& vertexDescriptor,
	CrBuiltinShaders::T vertexShaderIndex,
	CrBuiltinShaders::T pixelShaderIndex
)
{
	ICrRenderDevice* renderDevice = RenderSystem->GetRenderDevice().get();

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
		CrShaderBytecodeHandle vertexShaderBytecode = RenderSystem->GetBuiltinShaderBytecode(vertexShaderIndex);
		CrShaderBytecodeHandle pixelShaderBytecode = RenderSystem->GetBuiltinShaderBytecode(pixelShaderIndex);

		const CrRenderDeviceProperties& properties = renderDevice->GetProperties();

		CrGraphicsShaderDescriptor graphicsShaderDescriptor;
		graphicsShaderDescriptor.m_debugName += CrBuiltinShaders::GetBuiltinShaderMetadata(vertexShaderIndex, properties.graphicsApi).name.c_str();
		graphicsShaderDescriptor.m_debugName += "_";
		graphicsShaderDescriptor.m_debugName += CrBuiltinShaders::GetBuiltinShaderMetadata(pixelShaderIndex, properties.graphicsApi).name.c_str();
		graphicsShaderDescriptor.m_bytecodes.push_back(vertexShaderBytecode);
		graphicsShaderDescriptor.m_bytecodes.push_back(pixelShaderBytecode);

		CrGraphicsShaderHandle shader = renderDevice->CreateGraphicsShader(graphicsShaderDescriptor);

		CrGraphicsPipelineHandle graphicsPipeline = renderDevice->CreateGraphicsPipeline(graphicsPipelineDescriptor, shader, vertexDescriptor);
		graphicsPipeline->SetShaderIndices(vertexShaderIndex, pixelShaderIndex);

		m_builtinGraphicsPipelines.insert(finalHash.GetHash(), graphicsPipeline);
		return graphicsPipeline;
	}
}

CrComputePipelineHandle CrBuiltinPipelines::GetComputePipeline(CrBuiltinShaders::T computeShaderIndex)
{
	CrHash hash(computeShaderIndex);

	const auto pipelineIter = m_builtinComputePipelines.find(hash.GetHash());

	if (pipelineIter != m_builtinComputePipelines.end())
	{
		return pipelineIter->second;
	}
	else
	{
		ICrRenderDevice* renderDevice = RenderSystem->GetRenderDevice().get();

		const CrRenderDeviceProperties& properties = renderDevice->GetProperties();

		CrComputeShaderDescriptor computeShaderDescriptor;
		computeShaderDescriptor.m_debugName = CrBuiltinShaders::GetBuiltinShaderMetadata(computeShaderIndex, properties.graphicsApi).name.c_str();
		computeShaderDescriptor.m_bytecode = RenderSystem->GetBuiltinShaderBytecode(computeShaderIndex);

		CrComputeShaderHandle shader = renderDevice->CreateComputeShader(computeShaderDescriptor);

		CrComputePipelineHandle computePipeline = renderDevice->CreateComputePipeline(shader);
		computePipeline->SetComputeShaderIndex(computeShaderIndex);

		m_builtinComputePipelines.insert(hash.GetHash(), computePipeline);

		return computePipeline;
	}
}

void CrBuiltinPipelines::RecompileComputePipelines()
{
#if !defined(CR_CONFIG_FINAL)

	ICrRenderDevice* renderDevice = RenderSystem->GetRenderDevice().get();
	
	const CrRenderDeviceProperties& deviceProperties = renderDevice->GetProperties();
	
	CrFixedPath outputPath = CrFixedPath(CrGlobalPaths::GetTempEngineDirectory()) / "Bultin Shaders Runtime";
	
	CrFixedString2048 commandLine;
	
	commandLine += " -builtin";
	
	commandLine += " -input \"";
	commandLine += CrGlobalPaths::GetShaderSourceDirectory().c_str();
	commandLine += "\"";
	
	commandLine += " -output \"";
	commandLine += outputPath.c_str();
	commandLine += "\"";
	
	commandLine += " -platform windows";
	
	commandLine += " -graphicsapi ";
	commandLine += cr3d::GraphicsApi::ToString(deviceProperties.graphicsApi);
	
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
			renderDevice->WaitIdle();

			for (auto iter = m_builtinComputePipelines.begin(); iter != m_builtinComputePipelines.end(); ++iter)
			{
				CrComputePipelineHandle& computePipeline = iter->second;

				const CrBuiltinShaderMetadata& builtinShaderMetadata = CrBuiltinShaders::GetBuiltinShaderMetadata(computePipeline->GetComputeShaderIndex(), deviceProperties.graphicsApi);

				CrFixedPath binaryPath = outputPath;
				binaryPath /= builtinShaderMetadata.uniqueBinaryName;

				CrReadFileStream shaderBytecodeStream(binaryPath.c_str());

				if (shaderBytecodeStream.GetFile())
				{
					const CrShaderBytecodeHandle& bytecode = CrShaderBytecodeHandle(new CrShaderBytecode());
					shaderBytecodeStream << *bytecode.get();

					CrComputeShaderDescriptor computeShaderDescriptor;
					computeShaderDescriptor.m_debugName = builtinShaderMetadata.name.c_str();
					computeShaderDescriptor.m_bytecode = bytecode;

					CrComputeShaderHandle shader = renderDevice->CreateComputeShader(computeShaderDescriptor);

					computePipeline->Recompile(renderDevice, shader);
				}
			}

			for (auto iter = m_builtinGraphicsPipelines.begin(); iter != m_builtinGraphicsPipelines.end(); ++iter)
			{
				CrGraphicsPipelineHandle& graphicsPipeline = iter->second;

				const CrBuiltinShaderMetadata& vertexShaderMetadata = CrBuiltinShaders::GetBuiltinShaderMetadata(graphicsPipeline->GetVertexShaderIndex(), deviceProperties.graphicsApi);
				const CrBuiltinShaderMetadata& pixelShaderMetadata = CrBuiltinShaders::GetBuiltinShaderMetadata(graphicsPipeline->GetPixelShaderIndex(), deviceProperties.graphicsApi);

				CrGraphicsShaderDescriptor graphicsShaderDescriptor;
				graphicsShaderDescriptor.m_debugName += vertexShaderMetadata.name.c_str();
				graphicsShaderDescriptor.m_debugName += "_";
				graphicsShaderDescriptor.m_debugName += pixelShaderMetadata.name.c_str();

				CrFixedPath vertexBinaryPath = outputPath;
				vertexBinaryPath /= vertexShaderMetadata.uniqueBinaryName;

				CrReadFileStream vertexShaderBytecodeStream(vertexBinaryPath.c_str());

				if (vertexShaderBytecodeStream.GetFile())
				{
					const CrShaderBytecodeHandle& bytecode = CrShaderBytecodeHandle(new CrShaderBytecode());
					vertexShaderBytecodeStream << *bytecode.get();
					graphicsShaderDescriptor.m_bytecodes.push_back(bytecode);
				}

				CrFixedPath pixelBinaryPath = outputPath;
				pixelBinaryPath /= pixelShaderMetadata.uniqueBinaryName;

				CrReadFileStream pixelShaderBytecodeStream(pixelBinaryPath.c_str());

				if (pixelShaderBytecodeStream.GetFile())
				{
					const CrShaderBytecodeHandle& bytecode = CrShaderBytecodeHandle(new CrShaderBytecode());
					pixelShaderBytecodeStream << *bytecode.get();
					graphicsShaderDescriptor.m_bytecodes.push_back(bytecode);
				}

				CrGraphicsShaderHandle shader = renderDevice->CreateGraphicsShader(graphicsShaderDescriptor);

				graphicsPipeline->Recompile(renderDevice, shader);
			}
		}
		else
		{
			CrString processOutput;
			processOutput.resize_uninitialized(2048);
			//process.ReadStdOut(processOutput.data(), processOutput.size());
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