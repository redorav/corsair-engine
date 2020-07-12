#include "CrRendering_pch.h"
#include "CrShaderReflection_d3d12.h"

void CrShaderReflectionD3D12::AddShaderStagePS(cr3d::ShaderStage::T stage, const CrVector<unsigned char>& bytecode)
{
	
}

CrShaderResource CrShaderReflectionD3D12::GetResourcePS(cr3d::ShaderStage::T stage, cr3d::ShaderResourceType::T resourceType, uint32_t index) const
{
	CrShaderResource resource = CrShaderResource::Invalid;
	return resource;
}

uint32_t CrShaderReflectionD3D12::GetResourceCountPS(cr3d::ShaderStage::T stage, cr3d::ShaderResourceType::T resourceType) const
{
	return 0;
}
