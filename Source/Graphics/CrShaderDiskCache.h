#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"

#include "Core/CrCoreForwardDeclarations.h"
#include "Core/FileSystem/CrFixedPath.h"

struct CrMaterialShaderDescriptor;

class CrShaderDiskCache
{
public:

	CrShaderDiskCache() {}

	// The current hash is what we want to use this cache with. If the hash doesn't match, we'll delete the cache
	CrShaderDiskCache(const CrFixedPath& cachePath, const char* hashFilename, CrHash currentHash);

	CrFixedPath CreateCachedFilePath(const CrHash& hash, cr3d::GraphicsApi::T graphicsApi) const;

	CrShaderBytecodeHandle LoadFromCache(const CrHash& hash, cr3d::GraphicsApi::T graphicsApi) const;

	void SaveToCache(const CrHash& hash, cr3d::GraphicsApi::T graphicsApi, const CrShaderBytecodeHandle& bytecode) const;

private:

	CrFixedPath m_cachePath;
};