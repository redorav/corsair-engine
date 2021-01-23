#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Core/CrCoreForwardDeclarations.h"

#include "Core/Containers/CrVector.h"
#include "Core/String/CrFixedString.h"
#include <EASTL/fixed_function.h> // TODO Create platform-independent header CrFixedFunction

using bindpoint_t = uint8_t;
using CrShaderResourceName = CrFixedString128;

class CrShaderResource
{
public:

	const char* name = nullptr;
	bindpoint_t bindPoint = 0;

	static CrShaderResource Invalid;
};

// Make sure we don't allocate any memory on the heap
using ShaderReflectionFn = eastl::fixed_function<12, void(cr3d::ShaderStage::T stage, const CrShaderResource&)>; // TODO Create platform-independent header

// Provides shader reflection functionality. After a shader has been compiled or loaded the shader reflection structure can
// be queried for information regarding resource usage.

class ICrShaderReflection
{
public:

	virtual void AddBytecode(const CrShaderBytecodeSharedHandle& bytecode) = 0;

	virtual void ForEachConstantBuffer(ShaderReflectionFn fn) const = 0;

	virtual void ForEachTexture(ShaderReflectionFn fn) const = 0;

	virtual void ForEachSampler(ShaderReflectionFn fn) const = 0;

	const CrShaderResourceCount& GetShaderResourceCount()
	{
		return m_resourceCounts;
	}

protected:

	CrShaderResourceCount m_resourceCounts;
};
