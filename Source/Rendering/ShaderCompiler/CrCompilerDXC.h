#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"

struct CompilationDescriptor;

class CrCompilerDXC
{
public:

	static bool HLSLtoSPIRV(const CompilationDescriptor& compilationDescriptor, CrString& compilationStatus);

	static bool HLSLtoDXIL(const CompilationDescriptor& compilationDescriptor, CrString& compilationStatus);

private:
	
	static void CreateCommonCommandLine(const CompilationDescriptor& compilationDescriptor, CrFixedString512& commandLine);
};