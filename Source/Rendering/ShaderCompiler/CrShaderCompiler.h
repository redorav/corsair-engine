#pragma once

#include "Rendering/CrRendering.h"
#include "Core/CrPlatform.h"
#include "Core/String/CrString.h"
#include "Core/Containers/CrVector.h"

#include <string>

struct CompilationDescriptor
{
	CrString inputPath;
	CrString outputPath;
	CrString entryPoint;
	CrVector<CrString> defines;
	cr::Platform::T platform;
	cr3d::GraphicsApi::T graphicsApi;
	cr3d::ShaderStage::T shaderStage;
};

class CrShaderCompiler
{
public:

	static const std::string& GetExecutableDirectory();

	static std::string ExecutableDirectory;

	void Initialize();

	void Finalize();

	bool Compile(const CompilationDescriptor& compilationDescriptor, std::string& compilationStatus);
};