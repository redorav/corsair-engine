#pragma once

#include "Rendering/CrRendering.h"
#include "Core/CrPlatform.h"

#include <string>

struct CompilationDescriptor
{
	std::string inputPath;
	std::string outputPath;
	std::string entryPoint;
	cr3d::ShaderStage::T shaderStage;
	cr::Platform::T platform;
};

class CrShaderCompiler
{
public:

	void Initialize();

	void Finalize();

	void Compile(const CompilationDescriptor& compilationDescriptor);
};