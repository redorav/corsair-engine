#pragma once

#include "Rendering/ICrShaderReflection.h"

#include "Core/SmartPointers/CrUniquePtr.h"

class CrShaderReflectionD3D12 final : public ICrShaderReflection
{
public:

	virtual void AddBytecode(const CrShaderBytecodeSharedHandle& bytecode) override;

	virtual void ForEachResource(const ShaderReflectionFn& fn) const override;

private:

	
};
