#pragma once

#include "Rendering/CrRendering.h"
#include "Core/CrPlatform.h"
#include "Core/String/CrString.h"
#include "Core/Containers/CrVector.h"

struct CompilationDescriptor
{
	CrString inputPath;
	CrString outputPath;
	CrString tempPath; // Filename compiler can use to dump intermediate data
	CrString entryPoint;
	CrVector<CrString> defines;
	cr::Platform::T platform;
	cr3d::GraphicsApi::T graphicsApi;
	cr3d::ShaderStage::T shaderStage;
	bool buildReflection;
};

class CrShaderCompiler
{
public:

	static const CrString& GetExecutableDirectory();

	static CrString ExecutableDirectory;

	void Initialize();

	void Finalize();

	bool Compile(const CompilationDescriptor& compilationDescriptor, CrString& compilationStatus);
};