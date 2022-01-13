#include "Core/CrCoreForwardDeclarations.h"
#include "Core/FileSystem/CrPath.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

struct CrBuiltinShadersDescriptor
{
	// Path where .shaders files live
	CrPath inputPath;

	// Path where builtin shader binaries are output
	CrPath outputPath;
	
	// We can build builtin shaders for multiple APIs for certain platforms such as Windows
	// which builds Vulkan and D3D12
	CrVector<cr3d::GraphicsApi::T> graphicsApis;

	cr::Platform::T platform;
};

struct CompilationDescriptor;

struct CrShaderInfo
{
	CrString name;
};

struct CrShaderCompilationJob
{
	CrString name;
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
		const CrVector<CrShaderInfo>& shaderInfos,
		const CrVector<CrVector<CrShaderCompilationJob>>& compilationJobs
	);
};