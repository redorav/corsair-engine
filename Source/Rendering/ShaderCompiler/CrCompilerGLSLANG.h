#pragma once

#include <vector>

#include "Rendering/CrRenderingForwardDeclarations.h"

struct CompilationDescriptor;

namespace glslang { class TIntermediate; }

class CrCompilerGLSLANG
{
public:

	static bool HLSLtoAST(const CompilationDescriptor& compilationDescriptor, const glslang::TIntermediate*& intermediate, CrString& compilationStatus);

	static bool HLSLtoSPIRV(const CompilationDescriptor& compilationDescriptor, std::vector<uint32_t>& spirvBytecode, CrString& compilationStatus);
};