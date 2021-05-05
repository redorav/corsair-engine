#pragma once

#include "Rendering/CrRendering.h"
#include "Core/CrPlatform.h"

#include <string>

struct CompilationDescriptor
{
	std::string inputPath;
	std::string outputPath;
	std::string entryPoint;
	cr::Platform::T platform;
	cr3d::GraphicsApi::T graphicsApi;
	cr3d::ShaderStage::T shaderStage;
};

class CrShaderCompiler
{
public:

	static const std::string& GetExecutablePath();

	static std::string ExecutablePath;

	void Initialize();

	void Finalize();

	void Compile(const CompilationDescriptor& compilationDescriptor);
};