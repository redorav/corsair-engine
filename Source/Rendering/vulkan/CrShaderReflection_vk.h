#pragma once

#include "ICrShaderReflection.h"

#pragma warning (push, 0)
#include <spirv_cross.hpp>
#pragma warning (pop)

#include "Core/SmartPointers/CrUniquePtr.h"

class CrShaderReflectionVulkan final : public ICrShaderReflection
{
private:

	virtual void AddShaderStagePS(cr3d::ShaderStage::T stage, const CrVector<unsigned char>& bytecode) override;

	virtual CrShaderResource GetResourcePS(cr3d::ShaderStage::T stage, cr3d::ShaderResourceType::T resourceType, uint32_t index) const override;

	virtual uint32_t GetResourceCountPS(cr3d::ShaderStage::T stage, cr3d::ShaderResourceType::T resourceType) const override;

private:

	const spirv_cross::Resource& GetSpvResource(cr3d::ShaderStage::T stage, cr3d::ShaderResourceType::T resourceType, uint32_t index) const;
	
	static const spirv_cross::Resource defaultResource;

	CrUniquePtr<spirv_cross::Compiler> reflection[cr3d::ShaderStage::Count];

	spirv_cross::ShaderResources resources[cr3d::ShaderStage::Count];
};
