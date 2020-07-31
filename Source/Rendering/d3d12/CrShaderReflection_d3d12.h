#pragma once

#include "ICrShaderReflection.h"

#pragma warning (push, 0)
#include <spirv_cross.hpp>
#pragma warning (pop)

#include "Core/SmartPointers/CrUniquePtr.h"

class CrShaderReflectionD3D12 final : public ICrShaderReflection
{
private:

	virtual void AddShaderStagePS(cr3d::ShaderStage::T stage, const CrVector<unsigned char>& bytecode) override;

	virtual CrShaderResource GetResourcePS(cr3d::ShaderStage::T stage, cr3d::ShaderResourceType::T resourceType, uint32_t index) const override;

	virtual uint32_t GetResourceCountPS(cr3d::ShaderStage::T stage, cr3d::ShaderResourceType::T resourceType) const override;

private:

	
};
