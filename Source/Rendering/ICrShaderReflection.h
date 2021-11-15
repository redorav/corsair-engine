#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Core/CrCoreForwardDeclarations.h"

using bindpoint_t = uint8_t;

class CrShaderResource
{
public:

	const char* name = nullptr;
	bindpoint_t bindPoint = 0;
	cr3d::ShaderResourceType::T type = cr3d::ShaderResourceType::Count;

	static CrShaderResource Invalid;
};

// Make sure we don't allocate any memory on the heap
using ShaderReflectionFn = CrFixedFunction<12, void(cr3d::ShaderStage::T stage, const CrShaderResource&)>;

// Provides shader reflection functionality. After a shader has been compiled or loaded the shader reflection structure can
// be queried for information regarding resource usage.

class ICrShaderReflection
{
public:

	virtual ~ICrShaderReflection() {}

	virtual void AddBytecode(const CrShaderBytecodeSharedHandle& bytecode) = 0;

	virtual void ForEachResource(const ShaderReflectionFn& fn) const = 0;
};
