#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrShaderDiskCache.h"

#include "Core/CrCoreForwardDeclarations.h"

#include "crstl/string.h"

struct CrMaterialDescriptor;
struct CrMaterialShaderDescriptor;

// Used to define variables, options, etc
struct CrShaderHeaderGenerator
{
	static crstl::string HashDefine;

	// Adds a define, with no value, e.g. #define Example
	void Define(const char* define);

	// Adds a define as a character sequence, e.g. #define Example MyExample
	void DefineString(const char* define, const char* string);

	// Adds a define with an integer value, e.g. #define Example 2
	void DefineInt(const char* define, int value);

	const crstl::string& GetString() const
	{
		return m_header;
	}

private:

	crstl::string m_header;
};

// Creates materials out of a material definition. It takes a descriptor
// and up-to-date shader code, and produces the shaders necessary for
// the material to be then used to pair with a mesh. A material itself
// cannot be used on its own to render things yet, as it needs to be paired
// with a mesh and pipelines produced for it
class CrMaterialCompiler
{
public:

	static void Initialize();

	static void Deinitialize();

	// Creates set of defines based on the material shader descriptor
	void CreateMaterialShaderDefines(const CrMaterialShaderDescriptor& materialShaderDescriptor, CrShaderCompilerDefines& defines);

	// Compiles a material through its material descriptor
	CrMaterialHandle CompileMaterial(const CrMaterialDescriptor& descriptor);

	// Gets a material shader through the internal cache, or if not available, sends it off for compilation
	CrShaderBytecodeHandle GetDiskCachedOrCompileShaderBytecode
	(const CrFixedPath& shaderSourcePath, const crstl::string& entryPoint, const CrHash& shaderHash, const CrMaterialShaderDescriptor& materialShaderDescriptor);

private:

	CrMaterialCompiler();

	CrShaderDiskCache m_bytecodeDiskCache;
};

extern CrMaterialCompiler* MaterialCompiler;