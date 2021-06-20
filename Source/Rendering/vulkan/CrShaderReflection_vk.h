#pragma once

#include "Rendering/ICrShaderReflection.h"

#include <spirv_reflect.h>

#include "Core/SmartPointers/CrUniquePtr.h"

class CrShaderReflectionVulkan final : public ICrShaderReflection
{
public:

	~CrShaderReflectionVulkan();

	virtual void AddBytecode(const CrShaderBytecodeSharedHandle& bytecode) override;

	virtual void ForEachResource(ShaderReflectionFn fn) const override;

// TODO make private

	SpvReflectShaderModule m_reflection[cr3d::ShaderStage::Count] = {};

private:

	uint32_t m_currentBindSlot = 0;
};
