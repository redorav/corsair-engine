#pragma once

#include "Rendering/ICrShaderReflection.h"

#include "Core/SmartPointers/CrUniquePtr.h"

class CrShaderReflectionD3D12 final : public ICrShaderReflection
{
private:

	virtual void AddBytecode(const CrShaderBytecodeSharedHandle& bytecode) override;

	virtual void ForEachResource(ShaderReflectionFn fn) const override;

private:

	
};
