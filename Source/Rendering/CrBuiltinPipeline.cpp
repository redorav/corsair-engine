#include "CrRendering_pch.h"

#include "Rendering/CrBuiltinPipeline.h"
#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrShader.h"
#include "Rendering/CrPipelineStateManager.h"
#include "GeneratedShaders/BuiltinShaders.h"

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
	CrShaderBytecodeSharedHandle vertexShaderBytecode = ICrRenderSystem::GetBuiltinShaderBytecode(vertexShader);
	CrShaderBytecodeSharedHandle pixelShaderBytecode = ICrRenderSystem::GetBuiltinShaderBytecode(pixelShader);

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

CrBuiltinComputePipeline::CrBuiltinComputePipeline(ICrRenderDevice* renderDevice, const CrComputePipelineDescriptor& computePipelineDescriptor, CrBuiltinShaders::T computeShader)
	: m_computePipelineDescriptor(computePipelineDescriptor)
{
	const CrRenderDeviceProperties& properties = renderDevice->GetProperties();

	CrComputeShaderDescriptor computeShaderDescriptor;
	computeShaderDescriptor.m_debugName = CrBuiltinShaders::GetBuiltinShaderMetadata(computeShader, properties.graphicsApi).name.c_str();
	computeShaderDescriptor.m_bytecode = ICrRenderSystem::GetBuiltinShaderBytecode(computeShader);

	CrComputeShaderHandle shader = renderDevice->CreateComputeShader(computeShaderDescriptor);

	m_computePipeline = CrPipelineStateManager::Get().GetComputePipeline(computePipelineDescriptor, shader);
}
