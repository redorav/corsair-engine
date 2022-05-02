#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"

#include "Core/CrCoreForwardDeclarations.h"
#include "Core/FileSystem/CrPath.h"

struct CrMaterialShaderDescriptor;

class CrShaderDiskCache
{
public:

	CrShaderDiskCache() {}

	// The current hash is what we want to use this cache with. If the hash doesn't match, we'll delete the cache
	CrShaderDiskCache(const CrPath& cachePath, const char* hashFilename, CrHash currentHash);

	CrPath CreateCachedFilePath(const CrHash& hash, cr3d::GraphicsApi::T graphicsApi) const;

	CrShaderBytecodeSharedHandle LoadFromCache(const CrHash& hash, cr3d::GraphicsApi::T graphicsApi) const;

	void SaveToCache(const CrHash& hash, cr3d::GraphicsApi::T graphicsApi, const CrShaderBytecodeSharedHandle& bytecode) const;

private:

	CrPath m_cachePath;
};