#pragma once

#include <vector>

#include "Rendering/CrRenderingForwardDeclarations.h"

struct CompilationDescriptor;

class CrCompilerGLSLANG
{
public:

	static bool HLSLtoSPIRV(const CompilationDescriptor& compilationDescriptor, std::vector<uint32_t>& spirvBytecode);
};