#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"

#include "Core/CrCoreForwardDeclarations.h"
#include "Core/FileSystem/CrPath.h"

struct CrMaterialShaderDescriptor;

class CrShaderDiskCache
{
public:

	CrShaderDiskCache() {}

	CrShaderDiskCache(const CrPath& cachePath);

	CrPath CreateCachedFilePath(const CrHash& hash, cr3d::GraphicsApi::T graphicsApi) const;

	CrShaderBytecodeSharedHandle LoadFromCache
	(const CrHash& hash, const char* entryPoint, cr3d::GraphicsApi::T graphicsApi, cr3d::ShaderStage::T shaderStage) const;

	void SaveToCache(const CrHash& hash, cr3d::GraphicsApi::T graphicsApi, const CrShaderBytecodeSharedHandle& bytecode) const;

private:

	CrPath m_cachePath;
};