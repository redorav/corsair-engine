#include "CrRendering_pch.h"

#include "Rendering/CrBuiltinPipeline.h"
#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrShader.h"
#include "Rendering/CrPipelineStateManager.h"

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

	CrGraphicsShaderDescriptor graphicsShaderDescriptor;
	graphicsShaderDescriptor.m_bytecodes.push_back(vertexShaderBytecode);
	graphicsShaderDescriptor.m_bytecodes.push_back(pixelShaderBytecode);

	CrGraphicsShaderHandle shader = renderDevice->CreateGraphicsShader(graphicsShaderDescriptor);

	m_graphicsPipeline = CrPipelineStateManager::Get().GetGraphicsPipeline(graphicsPipelineDescriptor, shader, vertexDescriptor);
}

CrBuiltinComputePipeline::CrBuiltinComputePipeline(ICrRenderDevice* renderDevice, const CrComputePipelineDescriptor& computePipelineDescriptor, CrBuiltinShaders::T computeShader)
	: m_computePipelineDescriptor(computePipelineDescriptor)
{
	CrComputeShaderDescriptor computeShaderDescriptor;
	computeShaderDescriptor.m_bytecode = ICrRenderSystem::GetBuiltinShaderBytecode(computeShader);

	CrComputeShaderHandle shader = renderDevice->CreateComputeShader(computeShaderDescriptor);

	m_computePipeline = CrPipelineStateManager::Get().GetComputePipeline(computePipelineDescriptor, shader);
}
