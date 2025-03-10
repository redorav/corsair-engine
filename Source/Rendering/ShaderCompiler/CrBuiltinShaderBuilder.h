#include "Core/CrCoreForwardDeclarations.h"
#include "Core/FileSystem/CrFixedPath.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

struct CrBuiltinShadersDescriptor
{
	// Path where .shaders files live
	CrFixedPath inputPath;

	// Path where builtin shader binaries are output
	CrFixedPath outputPath;

	// We can build builtin shaders for multiple APIs for certain platforms such as Windows
	// which builds Vulkan and D3D12
	crstl::vector<cr3d::GraphicsApi::T> graphicsApis;

	cr::Platform::T platform;

	// Whether to build the headers from the binaries by creating headers and cpp files
	// This is useful as part of the main build, but not when live recompiling
	bool buildBuiltinHeaders;
};

struct CompilationDescriptor;

struct CrShaderInfo
{
	crstl::string name;
};

struct CrShaderCompilationJob
{
	crstl::string name;
	CompilationDescriptor compilationDescriptor;
};

class CrBuiltinShaderBuilder
{
public:

	static void ProcessBuiltinShaders(const CrBuiltinShadersDescriptor& builtinShadersDescriptor);

private:

	static void BuildBuiltinShaderMetadataAndHeaderFiles
	(
		const CrBuiltinShadersDescriptor& builtinShadersDescriptor, 
		const crstl::vector<CrShaderInfo>& shaderInfos,
		const crstl::vector<crstl::vector<CrShaderCompilationJob>>& compilationJobs
	);
};