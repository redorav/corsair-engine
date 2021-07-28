#include "CrRendering_pch.h"
#include "CrShaderReflection_d3d12.h"
#include "Core/CrMacros.h"

void CrShaderReflectionD3D12::AddBytecode(const CrShaderBytecodeSharedHandle& /*bytecode*/)
{
	
}

void CrShaderReflectionD3D12::ForEachResource(const ShaderReflectionFn& fn) const
{
	unused_parameter(fn);
}