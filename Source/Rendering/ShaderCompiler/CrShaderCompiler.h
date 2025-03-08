#pragma once

#include "Rendering/CrRendering.h"
#include "Core/CrPlatform.h"
#include "Core/FileSystem/CrFixedPath.h"

#include "crstl/string.h"
#include "crstl/vector.h"

namespace OptimizationLevel
{
	enum T
	{
		O0,
		O1,
		O2,
		O3,
		None
	};
};

struct CompilationDescriptor
{
	CompilationDescriptor()
	: platform(cr::Platform::Count)
	, graphicsApi(cr3d::GraphicsApi::Count)
	, shaderStage(cr3d::ShaderStage::Count)
	, buildReflection(true)
	, optimization(OptimizationLevel::O3)
	, processed(false)
	{}

	void Process() const;

	CrFixedPath inputPath;
	CrFixedPath outputPath;
	CrFixedPath tempPath; // Filename compiler can use to dump intermediate data
	crstl::string entryPoint;
	crstl::string uniqueBinaryName;
	mutable crstl::vector<crstl::string> defines;
	cr::Platform::T platform;
	cr3d::GraphicsApi::T graphicsApi;
	cr3d::ShaderStage::T shaderStage;
	bool buildReflection;
	OptimizationLevel::T optimization;

private:

	mutable bool processed;
};

class CrShaderCompiler
{
public:

	static const crstl::string& GetExecutableDirectory();

	static crstl::string ExecutableDirectory;

	static CrFixedPath PDBDirectory;

	static CrFixedPath PDBDirectories[cr::Platform::Count][cr3d::GraphicsApi::Count];

	static const CrFixedPath& GetPDBDirectory(cr::Platform::T platform, cr3d::GraphicsApi::T graphicsApi);

	static void Initialize();

	static void Finalize();

	static bool Compile(const CompilationDescriptor& compilationDescriptor, crstl::string& compilationStatus);
};