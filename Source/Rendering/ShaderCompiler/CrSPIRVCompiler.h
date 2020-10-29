#pragma once

#include <string>
#include <vector>

#include "Rendering/CrRenderingForwardDeclarations.h"

struct CompilationDescriptor;

class CrSPIRVCompiler
{
public:

	static bool HLSLtoSPIRV(const CompilationDescriptor& compilationDescriptor, std::vector<uint32_t>& spirvBytecode);
};