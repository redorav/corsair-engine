#include "Rendering/CrRendering_pch.h"

#include "Rendering/CrBuiltinPipeline.h"
#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/ICrShader.h"
#include "Rendering/CrPipelineStateManager.h"
#include "Rendering/CrRendererConfig.h"
#include "Rendering/CrCommonVertexLayouts.h"

#include "Core/Process/CrProcess.h"
#include "Core/CrGlobalPaths.h"

#include "GeneratedShaders/BuiltinShaders.h"

CrBuiltinGraphicsPipelineHandle CrBuiltinPipelines::BasicUbershaderForward;

CrBuiltinGraphicsPipelineHandle CrBuiltinPipelines::BasicUbershaderGBuffer;

CrBuiltinGraphicsPipelineHandle CrBuiltinPipelines::BasicUbershaderDebug;

CrHashMap<uint64_t, CrBuiltinGraphicsPipelineHandle> CrBuiltinPipelines::m_builtinGraphicsPipelines;

CrHashMap<uint64_t, CrBuiltinComputePipelineHandle> CrBuiltinPipelines::m_builtinComputePipelines;

CrBuiltinGraphicsPipeline::CrBuiltinGraphicsPipeline() : m_vertexShaderIndex(CrBuiltinShaders::Count), m_pixelShaderIndex(CrBuiltinShaders::Count) {}

CrBuiltinGraphicsPipeline::CrBuiltinGraphicsPipeline
(
	ICrRenderDevice* renderDevice,
	const CrGraphicsPipelineDescriptor& graphicsPipelineDescriptor,
	const CrVertexDescriptor& vertexDescriptor,
	CrBuiltinShaders::T vertexShaderIndex, 
	CrBuiltinShaders::T pixelShaderIndex
)
	: m_graphicsPipelineDescriptor(graphicsPipelineDescriptor)
	, m_vertexDescriptor(vertexDescriptor)
	, m_vertexShaderIndex(vertexShaderIndex)
	, m_pixelShaderIndex(pixelShaderIndex)
{
	CrShaderBytecodeHandle vertexShaderBytecode = ICrRenderSystem::GetBuiltinShaderBytecode(vertexShaderIndex);
	CrShaderBytecodeHandle pixelShaderBytecode = ICrRenderSystem::GetBuiltinShaderBytecode(pixelShaderIndex);

	const CrRenderDeviceProperties& properties = renderDevice->GetProperties();

	CrGraphicsShaderDescriptor graphicsShaderDescriptor;
	graphicsShaderDescriptor.m_debugName += CrBuiltinShaders::GetBuiltinShaderMetadata(vertexShaderIndex, properties.graphicsApi).name.c_str();
	graphicsShaderDescriptor.m_debugName += "_";
	graphicsShaderDescriptor.m_debugName += CrBuiltinShaders::GetBuiltinShaderMetadata(pixelShaderIndex, properties.graphicsApi).name.c_str();
	graphicsShaderDescriptor.m_bytecodes.push_back(vertexShaderBytecode);
	graphicsShaderDescriptor.m_bytecodes.push_back(pixelShaderBytecode);

	CrGraphicsShaderHandle shader = renderDevice->CreateGraphicsShader(graphicsShaderDescriptor);

	m_graphicsPipeline = CrPipelineStateManager::Get().GetGraphicsPipeline(graphicsPipelineDescriptor, shader, vertexDescriptor);
}

CrBuiltinComputePipeline::CrBuiltinComputePipeline(ICrRenderDevice* renderDevice, CrBuiltinShaders::T computeShaderIndex)
	: m_computeShaderIndex(computeShaderIndex)
{
	const CrRenderDeviceProperties& properties = renderDevice->GetProperties();

	CrComputeShaderDescriptor computeShaderDescriptor;
	computeShaderDescriptor.m_debugName = CrBuiltinShaders::GetBuiltinShaderMetadata(computeShaderIndex, properties.graphicsApi).name.c_str();
	computeShaderDescriptor.m_bytecode = ICrRenderSystem::GetBuiltinShaderBytecode(computeShaderIndex);

	CrComputeShaderHandle shader = renderDevice->CreateComputeShader(computeShaderDescriptor);

	m_computePipeline = CrPipelineStateManager::Get().GetComputePipeline(shader);
}

void CrBuiltinPipelines::Initialize()
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

