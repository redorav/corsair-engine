#include "CrRendering_pch.h"

#include "Rendering/CrShaderDiskCache.h"
#include "Rendering/CrShaderManager.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/ICrShader.h"

#include "Core/FileSystem/ICrFile.h"
#include "Core/CrHash.h"
#include "Core/Streams/CrFileStream.h"
#include "Core/Function/CrFixedFunction.h"

CrShaderDiskCache::CrShaderDiskCache(const CrPath& cachePath, const char* hashFilename, CrHash currentHash)
	: m_cachePath(cachePath)
{
	// We store the hash file at the same level as the folder
	CrPath hashPath = cachePath.parent_path() / hashFilename;
	CrHash storedHash;

	bool clearCache = true;

	// If we have a hash, check whether we've produced the same hash. If so, the cache is valid
	// If not, create the hash file and store it
	if (CrFileHandle hashFile = ICrFile::OpenFile(hashPath.c_str(), FileOpenFlags::Read))
	{
		hashFile->Read((void*)&storedHash, sizeof(storedHash));

		// If we have the same hash, the cache is valid and we don't need to do anything about it
		// If not, delete the contents of the folder and start again
		if (storedHash == currentHash)
		{
			clearCache = false;
		}
	}

	if (ICrFile::DirectoryExists(cachePath.c_str()))
	{
		if (clearCache)
		{
			ICrFile::ForEachDirectoryEntry(cachePath.c_str(), true, [](const CrDirectoryEntry& entry)
			{
				if (!entry.isDirectory)
				{
					CrPath filePath = entry.directory;
					filePath /= entry.filename;
					ICrFile::FileDelete(filePath.c_str());
				}

				return true;
			});
		}
	}
	else
	{
		// Make sure directory exists
		ICrFile::CreateDirectories(cachePath.c_str());
	}

	if (clearCache)
	{
		CrFileHandle hashFile = ICrFile::OpenFile(hashPath.c_str(), FileOpenFlags::ForceCreate | FileOpenFlags::Write);
		hashFile->Write((void*)&currentHash, sizeof(currentHash));
	}
}

CrPath CrShaderDiskCache::CreateCachedFilePath(const CrHash& hash, cr3d::GraphicsApi::T graphicsApi) const
{
	CrString hashString = eastl::to_string(hash.GetHash());
	CrPath cachedBytecodePath = m_cachePath;
	cachedBytecodePath /= hashString.c_str();
	cachedBytecodePath.replace_extension(CrShaderManager::Get().GetShaderBytecodeExtension(graphicsApi));
	return cachedBytecodePath;
}

CrShaderBytecodeHandle CrShaderDiskCache::LoadFromCache(const CrHash& hash, cr3d::GraphicsApi::T graphicsApi) const
{
	CrPath cachedBytecodePath = CreateCachedFilePath(hash, graphicsApi);
	
	CrReadFileStream cachedBytecodeFile(cachedBytecodePath.c_str());
	
	if (cachedBytecodeFile.GetFile())
	{
		CrShaderBytecodeHandle shaderBytecode(new CrShaderBytecode());
		cachedBytecodeFile << *shaderBytecode.get();
		return shaderBytecode;
	}
	else
	{
		return nullptr;
	}
}

void CrShaderDiskCache::SaveToCache(const CrHash& hash, cr3d::GraphicsApi::T graphicsApi, const CrShaderBytecodeHandle& bytecode) const
{
	if (bytecode)
	{
		CrPath cachedBytecodePath = CreateCachedFilePath(hash, graphicsApi);

		CrWriteFileStream writeFileStream(cachedBytecodePath.c_str());
		writeFileStream << *bytecode.get();
	}
}
