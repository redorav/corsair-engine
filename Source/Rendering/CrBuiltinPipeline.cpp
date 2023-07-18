#include "Rendering/CrRendering_pch.h"

#include "Rendering/CrBuiltinPipeline.h"
#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/ICrShader.h"
#include "Rendering/CrPipelineStateManager.h"
#include "Rendering/CrRendererConfig.h"
#include "Rendering/CrCommonVertexLayouts.h"

#include "GeneratedShaders/BuiltinShaders.h"

CrBuiltinGraphicsPipeline CrBuiltinPipelines::BasicUbershaderForward;

CrBuiltinGraphicsPipeline CrBuiltinPipelines::BasicUbershaderGBuffer;

CrBuiltinGraphicsPipeline CrBuiltinPipelines::BasicUbershaderDebug;

CrArray<CrBuiltinComputePipelineHandle, 256> CrBuiltinPipelines::m_builtinComputePipelines;

CrBuiltinGraphicsPipeline::CrBuiltinGraphicsPipeline
(
	ICrRenderDevice* renderDevice,
	const CrGraphicsPipelineDescriptor& graphicsPipelineDescriptor,
	const CrVertexDescriptor& vertexDescriptor,
	CrBuiltinShaders::T vertexShader, 
	CrBuiltinShaders::T pixelShader
)
	: m_graphicsPipelineDescriptor(graphicsPipelineDescriptor)
	, m_vertexDescriptor(vertexDescriptor)
{
	CrShaderBytecodeHandle vertexShaderBytecode = ICrRenderSystem::GetBuiltinShaderBytecode(vertexShader);
	CrShaderBytecodeHandle pixelShaderBytecode = ICrRenderSystem::GetBuiltinShaderBytecode(pixelShader);

	const CrRenderDeviceProperties& properties = renderDevice->GetProperties();

	CrGraphicsShaderDescriptor graphicsShaderDescriptor;
	graphicsShaderDescriptor.m_debugName += CrBuiltinShaders::GetBuiltinShaderMetadata(vertexShader, properties.graphicsApi).name.c_str();
	graphicsShaderDescriptor.m_debugName += "_";
	graphicsShaderDescriptor.m_debugName += CrBuiltinShaders::GetBuiltinShaderMetadata(pixelShader, properties.graphicsApi).name.c_str();
	graphicsShaderDescriptor.m_bytecodes.push_back(vertexShaderBytecode);
	graphicsShaderDescriptor.m_bytecodes.push_back(pixelShaderBytecode);

	CrGraphicsShaderHandle shader = renderDevice->CreateGraphicsShader(graphicsShaderDescriptor);

	m_graphicsPipeline = CrPipelineStateManager::Get().GetGraphicsPipeline(graphicsPipelineDescriptor, shader, vertexDescriptor);
}

CrBuiltinComputePipeline::CrBuiltinComputePipeline(ICrRenderDevice* renderDevice, CrBuiltinShaders::T computeShader)
{
	const CrRenderDeviceProperties& properties = renderDevice->GetProperties();

	CrComputeShaderDescriptor computeShaderDescriptor;
	computeShaderDescriptor.m_debugName = CrBuiltinShaders::GetBuiltinShaderMetadata(computeShader, properties.graphicsApi).name.c_str();
	computeShaderDescriptor.m_bytecode = ICrRenderSystem::GetBuiltinShaderBytecode(computeShader);

	CrComputeShaderHandle shader = renderDevice->CreateComputeShader(computeShaderDescriptor);

	m_computePipeline = CrPipelineStateManager::Get().GetComputePipeline(shader);
}

void CrBuiltinPipelines::Initialize(ICrRenderDevice* renderDevice)
{
	// Builtin ubershaders
	{
		CrGraphicsPipelineDescriptor forwardUbershaderDescriptor;
		forwardUbershaderDescriptor.renderTargets.colorFormats[0] = CrRendererConfig::SwapchainFormat;
		forwardUbershaderDescriptor.renderTargets.depthFormat = CrRendererConfig::DepthBufferFormat;
		BasicUbershaderForward = CrBuiltinGraphicsPipeline(renderDevice, forwardUbershaderDescriptor, ComplexVertexDescriptor, CrBuiltinShaders::BasicUbershaderVS, CrBuiltinShaders::BasicForwardUbershaderPS);

		CrGraphicsPipelineDescriptor gbufferUbershaderDescriptor;
		gbufferUbershaderDescriptor.renderTargets.colorFormats[0] = CrRendererConfig::GBufferAlbedoAOFormat;
		gbufferUbershaderDescriptor.renderTargets.colorFormats[1] = CrRendererConfig::GBufferNormalsFormat;
		gbufferUbershaderDescriptor.renderTargets.colorFormats[2] = CrRendererConfig::GBufferMaterialFormat;
		gbufferUbershaderDescriptor.renderTargets.depthFormat = CrRendererConfig::DepthBufferFormat;
		BasicUbershaderGBuffer = CrBuiltinGraphicsPipeline(renderDevice, gbufferUbershaderDescriptor, ComplexVertexDescriptor, CrBuiltinShaders::BasicUbershaderVS, CrBuiltinShaders::BasicGBufferUbershaderPS);

		CrGraphicsPipelineDescriptor debugUbershaderDescriptor;
		debugUbershaderDescriptor.renderTargets.colorFormats[0] = CrRendererConfig::DebugShaderFormat;
		debugUbershaderDescriptor.renderTargets.depthFormat = CrRendererConfig::DepthBufferFormat;
		BasicUbershaderDebug = CrBuiltinGraphicsPipeline(renderDevice, debugUbershaderDescriptor, ComplexVertexDescriptor, CrBuiltinShaders::BasicUbershaderVS, CrBuiltinShaders::BasicDebugUbershaderPS);
	}
}

CrBuiltinComputePipelineHandle CrBuiltinPipelines::GetComputePipeline(CrBuiltinShaders::T computeShader)
{
	const CrBuiltinComputePipelineHandle& builtinPipeline = m_builtinComputePipelines[computeShader];
	
	if (builtinPipeline)
	{
		return builtinPipeline;
		
	}
	else
	{
		m_builtinComputePipelines[computeShader] = CrIntrusivePtr<CrBuiltinComputePipeline>(new CrBuiltinComputePipeline(ICrRenderSystem::GetRenderDevice().get(), computeShader));
		return m_builtinComputePipelines[computeShader];
	}
}
