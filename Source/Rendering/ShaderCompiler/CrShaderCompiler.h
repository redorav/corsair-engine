#pragma once

#include "Rendering/CrRendering.h"
#include "Core/CrPlatform.h"
#include "Core/String/CrString.h"
#include "Core/FileSystem/CrPath.h"
#include "Core/Containers/CrVector.h"

struct CompilationDescriptor
{
	CompilationDescriptor() {}

	void Process() const;

	CrPath inputPath;
	CrPath outputPath;
	CrPath tempPath; // Filename compiler can use to dump intermediate data
	CrString entryPoint;
	mutable CrVector<CrString> defines;
	cr::Platform::T platform;
	cr3d::GraphicsApi::T graphicsApi;
	cr3d::ShaderStage::T shaderStage;
	bool buildReflection = true;

private:

	mutable bool processed = true;
};

class CrShaderCompiler
{
public:

	static const CrString& GetExecutableDirectory();

	static CrString ExecutableDirectory;

	static void Initialize();

	static void Finalize();

	static bool Compile(const CompilationDescriptor& compilationDescriptor, CrString& compilationStatus);
};