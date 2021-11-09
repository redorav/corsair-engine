#include "CrRendering_pch.h"

#include "Rendering/CrShaderDiskCache.h"
#include "Rendering/CrShaderManager.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/ICrShader.h"

#include "Core/FileSystem/ICrFile.h"
#include "Core/CrHash.h"
#include "Core/Streams/CrFileStream.h"

CrShaderDiskCache::CrShaderDiskCache(const CrPath& cachePath)
	: m_cachePath(cachePath)
{
	// Make sure directory exists
	ICrFile::CreateDirectories(cachePath.c_str());
}

CrPath CrShaderDiskCache::CreateCachedFilePath(const CrHash& hash, cr3d::GraphicsApi::T graphicsApi) const
{
	CrString hashString = eastl::to_string(hash.GetHash());
	CrPath cachedBytecodePath = m_cachePath;
	cachedBytecodePath /= hashString.c_str();
	cachedBytecodePath.replace_extension(CrShaderManager::Get().GetShaderBytecodeExtension(graphicsApi));
	return cachedBytecodePath;
}

CrShaderBytecodeSharedHandle CrShaderDiskCache::LoadFromCache(const CrHash& hash, cr3d::GraphicsApi::T graphicsApi) const
{
	CrPath cachedBytecodePath = CreateCachedFilePath(hash, graphicsApi);
	
	CrReadFileStream cachedBytecodeFile(cachedBytecodePath.c_str());
	
	if (cachedBytecodeFile.GetFile())
	{
		CrShaderBytecodeSharedHandle shaderBytecode(new CrShaderBytecode());
		cachedBytecodeFile << *shaderBytecode.get();
		return shaderBytecode;
	}
	else
	{
		return nullptr;
	}
}

void CrShaderDiskCache::SaveToCache(const CrHash& hash, cr3d::GraphicsApi::T graphicsApi, const CrShaderBytecodeSharedHandle& bytecode) const
{
	if (bytecode)
	{
		CrPath cachedBytecodePath = CreateCachedFilePath(hash, graphicsApi);

		CrWriteFileStream writeFileStream(cachedBytecodePath.c_str());
		writeFileStream << *bytecode.get();
	}
}
