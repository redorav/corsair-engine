#pragma once

#include "Rendering/ICrShaderReflection.h"

#pragma warning (push, 0)
#include <spirv_cross.hpp>
#pragma warning (pop)

#include "Core/SmartPointers/CrUniquePtr.h"

class CrShaderReflectionVulkan final : public ICrShaderReflection
{
public:

	virtual void AddBytecode(const CrShaderBytecodeSharedHandle& bytecode) override;

	virtual void ForEachConstantBuffer(ShaderReflectionFn fn) const override;

	virtual void ForEachTexture(ShaderReflectionFn fn) const override;

	virtual void ForEachSampler(ShaderReflectionFn fn) const override;

private:

	CrUniquePtr<spirv_cross::Compiler> m_reflection[cr3d::ShaderStage::Count];

	spirv_cross::ShaderResources m_resources[cr3d::ShaderStage::Count];
};
