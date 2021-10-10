#include "CrRendering_pch.h"

#include "Rendering/CrShaderDiskCache.h"
#include "Rendering/CrShaderManager.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/ICrShader.h"

#include "Core/FileSystem/ICrFile.h"
#include "Core/CrHash.h"

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

CrShaderBytecodeSharedHandle CrShaderDiskCache::LoadFromCache
(const CrHash& hash, const char* entryPoint, cr3d::GraphicsApi::T graphicsApi, cr3d::ShaderStage::T shaderStage) const
{
	CrPath cachedBytecodePath = CreateCachedFilePath(hash, graphicsApi);
	
	CrFileSharedHandle cachedBytecodeFile = ICrFile::OpenFile(cachedBytecodePath.c_str(), FileOpenFlags::Read);
	
	if (cachedBytecodeFile)
	{
		CrVector<unsigned char> bytecodeData;
		bytecodeData.resize(cachedBytecodeFile->GetSize());
		cachedBytecodeFile->Read(bytecodeData.data(), bytecodeData.size());
		return CrShaderBytecodeSharedHandle(new CrShaderBytecode(std::move(bytecodeData), entryPoint, shaderStage));
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

		// Save bytecode to cache for next run
		CrFileSharedHandle shaderBytecodeCached = ICrFile::OpenFile(cachedBytecodePath.c_str(), FileOpenFlags::ForceCreate | FileOpenFlags::Write);
		shaderBytecodeCached->Write((void*)bytecode->GetBytecode().data(), bytecode->GetBytecode().size());
	}
}
