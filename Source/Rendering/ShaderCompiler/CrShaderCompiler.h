#pragma once

#include "Rendering/CrRendering.h"
#include "Core/CrPlatform.h"
#include "Core/String/CrString.h"
#include "Core/FileSystem/CrPath.h"
#include "Core/Containers/CrVector.h"

struct CompilationDescriptor
{
	CompilationDescriptor()
	: platform(cr::Platform::Count)
	, graphicsApi(cr3d::GraphicsApi::Count)
	, shaderStage(cr3d::ShaderStage::Count)
	, buildReflection(true)
	, processed(false)
	{}

	void Process() const;

	CrPath inputPath;
	CrPath outputPath;
	CrPath tempPath; // Filename compiler can use to dump intermediate data
	CrString entryPoint;
	mutable CrVector<CrString> defines;
	cr::Platform::T platform;
	cr3d::GraphicsApi::T graphicsApi;
	cr3d::ShaderStage::T shaderStage;
	bool buildReflection;

private:

	mutable bool processed;
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