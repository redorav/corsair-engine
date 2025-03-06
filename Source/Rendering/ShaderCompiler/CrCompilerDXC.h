#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"

struct CompilationDescriptor;

class CrCompilerDXC
{
public:

	static bool HLSLtoSPIRV(const CompilationDescriptor& compilationDescriptor, CrVector<uint32_t>& bytecode, crstl::string& compilationStatus);

	static bool HLSLtoSPIRV(const CompilationDescriptor& compilationDescriptor, crstl::string& compilationStatus);

	static bool HLSLtoDXIL(const CompilationDescriptor& compilationDescriptor, crstl::string& compilationStatus);
};