#pragma once

#include "Rendering/ICrShaderReflection.h"

#include <spirv_reflect.h>

#include "Core/SmartPointers/CrUniquePtr.h"

class CrShaderReflectionVulkan final : public ICrShaderReflection
{
public:

	~CrShaderReflectionVulkan();

	virtual void AddBytecode(const CrShaderBytecodeSharedHandle& bytecode) override;

	virtual void ForEachResource(const ShaderReflectionFn& fn) const override;

	const SpvReflectShaderModule& GetReflection(cr3d::ShaderStage::T shaderStage) const
	{
		return m_reflection[shaderStage];
	}

private:

	SpvReflectShaderModule m_reflection[cr3d::ShaderStage::Count] = {};

	uint32_t m_currentBindSlot = 0;
};