CrBuiltinGraphicsPipelineHandle CrBuiltinPipelines::GetGraphicsPipeline
(
	const CrGraphicsPipelineDescriptor& graphicsPipelineDescriptor,
	const CrVertexDescriptor& vertexDescriptor,
	CrBuiltinShaders::T vertexShaderIndex,
	CrBuiltinShaders::T pixelShaderIndex
)
{
	ICrRenderDevice* renderDevice = ICrRenderSystem::GetRenderDevice().get();

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
		CrBuiltinGraphicsPipelineHandle pipeline = CrBuiltinGraphicsPipelineHandle(new CrBuiltinGraphicsPipeline(renderDevice, graphicsPipelineDescriptor, vertexDescriptor, vertexShaderIndex, pixelShaderIndex));
		m_builtinGraphicsPipelines.insert({ finalHash.GetHash(), pipeline });
		return pipeline;
	}
}

CrBuiltinComputePipelineHandle CrBuiltinPipelines::GetComputePipeline(CrBuiltinShaders::T computeShaderIndex)
{
	CrHash hash(computeShaderIndex);

	const auto pipelineIter = m_builtinComputePipelines.find(hash.GetHash());

	if (pipelineIter != m_builtinComputePipelines.end())
	{
		return pipelineIter->second;
		
	}
	else
	{
		CrBuiltinComputePipelineHandle pipeline(new CrBuiltinComputePipeline(ICrRenderSystem::GetRenderDevice().get(), computeShaderIndex));
		m_builtinComputePipelines.insert({ hash.GetHash(), pipeline });
		return pipeline;
	}
}

void CrBuiltinPipelines::RecompileComputePipelines()
{
	ICrRenderDevice* renderDevice = ICrRenderSystem::GetRenderDevice().get();

	const CrRenderDeviceProperties& deviceProperties = renderDevice->GetProperties();

	CrFixedPath outputPath = CrFixedPath(CrGlobalPaths::GetTempEngineDirectory()) / "Bultin Shaders Runtime";

	CrProcessDescriptor processDescriptor;

	processDescriptor.commandLine += CrGlobalPaths::GetShaderCompilerPath().c_str();

	processDescriptor.commandLine += " -builtin";

	processDescriptor.commandLine += " -input \"";
	processDescriptor.commandLine += CrGlobalPaths::GetShaderSourceDirectory().c_str();
	processDescriptor.commandLine += "\"";

	processDescriptor.commandLine += " -output \"";
	processDescriptor.commandLine += outputPath.c_str();
	processDescriptor.commandLine += "\"";

	processDescriptor.commandLine += " -platform windows";

	processDescriptor.commandLine += " -graphicsapi ";
	processDescriptor.commandLine += cr3d::GraphicsApi::ToString(deviceProperties.graphicsApi);

	CrProcess process(processDescriptor);

	process.Wait();

	if (process.GetReturnValue() >= 0)
	{
		// Load the new pipelines after compilation

		for (auto iter = m_builtinComputePipelines.begin(); iter != m_builtinComputePipelines.end(); ++iter)
		{
			CrBuiltinComputePipelineHandle& builtinPipeline = iter->second;

			const CrBuiltinShaderMetadata& builtinShaderMetadata = CrBuiltinShaders::GetBuiltinShaderMetadata(builtinPipeline->GetComputeShaderIndex(), deviceProperties.graphicsApi);

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

				CrComputePipelineHandle pipeline = CrPipelineStateManager::Get().GetComputePipeline(shader);

				builtinPipeline->SetPipeline(pipeline);
			}
		}

		for (auto iter = m_builtinGraphicsPipelines.begin(); iter != m_builtinGraphicsPipelines.end(); ++iter)
		{
			CrBuiltinGraphicsPipelineHandle& builtinPipeline = iter->second;

			const CrBuiltinShaderMetadata& vertexShaderMetadata = CrBuiltinShaders::GetBuiltinShaderMetadata(builtinPipeline->GetVertexShaderIndex(), deviceProperties.graphicsApi);
			const CrBuiltinShaderMetadata& pixelShaderMetadata = CrBuiltinShaders::GetBuiltinShaderMetadata(builtinPipeline->GetPixelShaderIndex(), deviceProperties.graphicsApi);

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

			CrGraphicsPipelineHandle pipeline = CrPipelineStateManager::Get().GetGraphicsPipeline(builtinPipeline->GetPipelineDescriptor(), shader, builtinPipeline->GetVertexDescriptor());

			builtinPipeline->SetPipeline(pipeline);
		}
	}
	else
	{
		CrString processOutput;
		processOutput.resize_uninitialized(2048);
		process.ReadStdOut(processOutput.data(), processOutput.size());
		CrLog("%s", processOutput.c_str());
	}
}
